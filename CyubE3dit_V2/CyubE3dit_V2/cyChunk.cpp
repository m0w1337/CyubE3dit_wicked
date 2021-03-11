#include "stdafx.h"
#include "cyChunk.h"

using namespace std;

cyChunk::cyChunk(void) {
	m_chunkdata = (char*)malloc(32 * 32 * 800 + 4);
}

void cyChunk::loadChunk(sqlite3* db, uint32_t chunkID, bool fast) {
	int rc;
	sqlite3_blob* pChunkBlob;
	m_id			= chunkID;
	m_isAirChunk	= true;
	m_surfaceheight = 799;
	rc				= sqlite3_blob_open(db, "main", "CHUNKDATA", "DATA", chunkID, 0, &pChunkBlob);
	if (rc) {
		Sleep(1);
		rc = sqlite3_blob_open(db, "main", "CHUNKDATA", "DATA", chunkID, 0, &pChunkBlob);
		if (rc) {
			std::string str(sqlite3_errmsg(db));
			wiBackLog::post(str.c_str());
			if (fast)
				solidChunk();
			else
				airChunk();
			return;
		}
	}

	int32_t chunksize	   = 0;
	int32_t compressedSize = sqlite3_blob_bytes(pChunkBlob) - 4;
	sqlite3_blob_read(pChunkBlob, &chunksize, 4, compressedSize);
	if (chunksize >= 32*32*800 + 4) {
		char* compressedData = (char*)malloc(compressedSize);
		sqlite3_blob_read(pChunkBlob, compressedData, compressedSize, 0);
		sqlite3_blob_close(pChunkBlob);
		memset(m_chunkdata, cyBlocks::m_voidID, 32 * 32 * 800 + 4);
		if (fast) {	 //only decompress the block data, if fast is chosen
			//m_chunkdata = (char*)realloc(m_chunkdata, 32 * 32 * 800 + 4);
			LZ4_decompress_fast(compressedData, m_chunkdata, 32 * 32 * 800 + 4);
		} else {
			m_chunkdata = (char*)realloc(m_chunkdata, chunksize);
			LZ4_decompress_fast(compressedData, m_chunkdata, chunksize);
			loadCustomBlocks();
			loadMeshes(db);
		}
		free(compressedData);
		m_isAirChunk = false;
		for (int_fast16_t z = 0; z < 800; z++) {
			uint16_t solidblocks = 0;
			for (uint_fast8_t x = 0; x < 32; x++) {
				for (uint_fast8_t y = 0; y < 32; y++) {
					if (cyBlocks::m_regBlockTypes[*(m_chunkdata + 4 + x + 32 * y + 32 * 32 * z)] < cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						solidblocks++;
					}
				}
			}
			if (solidblocks < 600) {
				m_surfaceheight = z;
				z				= 799;
			}
		}
	} else {
		sqlite3_blob_close(pChunkBlob);
		if (fast)
			solidChunk();
		else
			airChunk();
		wiBackLog::post("Empty BLOB");
	}
}

void cyChunk::replaceWithAir(uint8_t x, uint8_t y, uint16_t z) {
	m_chunkdata[4 + x + 32 * y + 32 * 32 * z] = cyBlocks::m_voidID;
}
void cyChunk::airChunk(void) {
	//m_chunkdata = (char*)realloc(m_chunkdata, 32 * 32 * 800 + 4);
	memset(m_chunkdata, cyBlocks::m_voidID, 32 * 32 * 800 + 4);
}
void cyChunk::solidChunk(void) {
	//m_chunkdata = (char*)realloc(m_chunkdata, 32 * 32 * 800 + 4);
	memset(m_chunkdata, 0, 32 * 32 * 800 + 4);
}

void cyChunk::addMesh(meshLoc mesh) {
	meshObjects.push_back(mesh);
}

void cyChunk::loadCustomBlocks(void) {
	if (_msize(m_chunkdata) > 0x190000) {
		uint32_t dSize	= 0;
		size_t offset = 4 + 32 * 32 * 800 * 2;	 //Torch rotations ---> TBD
		memcpy(&dSize, m_chunkdata + offset, 4);
		offset += 4;
		blockpos_t pos;
		uint8_t rot = 0;
		while (dSize) {
			memcpy(&(pos.x), m_chunkdata + offset, 1);
			memcpy(&(pos.y), m_chunkdata + offset + 1, 1);
			memcpy(&(pos.z), m_chunkdata + offset + 2, 2);
			memcpy(&(rot), m_chunkdata + offset + 4, 1);
			offset += 5;
			m_Torches[pos] = rot;
			dSize--;
		}
		offset = skipCdataArray(m_chunkdata, offset, 5);
		memcpy(&m_lowestZ, m_chunkdata + offset, 2);
		offset += 2;
		memcpy(&m_highestZ, m_chunkdata + offset, 2);
		offset += 6;
		memcpy(&dSize, m_chunkdata + offset, 4);
		offset += 4;
		while (dSize) {
			offset += 4;  //skip TMap key
			offset = skipCdataArray(m_chunkdata, offset, 1);
			offset = skipCdataArray(m_chunkdata, offset, 4);
			dSize--;
		}
		offset += 8 * 4;  //4 bools to samity-check in the future (must be 0 or 1)
		offset += 5;	  //unknown
		offset = skipCdataArray(m_chunkdata, offset, 8);
		offset += 8;
		offset = skipCdataArray(m_chunkdata, offset, 5);
		offset = skipCdataArray(m_chunkdata, offset, 1);	//length here must be 1024
		offset = skipCdataArray(m_chunkdata, offset, 4);
		loadCblockTmap(offset);
	}
}

void cyChunk::loadMeshes(sqlite3* db) {
	int rc;
	sqlite3_blob* pChunkBlob;
	meshObjects.clear();
	rc = sqlite3_blob_open(db, "main", "MESHOBJECTS", "DATA", m_id, 0, &pChunkBlob);
	if (rc) {
		Sleep(1);
		rc = sqlite3_blob_open(db, "main", "MESHOBJECTS", "DATA", m_id, 0, &pChunkBlob);
		if (rc) {
			return;
		}
	}

	int32_t chunksize = 0;
	int32_t size	  = sqlite3_blob_bytes(pChunkBlob);
	char* meshData	  = (char*)malloc(size);
	sqlite3_blob_read(pChunkBlob, meshData, size, 0);
	sqlite3_blob_close(pChunkBlob);
	uint32_t dSize = 0, treetypes = 0, offset = 4;
	if (size > 32) {
		meshLoc mesh;
		memcpy(&dSize, meshData + offset, 4);
		offset += 4;
		while (dSize) {
			memcpy(&(mesh.type), meshData + offset, 1);
			memcpy(&(mesh.qRot.x), meshData + 1 + offset, 4);
			memcpy(&(mesh.qRot.y), meshData + 5 + offset, 4);
			memcpy(&(mesh.qRot.z), meshData + 9 + offset, 4);
			memcpy(&(mesh.qRot.w), meshData + 13 + offset, 4);
			memcpy(&(mesh.pos.x), meshData + 17 + offset, 4);
			memcpy(&(mesh.pos.y), meshData + 21 + offset, 4);
			memcpy(&(mesh.pos.z), meshData + 25 + offset, 4);
			memcpy(&(mesh.scale.x), meshData + 29 + offset, 4);
			memcpy(&(mesh.scale.y), meshData + 33 + offset, 4);
			memcpy(&(mesh.scale.z), meshData + 37 + offset, 4);
			mesh.pos.x /= 100.0f;
			mesh.pos.y /= 100.0f;
			mesh.pos.z /= 100.0f;
			meshObjects.push_back(mesh);
			offset += 41;
			offset = skipCdataArray(meshData, offset, 1);
			dSize--;
		}
		memcpy(&treetypes, meshData + offset, 4);
		offset += 4;
		treeLoc tree;
			for (uint32_t ttype = 0; ttype < treetypes; ttype++) {
				memcpy(&dSize, meshData + offset, 4);
				offset += 4;
				tree.type = ttype;
				for (uint32_t i = 0; i < dSize && (offset + 20) < size; i++) {
					memcpy(&(tree.pos.x), meshData + offset, 1);
					memcpy(&(tree.pos.y), meshData + 1 + offset, 1);
					memcpy(&(tree.pos.z), meshData + 2 + offset, 2);
					memcpy(&(tree.yaw), meshData + 4 + offset, 4);
					memcpy(&(tree.scale.x), meshData + 8 + offset, 4);
					memcpy(&(tree.scale.y), meshData + 12 + offset, 4);
					memcpy(&(tree.scale.z), meshData + 16 + offset, 4);
					trees.push_back(tree);
					offset += 20;
				}
			}
	}
	free(meshData);
}

uint64_t cyChunk::skipCdataArray(char* memory, const uint64_t startpos, const uint8_t elementsize) {
	uint32_t size = 0;
	memcpy(&size, memory + startpos, 4);
	return startpos + 4 + elementsize * size;
}

void cyChunk::loadCblockTmap(const uint64_t startpos) {
	uint32_t size  = 0;
	uint32_t block = 0;
	blockpos_t pos;
	memcpy(&size, m_chunkdata + startpos, 4);

	for (uint32_t i = 0; i < size; i++) {
		memcpy(&(pos.x), m_chunkdata + startpos + 4 + i * 8, 1);
		memcpy(&(pos.y), m_chunkdata + startpos + 5 + i * 8, 1);
		memcpy(&(pos.z), m_chunkdata + startpos + 6 + i * 8, 2);
		memcpy(&(block), m_chunkdata + startpos + 8 + i * 8, 4);
		m_cBlocks[pos] = block;
	}
}

cyChunk::~cyChunk() {
	free(m_chunkdata);
}
