#include "stdafx.h"
#include "chunkLoader.h"

using namespace std;
using namespace wiScene;
extern mutex m;

std::vector<cyImportant::chunkpos_t> chunkLoader::maskedChunks;
mutex chunkLoader::maskMutex;

void chunkLoader::spawnThreads(uint8_t numthreads) {
	m_shutdown = 0;
	if (numthreads > MAX_THREADS) {
		numthreads = MAX_THREADS;
	}
	m_numthreads = numthreads;
	for (uint8_t i = 0; i < m_numthreads; i++) {
		m_threadstate[i] = THREAD_IDLE;
		m_thread[i]		 = thread(&chunkLoader::addChunks, this, i);
	}
	m_checkThread = thread(&chunkLoader::checkChunks, this);
}

chunkLoader::~chunkLoader(void) {
	shutdown();
}

void chunkLoader::shutdown(void) {
	if (m_checkThread.joinable()) {
		m_shutdown = 1;
		m_checkThread.join();
	}
	m_shutdown = 2;
	for (uint8_t i = 0; i < m_numthreads; i++) {
		if (m_thread[i].joinable()) {
			m_thread[i].join();
		}
	}
}

void chunkLoader::addMaskedChunk(const cyImportant::chunkpos_t chunkPos) {
	maskMutex.lock();
	maskedChunks.push_back(chunkPos);
	maskMutex.unlock();
}

void chunkLoader::checkChunks(void) {
	cyImportant::chunkpos_t ghostpos, coords, lastghostpos,exactGhostPos;
	cyImportant* world	  = settings::getWorld();
	uint32_t changed	  = 0;
	lastghostpos.x		  = -1000;
	lastghostpos.y		  = -1000;
	wiScene::Scene& scene = wiScene::GetScene();
	while (m_shutdown == 0) {
		Sleep(5);
		maskMutex.lock();
		for (size_t i = 0; i < maskedChunks.size(); i++) {
			auto it = m_visibleChunks.find(maskedChunks[i]);
			if (it != m_visibleChunks.end()) {
				if (it->second.chunkObj != wiECS::INVALID_ENTITY && it->second.chunkObj != 0xFFFFFFFE) {
					wiScene::Scene& scene = wiScene::GetScene();
					wiScene::ObjectComponent* obj = scene.objects.GetComponent(it->second.chunkObj);
					//if (obj != nullptr) {
					wiECS::Entity meshEnt = obj->meshID;
					m.lock();
					scene.Component_RemoveChildren(it->second.chunkObj);
					scene.Entity_Remove(it->second.chunkObj);
					scene.Entity_Remove(meshEnt);
					m.unlock();
					it->second.chunkObj = wiECS::INVALID_ENTITY;
				}
			}
		}
		maskMutex.unlock();
		uint32_t viewDist = settings::getViewDist();
		if (!world->isStopped()) {
			if (!settings::pauseChunkloader) {

				CameraComponent cam = wiScene::GetCamera();
				XMFLOAT3 campos		= cam.Eye;
				XMFLOAT3 camlook	= cam.At;
				ghostpos.x			= (int32_t)(campos.x);	//to move the center for chunkloading a little towards the view direction, to load more chunks in this direction --> add:  camlook.x * (viewDist * 4)
				ghostpos.y			= -(int32_t)(campos.z);
				exactGhostPos		= ghostpos;
				ghostpos.x &= 0xFFFFFFE0;
				ghostpos.y &= 0xFFFFFFE0;
				changed = 0;
				if (lastghostpos.x != ghostpos.x || lastghostpos.y != ghostpos.y) {
					coords = ghostpos;
					if (m_visibleChunks.find(coords) == m_visibleChunks.end()) {
						changed++;
						employThread(coords);
					}

					for (uint32_t r = 4; r <= viewDist; r = r + 4) {
						for (uint32_t i = 0; i < r; i = i + 1) {
							for (uint32_t ii = ((r - 4) > i) ? (1 + r - 4 - i) : 0; ii <= r - i; ii++) {
								if (i * i + ii * ii <= (viewDist * viewDist) / 2) {
									if (i || ii) {
										coords.x = ghostpos.x + i * 16;
										coords.y = ghostpos.y + ii * 16;
										std::unordered_map<cyImportant::chunkpos_t, chunkobjects_t, cyImportant::chunkpos_t>::iterator visChunk = m_visibleChunks.find(coords);
										if (visChunk == m_visibleChunks.end()) {
											changed++;
											employThread(coords);
										}
										coords.x = ghostpos.x - i * 16;
										coords.y = ghostpos.y - ii * 16;
										visChunk = m_visibleChunks.find(coords);
										if (visChunk == m_visibleChunks.end()) {
											changed++;
											employThread(coords);
										} 
									}
									if (i && ii) {
										coords.x = ghostpos.x - i * 16;
										coords.y = ghostpos.y + ii * 16;
										std::unordered_map<cyImportant::chunkpos_t, chunkobjects_t, cyImportant::chunkpos_t>::iterator visChunk = m_visibleChunks.find(coords);
										if (visChunk == m_visibleChunks.end()) {
											changed++;
											employThread(coords);
										}
										coords.x = ghostpos.x + i * 16;
										coords.y = ghostpos.y - ii * 16;
										visChunk = m_visibleChunks.find(coords);
										if (visChunk == m_visibleChunks.end()) {
											changed++;
											employThread(coords);
										} 
									}
								}
							}
						}
						if (changed > 100) {
							changed	   = 1;
							ghostpos.x = (int32_t)(campos.x);  //to move the center for chunkloading a little towards the view direction, to load more chunks in this direction --> add:  camlook.x * (viewDist * 4)
							ghostpos.y = -(int32_t)(campos.z);
							ghostpos.x &= 0xFFFFFFE0;
							ghostpos.y &= 0xFFFFFFE0;
							if (lastghostpos.x != ghostpos.x || lastghostpos.y != ghostpos.y) {
								break;
							}
						}
					}
					removeFarChunks(exactGhostPos);
					settings::numVisChunks = m_visibleChunks.size();
					if (changed == 0) {
						lastghostpos = ghostpos;
					}

				} else {
					removeFarChunks(exactGhostPos);
					settings::numVisChunks = m_visibleChunks.size();
					Sleep(100);
				}
			} else {
				Sleep(50);
			}
		} else {  //world stopped --> clear all existing chunks (if there are any)
			while (m_visibleChunks.size()) {
				removeFarChunks(exactGhostPos, true);
			}
			/*
			for (unordered_map<cyImportant::chunkpos_t, chunkobjects_t>::iterator it = m_visibleChunks.begin(); it != m_visibleChunks.end();) {
				if (it->second.chunkObj == 0xFFFFFFFE) {  //wait for the chunk to be finished
					it = m_visibleChunks.find(it->first);
					Sleep(2);
				} else {
					if (it->second.chunkObj != wiECS::INVALID_ENTITY) {
						wiECS::Entity meshEnt = scene.objects.GetComponent(it->second.chunkObj)->meshID;
						m.lock();
						scene.Component_RemoveChildren(it->second.chunkObj);
						scene.Entity_Remove(it->second.chunkObj);
						scene.Entity_Remove(meshEnt);
						//scene.Entity_Remove(meshEnt);
						//scene.Entity_Remove(it->second.chunkObj);
						//for (size_t i = 0; i < it->second.meshes.size(); i++) {
						//	scene.Entity_Remove(it->second.meshes[i]);
						//}
						m.unlock();
					}
					it++;
				}
			}*/
			m_visibleChunks.clear();
			world->cleaned = true;
		}
		
	}
	m_shutdown = 2;
}

void chunkLoader::addChunks(uint8_t threadNum) {
	chunkobjects_t chunkObj;
	while (m_shutdown < 2) {
		if (m_threadstate[threadNum] == THREAD_BUSY) {
			cyImportant* world = settings::getWorld();
			cyImportant::chunkpos_t coords, zero;
			coords = m_threadChunkPos[threadNum];
			if (world->isValid()) {
				uint32_t chunkID;
				cyChunk chunk;
				cyChunk chunkL;
				cyChunk chunkR;
				cyChunk chunkU;
				cyChunk chunkD;
				zero.x = (uint32_t)(world->m_playerpos.x / 100);
				zero.y = (uint32_t)(world->m_playerpos.y / 100);

				if (world->getChunkID(coords.x + zero.x, coords.y + zero.y, &chunkID)) {
					chunk.loadChunk(world->db[threadNum], chunkID);
					if (world->getChunkID(coords.x + 16 + zero.x, coords.y + zero.y, &chunkID))
						chunkL.loadChunk(world->db[threadNum], chunkID, true);
					if (world->getChunkID(coords.x - 16 + zero.x, coords.y + zero.y, &chunkID))
						chunkR.loadChunk(world->db[threadNum], chunkID, true);
					if (world->getChunkID(coords.x + zero.x, coords.y + zero.y + 16, &chunkID))
						chunkU.loadChunk(world->db[threadNum], chunkID, true);
					if (world->getChunkID(coords.x + zero.x, coords.y + zero.y - 16, &chunkID))
						chunkD.loadChunk(world->db[threadNum], chunkID, true);
					chunkObj = chunkLoader::RenderChunk(chunk, chunkU, chunkL, chunkD, chunkR, coords.x, -coords.y);
					//m_visibleChunks[coords] = chunkObj;
					m_threadstate[threadNum] = chunkObj.chunkObj;
				} else {
					//wiBackLog::post("CHunk not found");
					m_threadstate[threadNum] = wiECS::INVALID_ENTITY;
				}
			} else {
				m_threadstate[threadNum] = wiECS::INVALID_ENTITY;
			}

		} else {
			Sleep(10);
		}
	}
}

inline void chunkLoader::removeFarChunks(cyImportant::chunkpos_t ghostpos, bool cleanAll) {
	wiScene::Scene& scene = wiScene::GetScene();
	for (unordered_map<cyImportant::chunkpos_t, chunkobjects_t>::iterator it = m_visibleChunks.begin(); it != m_visibleChunks.end();) {
		if (it->second.chunkObj != 0xFFFFFFFE) {
			cyImportant::chunkpos_t diff = it->first;
			diff						 = diff - ghostpos;
			float dist					 = (diff.x * diff.x + diff.y * diff.y);
			float viewDist				 = (settings::getViewDist() * settings::getViewDist() * 16 * 16);
			if (cleanAll || dist > viewDist) {
				if (it->second.chunkObj != wiECS::INVALID_ENTITY) {
					wiScene::ObjectComponent* obj = scene.objects.GetComponent(it->second.chunkObj);
						wiECS::Entity meshEnt = obj->meshID;
						m.lock();
						scene.Component_RemoveChildren(it->second.chunkObj);
						scene.Entity_Remove(it->second.chunkObj);
						scene.Entity_Remove(meshEnt);
						m.unlock();
 						it = m_visibleChunks.erase(it);
				} else {
					it = m_visibleChunks.erase(it);
				}
			} else {
				if (it->second.chunkObj != wiECS::INVALID_ENTITY) {
					if (dist > viewDist / 4) {
						if (it->second.lod != LOD_MAX) {
							it->second.lod = LOD_MAX;
							for (size_t iii = 0; iii < it->second.meshes.size(); iii++) {
								m.lock();
								wiScene::ObjectComponent* mesh = wiScene::GetScene().objects.GetComponent(it->second.meshes[iii]);
								if (mesh != nullptr) {
									mesh->SetCastShadow(false);
									mesh->SetRenderable(false);
								}
								m.unlock();
							}
						}
					} else if (dist > viewDist / 16) {
						if (it->second.lod != LOD_MAX - 1) {
							it->second.lod = LOD_MAX-1;
							for (size_t iii = 0; iii < it->second.meshes.size(); iii++) {
								m.lock();
								wiScene::ObjectComponent* mesh = wiScene::GetScene().objects.GetComponent(it->second.meshes[iii]);
								if (mesh != nullptr) {
									mesh->SetCastShadow(false);
									mesh->SetRenderable(true);
									//mesh->color = XMFLOAT4(1.f, 1.f, 1.f, 1 - ((float)dist - (viewDist / 16.0f)) / (viewDist / 4.f - viewDist / 16.f));
								}
								m.unlock();
							}
						} /*else {
							for (size_t iii = 0; iii < it->second.meshes.size(); iii++) {
								m.lock();
								wiScene::ObjectComponent* mesh = wiScene::GetScene().objects.GetComponent(it->second.meshes[iii]);
								if (mesh != nullptr && it->second.meshes[iii] != it->second.chunkObj) {
									mesh->color = XMFLOAT4(1.f, 1.f, 1.f, 1 - ((float)dist - (viewDist / 16.0f)) / (viewDist / 4.f - viewDist / 16.f));
								}
								m.unlock();
							}
						}*/
					} else if (it->second.lod) {
						it->second.lod = 0;
						for (size_t iii = 0; iii < it->second.meshes.size(); iii++) {
							m.lock();
							wiScene::ObjectComponent* mesh = wiScene::GetScene().objects.GetComponent(it->second.meshes[iii]);
							if (mesh != nullptr) {
								mesh->SetCastShadow(true);
								mesh->SetRenderable(true);
								//mesh->color = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
							}
							m.unlock();
						}
					}
				}
				it++;
			}
				
		} else {
			it++;
			for (uint8_t thread = 0; thread < m_numthreads; thread++) {
				if (m_threadstate[thread] != THREAD_BUSY) {
					if (m_threadstate[thread] != THREAD_IDLE) {
						m_visibleChunks[m_threadChunkPos[thread]].chunkObj = m_threadstate[thread];
						m_visibleChunks[m_threadChunkPos[thread]].lod	   = LOD_MAX;
						m.lock();
						for (size_t i = 0; i < wiScene::GetScene().objects.GetCount();i++)
						{
							if (wiScene::GetScene().objects[i].parentObject == m_threadstate[thread] && wiScene::GetScene().objects.GetEntity(i) != m_threadstate[thread])
							{
								wiECS::Entity entity = wiScene::GetScene().objects.GetEntity(i);
								m_visibleChunks[m_threadChunkPos[thread]].meshes.push_back(entity);
							}
						}
						m.unlock();
						m_threadstate[thread]							   = THREAD_IDLE;
					}
				}
			}
		}
	}
}

chunkLoader::chunkobjects_t chunkLoader::RenderChunk(cyChunk& chunk, const cyChunk& northChunk, const cyChunk& eastChunk, const cyChunk& southChunk, const cyChunk& westChunk, const int32_t relX, const int32_t relY) {
	face_t tmpface;
	wiECS::Entity entity = wiECS::INVALID_ENTITY;
	chunkobjects_t ret;
	vector<face_t> faces;
	vector<torch_t> torches;
	uint8_t stepsize	   = 1;
	uint16_t surfaceHeight = chunk.m_surfaceheight;
	ret.chunkObj		   = entity;
	if (chunk.m_isAirChunk)
		return ret;

	if (settings::clipUnderground == true) {
		surfaceHeight = min(surfaceHeight, eastChunk.m_surfaceheight);
		surfaceHeight = min(surfaceHeight, westChunk.m_surfaceheight);
		surfaceHeight = min(surfaceHeight, northChunk.m_surfaceheight);
		surfaceHeight = min(surfaceHeight, southChunk.m_surfaceheight);
		if (surfaceHeight > 25)
			surfaceHeight -= 25;
	} else {

		if (chunk.m_lowestZ > 2)
			surfaceHeight = chunk.m_lowestZ - 2;
		else
			surfaceHeight = 0;
	}

	wiScene::Scene tmpScene;
	for (int_fast16_t z = min(chunk.m_highestZ + 2, 799); z > surfaceHeight; z -= stepsize) {
		for (uint_fast8_t x = 0; x < 32; x += stepsize) {
			for (uint_fast8_t y = 0; y < 32; y += stepsize) {
				uint8_t blocktype = (uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * z);
				if (cyBlocks::m_regBlockTypes[blocktype] <= cyBlocks::BLOCKTYPE_ALPHA) {
					uint8_t neighbour[6];
					if (z < 799)
						neighbour[0] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * (z + stepsize))];
					else
						neighbour[0] = cyBlocks::BLOCKTYPE_VOID;

					if (z > 1)
						neighbour[1] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * (z - stepsize))];
					else
						neighbour[1] = cyBlocks::BLOCKTYPE_VOID;

					if (y < 32 - stepsize)
						neighbour[4] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * (y + stepsize) + 32 * 32 * z)];
					else
						neighbour[4] = cyBlocks::m_regBlockTypes[(uint8_t) * (northChunk.m_chunkdata + 4 + x + 32 * 32 * z)];

					if (y >= 0 + stepsize)
						neighbour[5] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * (y - stepsize) + 32 * 32 * z)];
					else
						neighbour[5] = cyBlocks::m_regBlockTypes[(uint8_t) * (southChunk.m_chunkdata + 4 + x + 32 * 31 + 32 * 32 * z)];

					if (x < 32 - stepsize)
						neighbour[3] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + stepsize + x + 32 * y + 32 * 32 * z)];
					else
						neighbour[3] = cyBlocks::m_regBlockTypes[(uint8_t) * (eastChunk.m_chunkdata + 4 + 32 * y + 32 * 32 * z)];

					if (x >= 0 + stepsize)
						neighbour[2] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 - stepsize + x + 32 * y + 32 * 32 * z)];
					else
						neighbour[2] = cyBlocks::m_regBlockTypes[(uint8_t) * (westChunk.m_chunkdata + 4 + 31 + 32 * y + 32 * 32 * z)];

					uint8_t antitile = 0;

					if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_MOD) {
						cyChunk::blockpos_t pos(x, y, z);
						std::unordered_map<cyChunk::blockpos_t, uint32_t>::const_iterator cBlock = chunk.m_cBlocks.find(pos);
						if (cBlock != chunk.m_cBlocks.end()) {
							for (uint8_t ft = 0; ft < 6; ft++) {
								if (neighbour[ft] > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
									tmpface.x		 = relX + x / 2.0f;
									tmpface.y		 = relY + 16 - y / 2.0f;
									tmpface.z		 = z / 2.0f;
									tmpface.antitile = false;
									tmpface.face	 = ft;
									tmpface.material = cyBlocks::m_cBlockTypes[cBlock->second].material[ft];
									faces.emplace_back(tmpface);
								}
							}
						}
					} else {
						for (uint8_t ft = 0; ft < 6; ft++) {
							if (neighbour[ft] != cyBlocks::m_regBlockTypes[blocktype]) {
								tmpface.x		 = relX + x / 2.0f;
								tmpface.y		 = relY + 16 - y / 2.0f;
								tmpface.z		 = z / 2.0f;
								tmpface.antitile = cyBlocks::m_regBlockFlags[blocktype][ft] & cyBlocks::BLOCKFLAG_ANTITILE;
								tmpface.face	 = ft;
								tmpface.material = cyBlocks::m_regBlockMats[blocktype][ft];
								faces.emplace_back(tmpface);
							}
						}
					}
				} else if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_DECOMESH) {
					cyChunk::meshLoc mesh;
					mesh.pos.x = x / 2.0f;
					mesh.pos.y = y / 2.0f;
					mesh.pos.z = z / 2.0f;
					mesh.scale.x = (0.8 + wiRandom::getRandom(20)/50.f);
					mesh.scale.y = mesh.scale.x;
					mesh.scale.z = mesh.scale.x;
					mesh.type	 = blocktype;
					chunk.addMesh(mesh);
				} else if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_BILLBOARD) {
					tmpface.x		 = relX + x / 2.0f;
					tmpface.y		 = relY + 16 - y / 2.0f;
					tmpface.z		 = z / 2.0f;
					tmpface.material = cyBlocks::m_regBlockMats[blocktype][0];
					tmpface.face	 = cyBlocks::FACE_BILLBOARD;
					faces.emplace_back(tmpface);
				} else if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_TORCH) {
					cyChunk::blockpos_t pos(x, y, z);
					auto it = chunk.m_Torches.find(pos);
					if (it != chunk.m_Torches.end()) {
						torches.push_back(torch_t(blocktype, x, y, z, it->second));
					}
				}
			}
		}
	}
	if (faces.size()) {
		SimplexNoise noise;
		sort(faces.begin(), faces.end());
		MeshComponent* mesh;
		string meshname = to_string(relX) + to_string(relY);
		if (wiScene::GetScene().materials.GetComponent(faces[0].material) != nullptr) {
			mesh = meshGen::AddMesh(tmpScene, to_string(chunk.m_id), faces[0].material, &entity);
		} else {
			mesh = meshGen::AddMesh(tmpScene, to_string(chunk.m_id), cyBlocks::m_fallbackMat, &entity);
		}
		ret.chunkObj		  = entity;
		wiECS::Entity currMat = faces[0].material;
		mesh->SetDoubleSided(true);
		for (unsigned i = 0; i < faces.size(); ++i)
		{
			if (faces[i].material != currMat) {
				if (wiScene::GetScene().materials.GetComponent(faces[i].material) != nullptr) {
					meshGen::newMaterial(mesh, faces[i].material);
				}
				currMat = faces[i].material;
			}
			switch (faces[i].face) {
				case cyBlocks::FACE_TOP:
					meshGen::AddFaceTop(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_BOTTOM:
					meshGen::AddFaceBottom(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_LEFT:
					meshGen::AddFaceLeft(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_RIGHT:
					meshGen::AddFaceRight(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_BACK:
					meshGen::AddFaceBack(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_FRONT:
					meshGen::AddFaceFront(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_BILLBOARD:
					if (stepsize == 1) {
						meshGen::AddBillboard(mesh, faces[i].x, faces[i].y, faces[i].z);
					}
					break;
			}
		}
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
		mesh->SetDynamic(false);
		mesh->CreateRenderData();
		ObjectComponent* chunkObj = tmpScene.objects.GetComponent(entity);
		XMFLOAT4 treecolor1		  = XMFLOAT4(1, 0., 0., 1);
		XMFLOAT4 treecolor2		  = XMFLOAT4(0., 1, 0., 1);
		for (size_t i = 0; i < chunk.trees.size(); i++) {
			if (chunk.trees[i].scale.x > 2 || chunk.trees[i].scale.y > 2 || chunk.trees[i].scale.z > 2) {
				wiHelper::messageBox("Tree scaling out of bounds!", "Error!");
				uint32_t chID = chunk.m_id;
				wiBackLog::post("Weird tree data: ChunkID: ");
				wiBackLog::post(to_string(chID).c_str());
			} else {
				wiECS::Entity objEnt	= tmpScene.Entity_CreateObject("tree");
				ObjectComponent& object = *tmpScene.objects.GetComponent(objEnt);
				LayerComponent& layer	= *tmpScene.layers.GetComponent(objEnt);
				layer.layerMask			= LAYER_TREE;
				TransformComponent& tf	= *tmpScene.transforms.GetComponent(objEnt);
				switch (chunk.trees[i].type) {
					case 0:	 //leaf trees (light wood)
						object.meshID = cyBlocks::m_treeMeshes[((chunk.trees[i].pos.x + chunk.trees[i].pos.y + chunk.trees[i].pos.z) % 3)];
						break;
					case 1:	 //needle trees (dark wood)
						object.meshID = cyBlocks::m_treeMeshes[3];
						break;
					case 2:	 //cactus
					case 3:
					case 4:
					case 5:
						object.meshID = cyBlocks::m_treeMeshes[4];
						break;
					default:  //desert grass 6,7
						object.meshID = cyBlocks::m_treeMeshes[5];
						break;
				}
				tf.Translate(XMFLOAT3(relX + chunk.trees[i].pos.x / 2, chunk.trees[i].pos.z / 2 - 0.25, relY + 16 - chunk.trees[i].pos.y / 2));
				tf.Scale(XMFLOAT3(chunk.trees[i].scale.x, chunk.trees[i].scale.z, chunk.trees[i].scale.y));
				tf.RotateRollPitchYaw(XMFLOAT3(0, chunk.trees[i].yaw, 0));
				tf.UpdateTransform();
				object.parentObject = entity;
				object.SetRenderable(false);	//load the Mesh with lowest LOD to prevent flicker
				object.SetCastShadow(false);
				ret.meshes.push_back(objEnt);
				//tmpScene.Component_Attach(objEnt, entity);
			}
		}
		placeTorches(torches, relX, relY, tmpScene, entity);
		placeMeshes(chunk, relX, relY, tmpScene, entity);
		m.lock();
		wiScene::GetScene().Merge(tmpScene);
		m.unlock();
	}  //else	wiBackLog::post("Chunk has no blocks");
	return ret;
}

inline void chunkLoader::placeMeshes(const cyChunk& chunk, const int32_t relX, const int32_t relY, Scene& tmpScene, const wiECS::Entity parent) {
	for (size_t i = 0; i < chunk.meshObjects.size(); i++) {
		if (chunk.meshObjects[i].scale.x > 2 || chunk.meshObjects[i].scale.y > 2 || chunk.meshObjects[i].scale.z > 2) {
			wiHelper::messageBox("Mesh scaling out of bounds!", "Error!");
			uint32_t chID = chunk.m_id;
			wiBackLog::post("Weird mesh data: ChunkID: ");
			wiBackLog::post(to_string(chID).c_str());
		} else {
			try {
				wiECS::Entity mesh = cyBlocks::m_regMeshes.at(chunk.meshObjects[i].type).mesh[0];
				if (mesh) {
					wiECS::Entity objEnt	= tmpScene.Entity_CreateObject("mesh");
					ObjectComponent& object = *tmpScene.objects.GetComponent(objEnt);
					object.meshID			= mesh;
					LayerComponent& layer	= *tmpScene.layers.GetComponent(objEnt);
					layer.layerMask			= LAYER_TREE;
					TransformComponent& tf	= *tmpScene.transforms.GetComponent(objEnt);
					tf.Translate(XMFLOAT3(relX + 8 + chunk.meshObjects[i].pos.x, chunk.meshObjects[i].pos.z, relY + 8 - chunk.meshObjects[i].pos.y));
					tf.Scale(XMFLOAT3(chunk.meshObjects[i].scale.x, chunk.meshObjects[i].scale.z, chunk.meshObjects[i].scale.y));
					tf.Rotate(XMFLOAT4(chunk.meshObjects[i].qRot.x, chunk.meshObjects[i].qRot.z, chunk.meshObjects[i].qRot.y, chunk.meshObjects[i].qRot.w));
					tf.UpdateTransform();
					object.parentObject = parent;
				}
			}
			catch (...) {
			}
		}
	}
	
}
inline void chunkLoader::placeTorches(const std::vector<chunkLoader::torch_t>& torches, const int32_t relX, const int32_t relY, Scene& tmpScene, const wiECS::Entity parent) {
	for (size_t i = 0; i < torches.size(); i++) {
		torch_t torch = torches[i];
		wiECS::Entity lightEnt;
		LightComponent* light = nullptr;
		wiECS::Entity meshID = 0;
		TransformComponent transform;
		XMFLOAT3 color((float)cyBlocks::m_regBlockFlags[torch.ID][0] / 255.f, (float)cyBlocks::m_regBlockFlags[torch.ID][1] / 255.f, (float)cyBlocks::m_regBlockFlags[torch.ID][2] / 255.f);
		try {
			switch (torch.rotation) {
				case 0:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f + 0.248, torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(0, PI / 2, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f + 0.1, torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 1:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f - 0.248, torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(0, -PI / 2, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f - 0.1, torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 2:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f + 0.248));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f + 0.1), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 3:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f - 0.248));
					transform.RotateRollPitchYaw(XMFLOAT3(0, PI, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f - 0.1), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 4:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[1];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f - 0.135f, relY + 16 - torch.y / 2.0f));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f + 0.08, relY + 16 - torch.y / 2.0f), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				default:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[1];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f + 0.135f, relY + 16 - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(PI, 0, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f - 0.08, relY + 16 - torch.y / 2.0f), XMFLOAT3(1.0, 0.3, 0.0f), 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
			}
			if (light != nullptr) {
				light->SetCastShadow(true);
				if (settings::torchlights == false) {
					light->SetStatic(true);
				}
				//ret.meshes.push_back(lightEnt);

				wiECS::Entity objEnt	= tmpScene.Entity_CreateObject("torch");
				ObjectComponent& object = *tmpScene.objects.GetComponent(objEnt);
				LayerComponent& layer	= *tmpScene.layers.GetComponent(objEnt);
				TransformComponent* tf	= tmpScene.transforms.GetComponent(objEnt);
				object.meshID		 = meshID;
				object.emissiveColor = XMFLOAT4(color.x, color.y, color.z, 1.0f);
				layer.layerMask		 = LAYER_TORCH;
				*tf					 = transform;
				tf->UpdateTransform();
				object.parentObject = parent;
				light->parentObject = parent;
			}
		}
		catch (...) {
		}
	}
}

inline void chunkLoader::employThread(cyImportant::chunkpos_t coords) {
	for (uint8_t thread = 0; thread < m_numthreads; thread++) {
		if (m_threadstate[thread] != THREAD_BUSY) {
			if (m_threadstate[thread] != THREAD_IDLE) {
				m_visibleChunks[m_threadChunkPos[thread]].chunkObj = m_threadstate[thread];
				m_visibleChunks[m_threadChunkPos[thread]].lod	   = LOD_MAX;
				m.lock();
				for (size_t i = 0; i < wiScene::GetScene().objects.GetCount(); i++)
				{
					if (wiScene::GetScene().objects[i].parentObject == m_threadstate[thread])
					{
						wiECS::Entity entity = wiScene::GetScene().objects.GetEntity(i);
						m_visibleChunks[m_threadChunkPos[thread]].meshes.push_back(entity);
					}
				}
				m.unlock();
				m_threadstate[thread]							   = THREAD_IDLE;
			}
			m_threadChunkPos[thread]		 = coords;
			m_threadstate[thread]			 = THREAD_BUSY;
			m_visibleChunks[coords].chunkObj = 0xFFFFFFFE;
			break;
		}
	}
}


/*
cyImportant::chunkpos_t chunkLoader::spiral(const int32_t iteration) {
	cyImportant::chunkpos_t ret;
	double temp = (sqrt(iteration) - 1.0) / 2.0;
	int32_t k	= ceil(temp);
	int32_t t	= 2 * k + 1;
	int32_t m	= t * t;
	t--;

	if (iteration >= (m - t)) {
		ret.x = k - (m - iteration);
		ret.y = -k;
	} else {
		m = m - t;
		if (iteration >= (m - t)) {
			ret.x = -k;
			ret.y = -k + (m - iteration);
		} else {
			m = m - t;
			if (iteration >= (m - t)) {
				ret.x = -k + (m - iteration);
				ret.y = k;
			} else {
				ret.x = k;
				ret.y = k - (m - iteration - t);
			}
		}
	}
	ret.x *= 16;
	ret.y *= 16;
	return ret;
}
*/
