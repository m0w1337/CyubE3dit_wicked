#include "stdafx.h"
#include "chunkLoader.h"

using namespace std;
using namespace wiScene;
extern mutex m;

void chunkLoader::spawnThreads(uint8_t numthreads) {
	if (numthreads < 32) {
		m_numthreads  = numthreads;
		for (uint8_t i = 0; i < m_numthreads; i++) {
			m_threadstate[i] = 0;
			m_thread[i]		 = thread(&chunkLoader::addChunks, this, i);
		}
		m_checkThread = thread(&chunkLoader::checkChunks, this);
	}
}

chunkLoader::~chunkLoader(void) {
	for (uint8_t i = 0; i < m_numthreads; i++) {
		m_threadstate[i] = 99;
		m_thread[i].join();
	}
	m_checkThread.join();
}

void chunkLoader::checkChunks(void) {
	cyImportant::chunkpos_t ghostpos, coords, lastghostpos;
	bool changed = false;
	while (m_threadstate[0] != 99) {
		CameraComponent cam = wiScene::GetCamera();
		XMFLOAT3 campos		= cam.Eye;
		ghostpos.x			= (int32_t)campos.x;
		ghostpos.y			= -(int32_t)campos.z;
		ghostpos.x &= 0xFFFFFFE0;
		ghostpos.y &= 0xFFFFFFE0;
		if (lastghostpos.x != ghostpos.x || lastghostpos.y != ghostpos.y) {
			coords = ghostpos;
			if (m_visibleChunks.find(coords) == m_visibleChunks.end()) {
				uint32_t chunkID;
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
								coords.x = ghostpos.x - i * 16;
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
								//context.fillRect(x + 4 * i, y + 4 * ii, 4, 4);
								//context.fillRect(x - 4 * i, y - 4 * ii, 4, 4);
							}
							if (i && ii) {
								coords.x = ghostpos.x - i * 16;
								coords.y = ghostpos.y + ii * 16;
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
								//context.fillRect(x - 4 * i, y + 4 * ii, 4, 4);
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
								//context.fillRect(x + 4 * i, y - 4 * ii, 4, 4);
							}
						}
					}
				}
			}
			/*
			for (uint32_t i = 0; i < settings::getViewDist(); i++) {
				coords = spiral(i);
				coords = coords + ghostpos;
				if (m_visibleChunks.find(coords) == m_visibleChunks.end()) {
					uint32_t chunkID;
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
			}*/
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
						m_visibleChunks.erase(it);
					}
				}
				if (it != m_visibleChunks.end())
					it++;
			}
			/*
			for (auto& x : m_visibleChunks) {
				if (x.second != 0xFFFFFFFE) {
					cyImportant::chunkpos_t diff = x.first;
					diff						 = diff - ghostpos;
					if (diff.x * diff.x + diff.y * diff.y > (settings::getViewDist() * settings::getViewDist()*16*16)) {
						if (x.second != wiECS::INVALID_ENTITY) {
							m.lock();
							wiScene::GetScene().Entity_Remove(wiScene::GetScene().objects.GetComponent(x.second)->meshID);
							wiScene::GetScene().Entity_Remove(x.second);
							m.unlock();
						}
						m_visibleChunks.erase(x.first);
						break;
					}
				}
			}*/
			Sleep(10);
			if (changed == false) {
				lastghostpos = ghostpos;
			}
				
		}else
			Sleep(200);
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
				zero.x = world->m_playerpos.x / 100;
				zero.y = world->m_playerpos.y / 100;

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
					//wiScene::GetScene().impostors.Create(m_visibleChunks[coords]).swapInDistance = 30;
				} else {
					wiBackLog::post("CHunk not found");
					m_visibleChunks[coords] = wiECS::INVALID_ENTITY;
				}
			} else {
				m_visibleChunks[coords] = wiECS::INVALID_ENTITY;
			}
			m_threadstate[threadNum] = 0;
		} else {
			Sleep(10);
		}
	}
}

wiECS::Entity chunkLoader::RenderChunk(const cyChunk& chunk, const cyChunk& northChunk, const cyChunk& eastChunk, const cyChunk& southChunk, const cyChunk& westChunk, const int32_t relX, const int32_t relY) {
	face_t tmpface;
	wiECS::Entity entity = wiECS::INVALID_ENTITY;
	vector<face_t> faces;
	for (uint_fast8_t x = 0; x < 32; x++) {
		for (uint_fast8_t y = 0; y < 32; y++) {
			for (uint_fast16_t z = 0; z < 800; z++) {
				uint8_t blocktype = (uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * z);
				if (cyBlocks::m_regBlockTypes[blocktype] < cyBlocks::BLOCKTYPE_SOLID_THRESH) {
					uint8_t upperB, lowerB, leftB, rightB, frontB, backB;
					if (z < 799)
						upperB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * (z + 1))];
					else
						upperB = cyBlocks::BLOCKTYPE_VOID;

					if (z > 1)
						lowerB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * (z - 1))];
					else
						lowerB = cyBlocks::BLOCKTYPE_VOID;

					if (y < 31)
						frontB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * (y + 1) + 32 * 32 * z)];
					else
						frontB = cyBlocks::m_regBlockTypes[(uint8_t) * (northChunk.m_chunkdata + 4 + x + 32 * 32 * z)];

					if (y > 0)
						backB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * (y - 1) + 32 * 32 * z)];
					else
						backB = cyBlocks::m_regBlockTypes[(uint8_t) * (southChunk.m_chunkdata + 4 + x + 32 * 31 + 32 * 32 * z)];

					if (x < 31)
						rightB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 5 + x + 32 * y + 32 * 32 * z)];
					else
						rightB = cyBlocks::m_regBlockTypes[(uint8_t) * (eastChunk.m_chunkdata + 4 + 32 * y + 32 * 32 * z)];

					if (x > 0)
						leftB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 3 + x + 32 * y + 32 * 32 * z)];
					else
						leftB = cyBlocks::m_regBlockTypes[(uint8_t) * (westChunk.m_chunkdata + 4 + 31 + 32 * y + 32 * 32 * z)];

					tmpface.x		 = x / 2.0f;
					tmpface.y		 = 16 - y / 2.0f;
					tmpface.z		 = z / 2.0f;
					uint8_t antitile = 0;
					if (blocktype < 2 || blocktype == 13 || blocktype == 25)
						antitile = 6;

					if (upperB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.face	 = 0 + antitile;
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][0];
						faces.emplace_back(tmpface);
					}
					if (blocktype == 1)
						antitile = 0;
					if (lowerB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][1];
						tmpface.face	 = 1 + antitile;
						faces.emplace_back(tmpface);
					}
					if (leftB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][2];
						tmpface.face	 = 2 + antitile;
						faces.emplace_back(tmpface);
					}
					if (rightB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][3];
						tmpface.face	 = 3 + antitile;
						faces.emplace_back(tmpface);
					}
					if (frontB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][4];
						tmpface.face	 = 5 + antitile;
						faces.emplace_back(tmpface);
					}
					if (backB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][5];
						tmpface.face	 = 4 + antitile;
						faces.emplace_back(tmpface);
					}
				} else if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_BILLBOARD) {
					tmpface.x		 = x / 2.0f;
					tmpface.y		 = 16 - y / 2.0f;
					tmpface.z		 = z / 2.0f;
					tmpface.material = cyBlocks::m_regBlockMats[blocktype][0];
					tmpface.face	 = 12;
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
		string meshname		  = to_string(relX) + to_string(relY);
		mesh				  = meshGen::AddMesh(tmpScene, faces[0].material, &entity);
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
				case 0:
					meshGen::AddFaceTop(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, false);
					break;
				case 1:
					meshGen::AddFaceBottom(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, false);
					break;
				case 2:
					meshGen::AddFaceLeft(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, false);
					break;
				case 3:
					meshGen::AddFaceRight(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, false);
					break;
				case 4:
					meshGen::AddFaceFront(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, false);
					break;
				case 5:
					meshGen::AddFaceBack(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, false);
					break;
				case 6:
					meshGen::AddFaceTop(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, true);
					break;
				case 7:
					meshGen::AddFaceBottom(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, true);
					break;
				case 8:
					meshGen::AddFaceLeft(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, true);
					break;
				case 9:
					meshGen::AddFaceRight(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, true);
					break;
				case 10:
					meshGen::AddFaceFront(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, true);
					break;
				case 11:
					meshGen::AddFaceBack(mesh, faces[i].x, faces[i].y, faces[i].z, relX, relY, true);
					break;
				case 12:
					meshGen::AddBillboard(mesh, faces[i].x, faces[i].y, faces[i].z);
					break;
			}
		}
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
		for (auto& pos : mesh->vertex_positions)
		{
			pos.x += relX;
			pos.z += relY;
		}

		mesh->SetDynamic(false);
		mesh->CreateRenderData();
		m.lock();
		wiScene::GetScene().Merge(tmpScene);
		m.unlock();
	}else
		wiBackLog::post("Chunk has no blocks");
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
