#include "stdafx.h"
#include "chunkLoader.h"

using namespace std;
using namespace wiScene;
extern mutex m;

void chunkLoader::spawnThreads(uint8_t numthreads) {
	if (numthreads <= wiJobSystem::GetThreadCount()) {
		m_numthreads = numthreads;
		for (uint8_t i = 0; i < m_numthreads; i++) {
			m_threadstate[i] = 0;
			m_thread[i]		 = thread(&chunkLoader::addChunks, this, i);
		}
		m_checkThread = thread(&chunkLoader::checkChunks, this);
	}
}

chunkLoader::~chunkLoader(void) {
	m_threadstate[0] = 99;
	m_checkThread.join();
	for (uint8_t i = 0; i < m_numthreads; i++) {
		m_threadstate[i] = 99;
		m_thread[i].join();
	}
}

void chunkLoader::checkChunks(void) {
	cyImportant::chunkpos_t ghostpos, coords, lastghostpos;
	cyImportant* world = settings::getWorld();
	bool changed	   = false;
	lastghostpos.x	   = -1000;
	lastghostpos.y	   = -1000;
	while (m_threadstate[0] != 99) {
		Sleep(10);
		if (!world->isStopped()) {
			CameraComponent cam = wiScene::GetCamera();
			XMFLOAT3 campos		= cam.Eye;
			XMFLOAT3 camlook	= cam.At;
			ghostpos.x			= (int32_t)(camlook.x * (settings::viewDist * 4) + campos.x);  //move the center for chunkloading a little towards the view direction, to load more chunks in this direction
			ghostpos.y			= -(int32_t)(camlook.z * (settings::viewDist * 4) + campos.z);
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
							m_visibleChunks[coords] = 0xFFFFFFFE;
							break;
						}
					}
				}
				uint32_t rges = settings::getViewDist();
				for (uint32_t r = 4; r <= rges; r = r + 4) {
					for (uint32_t i = 0; i < r; i = i + 1) {
						for (uint32_t ii = ((r - 4) > i) ? (1 + r - 4 - i) : 0; ii <= r - i; ii++) {
							if (i * i + ii * ii <= (rges * rges) / 2) {
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
												m_visibleChunks[coords] = 0xFFFFFFFE;
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
												m_visibleChunks[coords] = 0xFFFFFFFE;
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
												m_visibleChunks[coords] = 0xFFFFFFFE;
												break;
											}
										}
									}
									coords.x = ghostpos.x + i * 16;
									coords.y = ghostpos.y - ii * 16;
									if (m_visibleChunks.find(coords) == m_visibleChunks.end()) {
										uint32_t chunkID;
										changed = true;
										for (uint8_t thread = 0; thread < m_numthreads; thread++) {
											if (m_threadstate[thread] == 0) {
												m_threadChunkX[thread]	= coords.x;
												m_threadChunkY[thread]	= coords.y;
												m_threadstate[thread]	= 1;
												m_visibleChunks[coords] = 0xFFFFFFFE;
												break;
											}
										}
									}
								}
							}
						}
					}
				}

				for (unordered_map<cyImportant::chunkpos_t, wiECS::Entity>::iterator it = m_visibleChunks.begin(); it != m_visibleChunks.end();) {
					if (it->second != 0xFFFFFFFE) {
						cyImportant::chunkpos_t diff = it->first;
						diff						 = diff - ghostpos;
						if (diff.x * diff.x + diff.y * diff.y > (settings::getViewDist() * settings::getViewDist() * 16 * 16)) {
							if (it->second != wiECS::INVALID_ENTITY) {
								m.lock();
								wiScene::GetScene().Entity_Remove(wiScene::GetScene().objects.GetComponent(it->second)->meshID);
								wiScene::GetScene().Entity_Remove(it->second);
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
			for (unordered_map<cyImportant::chunkpos_t, wiECS::Entity>::iterator it = m_visibleChunks.begin(); it != m_visibleChunks.end();) {
				if (it->second == 0xFFFFFFFE) {	 //wait for the chunk to be finished
					Sleep(1);
					it = m_visibleChunks.find(it->first);
				} else {
					if (it->second != wiECS::INVALID_ENTITY) {
						m.lock();
						wiScene::GetScene().Entity_Remove(wiScene::GetScene().objects.GetComponent(it->second)->meshID);
						wiScene::GetScene().Entity_Remove(it->second);
						m.unlock();
					}
					it++;
				}
			}
			m_visibleChunks.clear();
			world->cleaned = true;
		}
	}
	for (uint8_t i = 0; i < m_numthreads; i++) {
		m_threadstate[i] = 99;
	}
}

void chunkLoader::addChunks(uint8_t threadNum) {
	while (m_threadstate[threadNum] != 99) {
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
					if (m_visibleChunks.size() > 2) {
						//wiScene::GetScene().impostors.Create(wiScene::GetScene().objects.GetComponent(m_visibleChunks[coords])->meshID).swapInDistance = 30;
					}

				} else {
					//wiBackLog::post("CHunk not found");
					m_visibleChunks[coords] = wiECS::INVALID_ENTITY;
				}
			} else {
				m_visibleChunks[coords] = wiECS::INVALID_ENTITY;
			}
			if (m_threadstate[threadNum] != 99) {
				m_threadstate[threadNum] = 0;
			}
			
		} else {
			Sleep(10);
		}
	}
}

wiECS::Entity chunkLoader::RenderChunk(const cyChunk& chunk, const cyChunk& northChunk, const cyChunk& eastChunk, const cyChunk& southChunk, const cyChunk& westChunk, const int32_t relX, const int32_t relY) {
	face_t tmpface;
	wiECS::Entity entity = wiECS::INVALID_ENTITY;
	vector<face_t> faces;
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
	for (int_fast16_t z = 799; z > surfaceHeight; z--) {
		for (uint_fast8_t x = 0; x < 32; x++) {
			for (uint_fast8_t y = 0; y < 32; y++) {
				uint8_t blocktype = (uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * z);
				if (cyBlocks::m_regBlockTypes[blocktype] <= cyBlocks::BLOCKTYPE_ALPHA) {
					uint8_t neighbour[6];
					if (z < 799)
						neighbour[0] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * (z + 1))];
					else
						neighbour[0] = cyBlocks::BLOCKTYPE_VOID;

					if (z > 1)
						neighbour[1] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * (z - 1))];
					else
						neighbour[1] = cyBlocks::BLOCKTYPE_VOID;

					if (y < 31)
						neighbour[4] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * (y + 1) + 32 * 32 * z)];
					else
						neighbour[4] = cyBlocks::m_regBlockTypes[(uint8_t) * (northChunk.m_chunkdata + 4 + x + 32 * 32 * z)];

					if (y > 0)
						neighbour[5] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * (y - 1) + 32 * 32 * z)];
					else
						neighbour[5] = cyBlocks::m_regBlockTypes[(uint8_t) * (southChunk.m_chunkdata + 4 + x + 32 * 31 + 32 * 32 * z)];

					if (x < 31)
						neighbour[3] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 5 + x + 32 * y + 32 * 32 * z)];
					else
						neighbour[3] = cyBlocks::m_regBlockTypes[(uint8_t) * (eastChunk.m_chunkdata + 4 + 32 * y + 32 * 32 * z)];

					if (x > 0)
						neighbour[2] = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 3 + x + 32 * y + 32 * 32 * z)];
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
							if (neighbour[ft] > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
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
				}
			}
		}
	}
	if (faces.size()) {
		SimplexNoise noise;
		sort(faces.begin(), faces.end());
		MeshComponent* mesh;
		wiScene::Scene tmpScene;
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
						meshGen::AddFaceTop(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
						break;
					case cyBlocks::FACE_BOTTOM:
						meshGen::AddFaceBottom(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
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
						meshGen::AddBillboard(mesh, faces[i].x, faces[i].y, faces[i].z);
						break;
				}
			}
			mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;

			mesh->SetDynamic(false);
			mesh->CreateRenderData();
			m.lock();
			wiScene::GetScene().Merge(tmpScene);
			m.unlock();
		}  //else
		//	wiBackLog::post("Chunk has no blocks");
		return entity;
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
