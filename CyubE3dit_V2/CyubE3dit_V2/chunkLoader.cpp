#include "stdafx.h"
#include "chunkLoader.h"
#include "cySchematic.h"

using namespace std;
using namespace wiScene;
extern mutex m;

std::vector<cyImportant::chunkpos_t> chunkLoader::maskedChunks;
std::vector<cyImportant::chunkpos_t> chunkLoader::pendingChunks;
mutex chunkLoader::maskMutex;

void chunkLoader::spawnThreads(uint8_t numthreads) {
	m_shutdown = 0;
	if (numthreads > cyImportant::MAX_THREADS) {
		numthreads = cyImportant::MAX_THREADS;
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

void chunkLoader::clearMaskedChunk(void) {
	maskMutex.lock();
	pendingChunks = maskedChunks;
	maskedChunks.clear();
	maskMutex.unlock();
}

void chunkLoader::checkChunks(void) {
	cyImportant::chunkpos_t ghostpos, coords, lastghostpos, exactGhostPos;
	cyImportant* world	  = settings::getWorld();
	uint32_t changed	  = 0;
	lastghostpos.x		  = -1000;
	lastghostpos.y		  = -1000;
	wiScene::Scene& scene = wiScene::GetScene();
	while (m_shutdown == 0) {

		maskMutex.lock();
		for (size_t i = 0; i < maskedChunks.size(); i++) {
			auto it = m_visibleChunks.find(maskedChunks[i]);
			if (it != m_visibleChunks.end()) {
				if (it->second.chunkObj != wiECS::INVALID_ENTITY && it->second.chunkObj != 0xFFFFFFFE) {
					m.lock();
					wiScene::Scene& scene		  = wiScene::GetScene();
					wiScene::ObjectComponent* obj = scene.objects.GetComponent(it->second.chunkObj);
					if (obj != nullptr) {
						wiECS::Entity meshEnt = obj->meshID;
						scene.Component_RemoveChildren(it->second.chunkObj);
						scene.Entity_Remove(it->second.chunkObj);
						scene.Entity_Remove(meshEnt);
						it->second.chunkObj = wiECS::INVALID_ENTITY;
					}
					m.unlock();
				}
			}
		}
		for (size_t i = 0; i < pendingChunks.size(); i++) {
			auto it = m_visibleChunks.find(pendingChunks[i]);
			if (it != m_visibleChunks.end()) {
				while (!employThread(pendingChunks[i])) {
					Sleep(4);
				}
			}
		}
		pendingChunks.clear();
		maskMutex.unlock();
		Sleep(5);
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
										coords.x	  = ghostpos.x + i * 16;
										coords.y	  = ghostpos.y + ii * 16;
										auto visChunk = m_visibleChunks.find(coords);
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
										coords.x	  = ghostpos.x - i * 16;
										coords.y	  = ghostpos.y + ii * 16;
										auto visChunk = m_visibleChunks.find(coords);
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
						if (pendingChunks.size()) {
							break;
						} else if (changed > 100) {
							changed	   = 1;
							ghostpos.x = (int32_t)(campos.x);
							ghostpos.y = -(int32_t)(campos.z);
							ghostpos.x &= 0xFFFFFFC0;
							ghostpos.y &= 0xFFFFFFC0;
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
					Sleep(20);
				}
			} else {
				Sleep(50);
			}
		} else {  //world stopped --> clear all existing chunks (if there are any)
			while (m_visibleChunks.size()) {
				removeFarChunks(exactGhostPos, true);
			}
			m_visibleChunks.clear();
			lastghostpos.x = -1000;
			lastghostpos.y = -1000;
			world->cleaned = true;
		}
	}
	m_shutdown = 2;
}

void chunkLoader::addChunks(uint8_t threadNum) {
	chunkobjects_t chunkObj;
	uint32_t chunkID;

	while (m_shutdown < 2) {
		if (m_threadstate[threadNum] == THREAD_BUSY) {
			cyImportant* world = settings::getWorld();
			cyImportant::chunkpos_t coords, zero;
			coords = m_threadChunkPos[threadNum];
			if (world->isValid()) {
				zero.x = (uint32_t)(world->m_playerpos.x / 100);
				zero.y = (uint32_t)(world->m_playerpos.y / 100);

				if (world->getChunkID(coords.x + zero.x, coords.y + zero.y, &chunkID)) {
					cyChunk chunk;
					cyChunk chunkL;
					cyChunk chunkR;
					cyChunk chunkU;
					cyChunk chunkD;
					chunk.loadChunk(world->db[threadNum], chunkID);
					if (world->getChunkID(coords.x + 16 + zero.x, coords.y + zero.y, &chunkID))
						chunkL.loadChunk(world->db[threadNum], chunkID, true);
					else
						chunkL.airChunk();
					if (world->getChunkID(coords.x - 16 + zero.x, coords.y + zero.y, &chunkID))
						chunkR.loadChunk(world->db[threadNum], chunkID, true);
					else
						chunkR.airChunk();
					if (world->getChunkID(coords.x + zero.x, coords.y + zero.y + 16, &chunkID))
						chunkU.loadChunk(world->db[threadNum], chunkID, true);
					else
						chunkU.airChunk();
					if (world->getChunkID(coords.x + zero.x, coords.y + zero.y - 16, &chunkID))
						chunkD.loadChunk(world->db[threadNum], chunkID, true);
					else
						chunkD.airChunk();
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
	for (auto it = m_visibleChunks.begin(); it != m_visibleChunks.end();) {
		if (it->second.chunkObj != 0xFFFFFFFE) {
			cyImportant::chunkpos_t diff = it->first;
			diff						 = diff - ghostpos;
			float dist					 = (diff.x * diff.x + diff.y * diff.y);
			float viewDist				 = (settings::getViewDist() * settings::getViewDist() * 16 * 16);
			if (cleanAll || dist > viewDist) {	//remove far chunks
				if (it->second.chunkObj != wiECS::INVALID_ENTITY) {
					wiScene::ObjectComponent* obj = scene.objects.GetComponent(it->second.chunkObj);
					wiECS::Entity meshEnt		  = obj->meshID;
					m.lock();
					scene.Component_RemoveChildren(it->second.chunkObj);
					scene.Entity_Remove(it->second.chunkObj);
					scene.Entity_Remove(meshEnt);
					m.unlock();
					it = m_visibleChunks.erase(it);
				} else {
					it = m_visibleChunks.erase(it);
				}
			} else {  //Manage LODs
				if (it->second.chunkObj != wiECS::INVALID_ENTITY) {
					if (dist > viewDist / 8) {
						if (it->second.lod != LOD_MAX) {
							it->second.lod = LOD_MAX;
							//wiScene::ObjectComponent* mesh = wiScene::GetScene().objects.GetComponent(it->second.chunkObj); does not really help with framerate... ?
							//mesh->SetCastShadow(false);
							wiScene::ObjectComponent* mesh = nullptr;
							for (size_t iii = 0; iii < it->second.meshes.size(); iii++) {
								m.lock();
								mesh = wiScene::GetScene().objects.GetComponent(it->second.meshes[iii]);
								if (mesh != nullptr) {
									mesh->SetCastShadow(false);
									mesh->SetRenderable(false);
								}
								m.unlock();
							}
						}
					} else if (dist > viewDist / 16) {
						if (it->second.lod != LOD_MAX - 1) {
							it->second.lod = LOD_MAX - 1;
							//wiScene::ObjectComponent* mesh = wiScene::GetScene().objects.GetComponent(it->second.chunkObj);	does not really help with framerate... ?
							//mesh->SetCastShadow(true);
							wiScene::ObjectComponent* mesh = nullptr;
							for (size_t iii = 0; iii < it->second.meshes.size(); iii++) {
								m.lock();
								mesh = wiScene::GetScene().objects.GetComponent(it->second.meshes[iii]);
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
						//wiScene::ObjectComponent* mesh = wiScene::GetScene().objects.GetComponent(it->second.chunkObj);
						//mesh->SetCastShadow(true);
						wiScene::ObjectComponent* mesh = nullptr;
						for (size_t iii = 0; iii < it->second.meshes.size(); iii++) {
							m.lock();
							mesh = wiScene::GetScene().objects.GetComponent(it->second.meshes[iii]);
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
						for (size_t i = 0; i < wiScene::GetScene().objects.GetCount(); i++)
						{
							if (wiScene::GetScene().objects[i].parentObject == m_threadstate[thread] && wiScene::GetScene().objects.GetEntity(i) != m_threadstate[thread])
							{
								wiECS::Entity entity = wiScene::GetScene().objects.GetEntity(i);
								m_visibleChunks[m_threadChunkPos[thread]].meshes.push_back(entity);
							}
						}
						m.unlock();
						m_threadstate[thread] = THREAD_IDLE;
					}
				}
			}
		}
	}
}

chunkLoader::chunkobjects_t chunkLoader::RenderChunk(cyChunk& chunk, const cyChunk& northChunk, const cyChunk& eastChunk, const cyChunk& southChunk, const cyChunk& westChunk, const int32_t relX, const int32_t relY, const bool _lod) {
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
		else
			surfaceHeight = 0;
	} else {

		if (chunk.m_lowestZ > 2)
			surfaceHeight = chunk.m_lowestZ - 2;
		else
			surfaceHeight = 0;
	}

	wiScene::Scene tmpScene;

	for (int_fast16_t z = min(chunk.m_highestZ + 20, 799); z > surfaceHeight; z -= stepsize) {
		for (uint_fast8_t x = 0; x < 32; x += stepsize) {
			for (uint_fast8_t y = 0; y < 32; y += stepsize) {
				uint8_t blocktype = (uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * z);
				if (cyBlocks::m_regBlockTypes[blocktype] <= cyBlocks::BLOCKTYPE_ALPHA) {
					uint8_t neighbour[6];
					if (z < 799)
						neighbour[cyBlocks::FACE_TOP] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * (z + stepsize))];
					else
						neighbour[cyBlocks::FACE_TOP] = cyBlocks::BLOCKTYPE_VOID;

					if (z > 1)
						neighbour[cyBlocks::FACE_BOTTOM] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * (z - stepsize))];
					else
						neighbour[cyBlocks::FACE_BOTTOM] = cyBlocks::BLOCKTYPE_VOID;

					if (y < 32 - stepsize)
						neighbour[cyBlocks::FACE_BACK] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * (y + stepsize) + 32 * 32 * z)];
					else
						neighbour[cyBlocks::FACE_BACK] = cyBlocks::m_regBlockTypes[(uint8_t) * (northChunk.m_chunkdata + 4 + x + 32 * 32 * z)];

					if (y >= 0 + stepsize)
						neighbour[cyBlocks::FACE_FRONT] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * (y - stepsize) + 32 * 32 * z)];
					else
						neighbour[cyBlocks::FACE_FRONT] = cyBlocks::m_regBlockTypes[(uint8_t) * (southChunk.m_chunkdata + 4 + x + 32 * 31 + 32 * 32 * z)];

					if (x < 32 - stepsize)
						neighbour[cyBlocks::FACE_RIGHT] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + stepsize + x + 32 * y + 32 * 32 * z)];
					else
						neighbour[cyBlocks::FACE_RIGHT] = cyBlocks::m_regBlockTypes[(uint8_t) * (eastChunk.m_chunkdata + 4 + 32 * y + 32 * 32 * z)];

					if (x >= 0 + stepsize)
						neighbour[cyBlocks::FACE_LEFT] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 - stepsize + x + 32 * y + 32 * 32 * z)];
					else
						neighbour[cyBlocks::FACE_LEFT] = cyBlocks::m_regBlockTypes[(uint8_t) * (westChunk.m_chunkdata + 4 + 31 + 32 * y + 32 * 32 * z)];

					uint8_t antitile = 0;

					if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_MOD) {
						cyChunk::blockpos_t pos(x, y, z);
						auto cBlock = chunk.m_cBlocks.find(pos);
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
					mesh.pos.x	 = x / 2.0f;
					mesh.pos.y	 = y / 2.0f;
					mesh.pos.z	 = z / 2.0f;
					mesh.scale.x = (0.8f + wiRandom::getRandom(20) / 50.f);
					mesh.scale.y = mesh.scale.x;
					mesh.scale.z = mesh.scale.x;
					mesh.type	 = blocktype;
					chunk.addMesh(mesh);
				} else if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_BILLBOARD) {
					
						tmpface.x		 = relX + x / 2.0f;
						tmpface.y		 = relY + 16 - y / 2.0f;
						tmpface.z		 = z / 2.0f;
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][0];
						if (cyBlocks::m_regBlockMats[blocktype][1] != wiECS::INVALID_ENTITY) {
							int rnd = wiRandom::getRandom(20);
							if (rnd > 17)
								tmpface.material = cyBlocks::m_regBlockMats[blocktype][1];
							else if (rnd > 13) {
								if (cyBlocks::m_regBlockMats[blocktype][2] != wiECS::INVALID_ENTITY)
									tmpface.material = cyBlocks::m_regBlockMats[blocktype][2];
							}
						}
						tmpface.face = cyBlocks::FACE_BILLBOARD;
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
		float len = 0, x, z, y;
		SimplexNoise noise;
		stable_sort(faces.begin(), faces.end());
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
					len = 0;
					x	= faces[i].x;
					y	= faces[i].y;
					z	= faces[i].z;

					if (i < faces.size() - 1) {
						i++;
						while (faces[i].face == cyBlocks::FACE_TOP && faces[i].x == x && faces[i].z == z && faces[i].material == currMat && y - faces[i].y < 0.9) {
							len += 0.5;
							y = faces[i].y;
							if (++i >= faces.size())
								break;
						}
						i--;
					}

					meshGen::AddFaceTop(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile, len);
					break;
				case cyBlocks::FACE_BOTTOM:
					len = 0;
					x	= faces[i].x;
					y	= faces[i].y;
					z	= faces[i].z;
					if (i < faces.size() - 1) {
						i++;
						while (faces[i].face == cyBlocks::FACE_TOP && faces[i].x == x && faces[i].z == z && faces[i].material == currMat && y - faces[i].y < 0.9) {
							len += 0.5;
							y = faces[i].y;
							if (++i >= faces.size())
								break;
						}
						i--;
					}
					meshGen::AddFaceBottom(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile, len);
					break;
				case cyBlocks::FACE_LEFT:
					meshGen::AddFaceLeft(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
					break;
				case cyBlocks::FACE_RIGHT:
					meshGen::AddFaceRight(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
					break;
				case cyBlocks::FACE_BACK:
					meshGen::AddFaceBack(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
					break;
				case cyBlocks::FACE_FRONT:
					meshGen::AddFaceFront(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
					break;
				case cyBlocks::FACE_BILLBOARD:
					if (settings::rendermask & LAYER_FOILAGE) {
					meshGen::AddBillboard(mesh, faces[i].x, faces[i].y, faces[i].z);
					}
					break;
			}
		}
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
		mesh->SetDynamic(false);
		mesh->CreateRenderData();
		//m.lock();
		//wiScene::GetScene().Merge(tmpScene);
		//tmpScene.Clear();
		//m.unlock();
		ObjectComponent* chunkObj = tmpScene.objects.GetComponent(entity);
		if (settings::rendermask & LAYER_TREE) {
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
					tf.Translate(XMFLOAT3(relX + chunk.trees[i].pos.x / 2., chunk.trees[i].pos.z / 2. - 0.25, relY + 16 - chunk.trees[i].pos.y / 2.));
					tf.Scale(XMFLOAT3(chunk.trees[i].scale.x, chunk.trees[i].scale.z, chunk.trees[i].scale.y));
					switch (chunk.trees[i].type) {
						case 0:	 //leaf trees (light wood)
							object.meshID = cyBlocks::m_treeMeshes[((chunk.trees[i].pos.x + chunk.trees[i].pos.y + chunk.trees[i].pos.z) % 3)];
							break;
						case 1:	 //needle trees (dark wood)
							object.meshID = cyBlocks::m_treeMeshes[3];
							break;
						case 2:	 //cactus
							tf.Scale(XMFLOAT3(chunk.trees[i].scale.x * 3, chunk.trees[i].scale.z * 1.5, chunk.trees[i].scale.y * 3));
							object.meshID = cyBlocks::m_treeMeshes[6];	//desert bush
							break;
						case 3:
						case 4:
						case 5:
							object.meshID = cyBlocks::m_treeMeshes[4];
							break;
						case 6:	 //desert grass 6,7
						case 7:
							object.meshID = cyBlocks::m_treeMeshes[5];
						default:
							object.meshID = cyBlocks::m_treeMeshes[5];
							break;
					}

					tf.RotateRollPitchYaw(XMFLOAT3(0, chunk.trees[i].yaw, 0));
					tf.UpdateTransform();
					object.parentObject = entity;
					if (_lod == true) {
						object.SetRenderable(false);  //load the Mesh with lowest LOD to prevent flicker
						object.SetCastShadow(false);
					}
					ret.meshes.push_back(objEnt);
					//tmpScene.Component_Attach(objEnt, entity);
				}
			}
			//m.lock();
			//wiScene::GetScene().Merge(tmpScene);
			//tmpScene.Clear();
			//m.unlock();
		}
		if (settings::rendermask & LAYER_TORCH) {
			placeTorches(torches, relX, relY, tmpScene, entity);
			//m.lock();
			//wiScene::GetScene().Merge(tmpScene);
			//tmpScene.Clear();
			//m.unlock();
		}
		if (settings::rendermask & LAYER_MESH) {
			placeMeshes(chunk, relX, relY, tmpScene, entity);
			//m.lock();
			//wiScene::GetScene().Merge(tmpScene);
			//m.unlock();
		}
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
				wiECS::Entity mesh = cyBlocks::m_regMeshes.at(chunk.meshObjects[i].type).mesh[0];  //invalid unordered map for .at() when large schematic is loaded
				if (mesh) {
					wiECS::Entity objEnt	= tmpScene.Entity_CreateObject("mesh");
					ObjectComponent& object = *tmpScene.objects.GetComponent(objEnt);
					object.meshID			= mesh;
					LayerComponent& layer	= *tmpScene.layers.GetComponent(objEnt);
					layer.layerMask			= LAYER_MESH;
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
		LightComponent* light				= nullptr;
		wiECS::Entity meshID				= 0;
		wiScene::wiEmittedParticle* fire	= nullptr;
		wiScene::MaterialComponent* dustmat = nullptr;
		wiScene::WeatherComponent* weather	= wiScene::GetScene().weathers.GetComponent(wiScene::GetScene().weathers.GetEntity(0));
		TransformComponent transform;
		TransformComponent* tf;
		XMFLOAT3 color((float)cyBlocks::m_regBlockFlags[torch.ID][0] / 255.f, (float)cyBlocks::m_regBlockFlags[torch.ID][1] / 255.f, (float)cyBlocks::m_regBlockFlags[torch.ID][2] / 255.f);
		XMFLOAT3 lightPos;
		try {
			switch (torch.rotation) {
				case 0:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f + 0.248, torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(0, PI / 2, 0));
					lightPos = XMFLOAT3(relX + torch.x / 2.0f + 0.1, torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f);

					break;
				case 1:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f - 0.248, torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(0, -PI / 2, 0));
					lightPos = XMFLOAT3(relX + torch.x / 2.0f - 0.1, torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f);

					break;
				case 2:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f + 0.248));
					lightPos = XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f + 0.1);

					break;
				case 3:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f - 0.248));
					transform.RotateRollPitchYaw(XMFLOAT3(0, PI, 0));
					lightPos = XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f - 0.1);

					break;
				case 4:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[1];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f - 0.135f, relY + 16 - torch.y / 2.0f));
					lightPos = XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f + 0.08, relY + 16 - torch.y / 2.0f);

					break;
				default:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[1];
					transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f + 0.135f, relY + 16 - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(PI, 0, 0));
					lightPos = XMFLOAT3(relX + torch.x / 2.0f, torch.z / 2.0f - 0.08, relY + 16 - torch.y / 2.0f);

					break;
			}
			if (settings::torchlights) {
				lightEnt = tmpScene.Entity_CreateLight("TL", lightPos, color, 5, 4);
				light	 = tmpScene.lights.GetComponent(lightEnt);
				light->SetCastShadow(true);
				light->parentObject = parent;
				if (settings::torchlights == false) {
					light->SetStatic(true);
				}
			}
			/* eats way too many memory with  lots of torches
			//fire
			if (settings::rendermask & LAYER_EMITTER) {
				lightEnt		 = tmpScene.Entity_CreateEmitter("TE", lightPos);
				fire			 = tmpScene.emitters.GetComponent(lightEnt);
				fire->layerMask	 = LAYER_EMITTER;
				fire->shaderType = wiScene::wiEmittedParticle::SOFT;
				fire->SetMaxParticleCount(300);
				fire->life			   = 2.0;
				fire->random_life	   = 0.3f;
				fire->random_factor	   = 0.2f;
				fire->count			   = 40;
				fire->normal_factor	   = 0.15f;
				fire->size			   = .1f;
				fire->scaleX		   = 0.5;
				fire->scaleY		   = 0.5;
				fire->motionBlurAmount = 0.0f;
				fire->drag			   = 0.99f;
				fire->random_color	   = 0.8;
				fire->gravity		   = XMFLOAT3(weather->windDirection.x * weather->windSpeed / 10, 0.1, weather->windDirection.z * weather->windSpeed / 10);
				fire->velocity		   = XMFLOAT3(0, 0.05, 0);
				fire->parentObject	   = parent;
				//fire->SetPaused(true);
				dustmat														= tmpScene.materials.GetComponent(lightEnt);
				dustmat->textures[MaterialComponent::BASECOLORMAP].resource = cyBlocks::emitter_fire_material;
				dustmat->userBlendMode										= BLENDMODE_ADDITIVE;
				dustmat->SetBaseColor(XMFLOAT4(color.x, color.y, color.z, 0.3));
				dustmat->SetEmissiveColor(XMFLOAT4(1, 1, 1, 1));
				dustmat->SetEmissiveStrength(15.f);

				//flare
				lightEnt		 = tmpScene.Entity_CreateEmitter("TE", lightPos);
				fire			 = tmpScene.emitters.GetComponent(lightEnt);
				fire->layerMask	 = LAYER_EMITTER;
				fire->shaderType = wiScene::wiEmittedParticle::SOFT_DISTORTION;
				fire->SetMaxParticleCount(300);
				fire->life			= 2.0;
				fire->random_life	= 0.3f;
				fire->random_factor = 0.2f;
				fire->rotation		= 0.01f;
				fire->count			= 6;
				fire->normal_factor = 0.2f;
				fire->SetVolumeEnabled(true);
				fire->size			   = .015f;
				fire->scaleX		   = 20.;
				fire->scaleY		   = 20.;
				fire->motionBlurAmount = 0.5f;
				fire->drag			   = 0.99f;
				fire->gravity		   = XMFLOAT3(weather->windDirection.x * weather->windSpeed / 10, 0.2, weather->windDirection.z * weather->windSpeed / 10);
				fire->velocity		   = XMFLOAT3(0, 0, 0);
				fire->parentObject	   = parent;
				//fire->SetPaused(true);
				dustmat														= tmpScene.materials.GetComponent(lightEnt);
				dustmat->textures[MaterialComponent::BASECOLORMAP].resource = cyBlocks::emitter_flare_material;
				dustmat->userBlendMode										= BLENDMODE_ADDITIVE;
				dustmat->SetBaseColor(XMFLOAT4(1, 1, 1, 0.01));
				tf = tmpScene.transforms.GetComponent(lightEnt);
				tf->Scale(XMFLOAT3(0.05, 0.05, 0.07));
				tf->UpdateTransform();

				//smoke
				lightPos.y += 0.25;
				lightPos.x += weather->windDirection.x * weather->windSpeed / 6;
				lightPos.z += weather->windDirection.z * weather->windSpeed / 6;
				lightEnt		 = tmpScene.Entity_CreateEmitter("TE", lightPos);
				fire			 = tmpScene.emitters.GetComponent(lightEnt);
				fire->layerMask	 = LAYER_EMITTER;
				fire->shaderType = wiScene::wiEmittedParticle::SOFT;
				fire->SetMaxParticleCount(500);
				fire->life			   = 3.0;
				fire->random_life	   = 0.7f;
				fire->random_factor	   = 0.7f;
				fire->count			   = 40;
				fire->normal_factor	   = 0.7f;
				fire->size			   = .1f;
				fire->scaleX		   = 1.;
				fire->scaleY		   = 1.;
				fire->motionBlurAmount = 1.70f;
				fire->drag			   = 0.94f;
				fire->random_color	   = 0.1;
				fire->gravity		   = XMFLOAT3(weather->windDirection.x * weather->windSpeed, -0.04, weather->windDirection.z * weather->windSpeed);
				fire->velocity		   = XMFLOAT3(weather->windDirection.x * weather->windSpeed / 10, 0.4, weather->windDirection.z * weather->windSpeed / 10);
				fire->parentObject	   = parent;
				fire->SetDepthCollisionEnabled(true);
				//fire->SetPaused(true);
				dustmat														= tmpScene.materials.GetComponent(lightEnt);
				dustmat->textures[MaterialComponent::BASECOLORMAP].resource = cyBlocks::emitter_smoke_material;
				dustmat->userBlendMode										= BLENDMODE_ADDITIVE;
				dustmat->SetBaseColor(XMFLOAT4(.1, .1, 0.1, 0.35));
				//dustmat->SetEmissiveColor(XMFLOAT4(1, 1, 1, 1));
				//dustmat->SetEmissiveStrength(15.f);
			}*/
			wiECS::Entity objEnt	= tmpScene.Entity_CreateObject("torch");
			ObjectComponent& object = *tmpScene.objects.GetComponent(objEnt);
			LayerComponent& layer	= *tmpScene.layers.GetComponent(objEnt);
			layer.layerMask			= LAYER_TORCH;
			tf						= tmpScene.transforms.GetComponent(objEnt);
			object.meshID			= meshID;
			object.emissiveColor	= XMFLOAT4(color.x, color.y, color.z, 1.0f);
			*tf						= transform;
			tf->UpdateTransform();
			object.parentObject = parent;
		}
		catch (...) {
		}
	}
}

inline bool chunkLoader::employThread(cyImportant::chunkpos_t coords) {
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
				m_threadstate[thread] = THREAD_IDLE;
			}
			m_threadChunkPos[thread]		 = coords;
			m_threadstate[thread]			 = THREAD_BUSY;
			m_visibleChunks[coords].chunkObj = 0xFFFFFFFE;
			return true;
		}
	}
	return false;
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
