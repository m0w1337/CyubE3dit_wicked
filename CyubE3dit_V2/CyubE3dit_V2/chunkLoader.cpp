#include "stdafx.h"
#include "chunkLoader.h"
using namespace std;
using namespace wiScene;
extern mutex m;

void chunkLoader::spawnThreads(uint8_t numthreads) {
	m_shutdown = 0;
	if (numthreads <= MAX_THREADS) {
		m_numthreads = numthreads;
		for (uint8_t i = 0; i < m_numthreads; i++) {
			m_threadstate[i] = 0;
			m_thread[i]		 = thread(&chunkLoader::addChunks, this, i);
		}
		m_checkThread = thread(&chunkLoader::checkChunks, this);
	}
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

void chunkLoader::checkChunks(void) {
	cyImportant::chunkpos_t ghostpos, coords, lastghostpos;
	cyImportant* world = settings::getWorld();
	bool changed	   = false;
	lastghostpos.x	   = -1000;
	lastghostpos.y	   = -1000;
	while (m_shutdown == 0) {
		Sleep(10);
		uint32_t viewDist = settings::getViewDist();
		if (!world->isStopped()) {
			CameraComponent cam = wiScene::GetCamera();
			XMFLOAT3 campos		= cam.Eye;
			XMFLOAT3 camlook	= cam.At;
			ghostpos.x			= (int32_t)(campos.x);  //move the center for chunkloading a little towards the view direction, to load more chunks in this direction --> add:  camlook.x * (viewDist * 4)
			ghostpos.y			= -(int32_t)(campos.z);
			ghostpos.x &= 0xFFFFFFE0;
			ghostpos.y &= 0xFFFFFFE0;
			if (lastghostpos.x != ghostpos.x || lastghostpos.y != ghostpos.y) {
				coords = ghostpos;
				if (m_visibleChunks.find(coords) == m_visibleChunks.end()) {
					for (uint8_t thread = 0; thread < m_numthreads; thread++) {
						if (m_threadstate[thread] == 0) {
							m_threadChunkX[thread]	= coords.x;
							m_threadChunkY[thread]	= coords.y;
							m_threadstate[thread]	= 1;
							m_visibleChunks[coords].chunkObj = 0xFFFFFFFE;
							break;
						}
					}
				}
				for (uint32_t r = 4; r <= viewDist; r = r + 4) {
					for (uint32_t i = 0; i < r; i = i + 1) {
						for (uint32_t ii = ((r - 4) > i) ? (1 + r - 4 - i) : 0; ii <= r - i; ii++) {
							if (i * i + ii * ii <= (viewDist * viewDist) / 2) {
								if (i || ii) {
									coords.x = ghostpos.x + i * 16;
									coords.y = ghostpos.y + ii * 16;
									if (m_visibleChunks.find(coords) == m_visibleChunks.end()) {
										changed = true;
										for (uint8_t thread = 0; thread < m_numthreads; thread++) {
											if (m_threadstate[thread] == 0) {
												m_threadChunkX[thread]	= coords.x;
												m_threadChunkY[thread]	= coords.y;
												m_threadstate[thread]	= 1;
												m_visibleChunks[coords].chunkObj = 0xFFFFFFFE;
												break;
											}
										}
									}
									coords.x = ghostpos.x - i * 16;
									coords.y = ghostpos.y - ii * 16;
									if (m_visibleChunks.find(coords) == m_visibleChunks.end()) {
										changed = true;
										for (uint8_t thread = 0; thread < m_numthreads; thread++) {
											if (m_threadstate[thread] == 0) {
												m_threadChunkX[thread]	= coords.x;
												m_threadChunkY[thread]	= coords.y;
												m_threadstate[thread]	= 1;
												m_visibleChunks[coords].chunkObj = 0xFFFFFFFE;
												break;
											}
										}
									}
								}
								if (i && ii) {
									coords.x = ghostpos.x - i * 16;
									coords.y = ghostpos.y + ii * 16;
									if (m_visibleChunks.find(coords) == m_visibleChunks.end()) {
										changed = true;
										for (uint8_t thread = 0; thread < m_numthreads; thread++) {
											if (m_threadstate[thread] == 0) {
												m_threadChunkX[thread]	= coords.x;
												m_threadChunkY[thread]	= coords.y;
												m_threadstate[thread]	= 1;
												m_visibleChunks[coords].chunkObj = 0xFFFFFFFE;
												break;
											}
										}
									}
									coords.x = ghostpos.x + i * 16;
									coords.y = ghostpos.y - ii * 16;
									if (m_visibleChunks.find(coords) == m_visibleChunks.end()) {
										changed = true;
										for (uint8_t thread = 0; thread < m_numthreads; thread++) {
											if (m_threadstate[thread] == 0) {
												m_threadChunkX[thread]	= coords.x;
												m_threadChunkY[thread]	= coords.y;
												m_threadstate[thread]	= 1;
												m_visibleChunks[coords].chunkObj = 0xFFFFFFFE;
												break;
											}
										}
									}
								}
							}
						}
					}
				}

				for (unordered_map<cyImportant::chunkpos_t, chunkobjects_t>::iterator it = m_visibleChunks.begin(); it != m_visibleChunks.end();) {
					if (it->second.chunkObj != 0xFFFFFFFE) {
						cyImportant::chunkpos_t diff = it->first;
						diff						 = diff - ghostpos;
						if ((diff.x * diff.x + diff.y * diff.y) > (viewDist * viewDist * 16 * 16)) {
							if (it->second.chunkObj != wiECS::INVALID_ENTITY) {
								wiScene::Scene& scene = wiScene::GetScene();
								wiECS::Entity meshEnt = scene.objects.GetComponent(it->second.chunkObj)->meshID;
								m.lock();
								scene.Entity_Remove(meshEnt);
								scene.Entity_Remove(it->second.chunkObj);
								for (size_t i = 0; i < it->second.trees.size(); i++) {
									scene.Entity_Remove(it->second.trees[i]);
								}
								m.unlock();
							}
							auto pos = m_visibleChunks.erase(it);
							it		 = pos;
						} else
							it++;
					} else {
						it++;
					}
				}

				settings::numVisChunks = m_visibleChunks.size();
				if (changed == false) {
					lastghostpos = ghostpos;
				}

			} else {
				Sleep(200);
			}
		} else {  //No valid world --> clear all axisting chunks (if there are any)
			for (unordered_map<cyImportant::chunkpos_t, chunkobjects_t>::iterator it = m_visibleChunks.begin(); it != m_visibleChunks.end();) {
				if (it->second.chunkObj == 0xFFFFFFFE) {  //wait for the chunk to be finished
					Sleep(1);
					it = m_visibleChunks.find(it->first);
				} else {
					if (it->second.chunkObj != wiECS::INVALID_ENTITY) {
						wiScene::Scene& scene = wiScene::GetScene();
						wiECS::Entity meshEnt = scene.objects.GetComponent(it->second.chunkObj)->meshID;
						m.lock();
						scene.Entity_Remove(meshEnt);
						scene.Entity_Remove(it->second.chunkObj);
						for (size_t i = 0; i < it->second.trees.size(); i++) {
							scene.Entity_Remove(it->second.trees[i]);
						}
						m.unlock();
					}
					it++;
				}
			}
			m_visibleChunks.clear();
			world->cleaned = true;
		}
	}
	m_shutdown = 2;
}

void chunkLoader::addChunks(uint8_t threadNum) {
	while (m_shutdown < 2) {
		if (m_threadstate[threadNum] == 1) {
			cyImportant* world = settings::getWorld();
			cyImportant::chunkpos_t coords, zero;
			coords.x = m_threadChunkX[threadNum];
			coords.y = m_threadChunkY[threadNum];
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
					m_visibleChunks[coords] = chunkLoader::RenderChunk(chunk, chunkU, chunkL, chunkD, chunkR, coords.x, -coords.y);
				} else {
					//wiBackLog::post("CHunk not found");
					m_visibleChunks[coords].chunkObj = wiECS::INVALID_ENTITY;
				}
			} else {
				m_visibleChunks[coords].chunkObj = wiECS::INVALID_ENTITY;
			}
			if (m_threadstate[threadNum] != 99) {
				m_threadstate[threadNum] = 0;
			}
			
		} else {
			Sleep(10);
		}
	}
}

chunkLoader::chunkobjects_t chunkLoader::RenderChunk(const cyChunk& chunk, const cyChunk& northChunk, const cyChunk& eastChunk, const cyChunk& southChunk, const cyChunk& westChunk, const int32_t relX, const int32_t relY) {
	face_t tmpface;
	chunkobjects_t ret;
	wiECS::Entity entity = wiECS::INVALID_ENTITY;
	vector<face_t> faces;
	uint8_t stepsize	   = 1;
	uint16_t surfaceHeight = chunk.m_surfaceheight;
	if (eastChunk.m_surfaceheight < surfaceHeight)
		surfaceHeight = eastChunk.m_surfaceheight;
	if (westChunk.m_surfaceheight < surfaceHeight)
		surfaceHeight = westChunk.m_surfaceheight;
	if (northChunk.m_surfaceheight < surfaceHeight)
		surfaceHeight = northChunk.m_surfaceheight;
	if (southChunk.m_surfaceheight < surfaceHeight)
		surfaceHeight = southChunk.m_surfaceheight;
	if (surfaceHeight > 25)
		surfaceHeight -= 25;
	wiScene::Scene tmpScene;
	for (int_fast16_t z = 799; z > surfaceHeight; z -= stepsize) {
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
						wiECS::Entity objEnt	= tmpScene.Entity_CreateObject("torch");
						ObjectComponent& object = *tmpScene.objects.GetComponent(objEnt);
						LayerComponent& layer	= *tmpScene.layers.GetComponent(objEnt);
						TransformComponent* tf = tmpScene.transforms.GetComponent(objEnt);
						layer.layerMask = LAYER_TORCH;
						ret.trees.push_back(objEnt);
						try {
							
							switch (it->second) {
								case 0:
									object.meshID = cyBlocks::m_regMeshes.at(blocktype).mesh[0];
									tf->Translate(XMFLOAT3(relX + x / 2.0f + 0.248, z / 2.0f, relY + 16 - y / 2.0f));
									tf->RotateRollPitchYaw(XMFLOAT3(0, PI / 2, 0));
									break;
								case 1:
									object.meshID = cyBlocks::m_regMeshes.at(blocktype).mesh[0];
									tf->Translate(XMFLOAT3(relX + x / 2.0f - 0.248, z / 2.0f, relY + 16 - y / 2.0f));
									tf->RotateRollPitchYaw(XMFLOAT3(0, -PI/2, 0));
									break;
								case 2:
									object.meshID = cyBlocks::m_regMeshes.at(blocktype).mesh[0];
									tf->Translate(XMFLOAT3(relX + x / 2.0f, z / 2.0f, relY + 16 - y / 2.0f + 0.248));
									
									break;
								case 3:
									object.meshID = cyBlocks::m_regMeshes.at(blocktype).mesh[0];
									tf->Translate(XMFLOAT3(relX + x / 2.0f, z / 2.0f, relY + 16 - y / 2.0f - 0.248));
									tf->RotateRollPitchYaw(XMFLOAT3(0, PI, 0));
									break;
								case 4:
									object.meshID = cyBlocks::m_regMeshes.at(blocktype).mesh[1];
									tf->Translate(XMFLOAT3(relX + x / 2.0f, z / 2.0f - 0.135f, relY + 16 - y / 2.0f));
									break;
								case 5:
									object.meshID = cyBlocks::m_regMeshes.at(blocktype).mesh[1];
									tf->Translate(XMFLOAT3(relX + x / 2.0f, z / 2.0f +0.135f, relY + 16 - y / 2.0f));
									tf->RotateRollPitchYaw(XMFLOAT3(PI,0,0));
									break;
							}
							
							tf->UpdateTransform();
						}
						catch (...) {
						}
					}
					

					/*
					wiECS::Entity entity	= tmpScene.Entity_CreateEmitter("torchEmitter");
					wiScene::wiEmittedParticle* emitter = tmpScene.emitters.GetComponent(entity);
					wiScene::MaterialComponent* emitterMat = tmpScene.materials.GetComponent(entity);
					tf									   = tmpScene.transforms.GetComponent(entity);
					tf->Translate(XMFLOAT3(relX + x / 2.0f, z / 2.0f, relY + 16 - y / 2.0f));
					tf->UpdateTransform();
					emitter->SetSPHEnabled(true);
					emitter->SetMaxParticleCount(100);
					emitter->count			= 80.0f;
					emitter->normal_factor = 4.4f;
					emitter->scaleX		   = 2.5;
					emitter->life			= 2.5;
					emitter->size			= 0.17f;
					emitter->shaderType		= wiEmittedParticle::SOFT;
					emitter->scaleY		   = 2.5;
					emitter->random_factor = 0.3f;
					emitter->random_life = 0.01f;
					emitter->FIXED_TIMESTEP = 0.005f;
					emitter->SPH_e			= 26.5f;
					emitter->SPH_h			= 0.1f;
					emitter->SPH_K			= 0.1f;
					emitter->SPH_p0			= 14.0f;
					emitter->mass			= 0.1f;
					emitter->SetPaused(false);
					emitterMat->SetBaseColor(XMFLOAT4(1.0f,0.8f,0.0f,1.0f));
					emitterMat->baseColorMap = wiResourceManager::Load("images/fire.jpg");
					emitterMat->userBlendMode = BLENDMODE_ADDITIVE;*/
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
			mesh = meshGen::AddMesh(tmpScene, chunk.m_id, faces[0].material, &entity);
		} else {
			mesh = meshGen::AddMesh(tmpScene, chunk.m_id, cyBlocks::m_fallbackMat, &entity);
		}
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

			for (size_t i = 0; i != chunk.meshObjects.size(); i++) {

				if (chunk.meshObjects[i].scale.x > 2 || chunk.meshObjects[i].scale.y > 2 || chunk.meshObjects[i].scale.z > 2) {
					wiHelper::messageBox("Tree scaling out of bounds!", "Error!");
					uint32_t chID = chunk.m_id;
					wiBackLog::post("Weird tree data: ChunkID: ");
					wiBackLog::post(to_string(chID).c_str());
				} else {
					wiECS::Entity objEnt	= tmpScene.Entity_CreateObject("tree");
					ObjectComponent& object = *tmpScene.objects.GetComponent(objEnt);
					LayerComponent& layer	= *tmpScene.layers.GetComponent(objEnt);
					layer.layerMask			= LAYER_TREE;
					switch (chunk.meshObjects[i].type) {
						case 0:		//leaf trees (light wood)
							object.meshID = cyBlocks::m_treeMeshes[((chunk.meshObjects[i].pos.x + chunk.meshObjects[i].pos.y + chunk.meshObjects[i].pos.z) % 3)];
							break;
						case 1:		//needle trees (dark wood)
							object.meshID = cyBlocks::m_treeMeshes[3];
							break;
						case 2:	 //cactus
						case 3:
						case 4:
						case 5:
							object.meshID = cyBlocks::m_treeMeshes[4];
							break;
						case 6:	//desert grass
						case 7:
							object.meshID = cyBlocks::m_treeMeshes[5];
							break;
					}
					TransformComponent& tf	= *tmpScene.transforms.GetComponent(objEnt);
					tf.Scale(XMFLOAT3(chunk.meshObjects[i].scale.x * 0.8, chunk.meshObjects[i].scale.z * 0.8, chunk.meshObjects[i].scale.y * 0.8));
					tf.RotateRollPitchYaw(XMFLOAT3(0, chunk.meshObjects[i].Yaw, 0));
					tf.Translate(XMFLOAT3(relX + chunk.meshObjects[i].pos.x / 2.0f, chunk.meshObjects[i].pos.z / 2.0f - 0.3f, relY + 16 - chunk.meshObjects[i].pos.y / 2.0f));
					tf.UpdateTransform();
					ret.trees.push_back(objEnt);
				}
			}
			m.lock();
			wiScene::GetScene().Merge(tmpScene);
			m.unlock();
		}  //else	wiBackLog::post("Chunk has no blocks");
		ret.chunkObj = entity;
		return ret;
	}

	void chunkLoader::updateDisplayedChunks(void) {
		for (uint_fast16_t mesh = 0; mesh < VIEWDISTANCE; mesh++) {
		}
	}

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

	void chunkLoader::loadWorld(void) {
	}
