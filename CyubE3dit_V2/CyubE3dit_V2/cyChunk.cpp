#include "stdafx.h"
#include "cyChunk.h"
#include "settings.h"


cyChunk::cyChunk(void) {
	m_chunkdata = (char*)malloc(32 * 32 * 800 + 4);
	m_version	= 0;
	m_saveable	= false;
}

void cyChunk::loadChunk(sqlite3* db, uint32_t chunkID, bool fast) {
	int rc;
	sqlite3_blob* pChunkBlob;
	m_id			 = chunkID;
	m_treetypeOffset = 0;
	m_cBlocksTmapPos = 0;
	m_torchTmapPos	 = 0;
	m_saveable		 = false;  //set it to true to mark not chunk as completely loaded until all data is found successfully, then set to correct state
	m_db			 = db;
	m_isAirChunk	 = true;
	m_surfaceheight	 = 799;
	rc				 = sqlite3_blob_open(db, "main", "CHUNKDATA", "DATA", chunkID, 0, &pChunkBlob);
	while (rc) {
		if (sqlite3_errmsg(db) == std::string("database is locked")) {
			Sleep(2);
			std::string str(sqlite3_errmsg(db));
			wiBackLog::post("retry...");
			wiBackLog::post(str.c_str());
			rc = sqlite3_blob_open(db, "main", "CHUNKDATA", "DATA", chunkID, 0, &pChunkBlob);
		} else {
			break;
		}
	}
	int32_t chunksize	   = 0;
	int32_t compressedSize = 0;
	char* compressedData;

	if (rc) {
		ifstream file;
		file.open(settings::getWorld()->m_worldFolder.c_str() + to_wstring(chunkID) + L".chunks", fstream::in | ios::binary | std::ios::ate);
		if (file.is_open()) {
			std::streamsize size = file.tellg();
			compressedSize		 = (uint32_t)size - 4;
			file.seekg(0, std::ios::beg);
			compressedData = (char*)malloc(compressedSize);
			if (!file.read(compressedData, compressedSize))
			{
				free(compressedData);
				file.close();
				if (fast)
					solidChunk();
				else
					airChunk();
				return;
			}
			file.read((char*)&chunksize, 4);
			file.close();

		} else {
			if (fast)
				solidChunk();
			else
				airChunk();
			return;
		}
	} else {
		compressedSize = sqlite3_blob_bytes(pChunkBlob) - 4;
		compressedData = (char*)malloc(compressedSize);
		sqlite3_blob_read(pChunkBlob, &chunksize, 4, compressedSize);
		sqlite3_blob_read(pChunkBlob, compressedData, compressedSize, 0);
		sqlite3_blob_close(pChunkBlob);
	}

	if (chunksize >= 32 * 32 * 800 + 4) {
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
		if (fast)
			solidChunk();
		else
			airChunk();
	}
	if (m_cBlocksTmapPos && m_torchTmapPos && !fast)
		m_saveable = true;
}

void cyChunk::replaceWithAir(const uint8_t x, const uint8_t y, const uint16_t z) {
	m_chunkdata[4 + x + 32 * y + 32 * 32 * z] = cyBlocks::m_voidID;
}

void cyChunk::replaceBlock(const uint8_t x, const uint8_t y, const uint16_t z, const uint8_t _blockID, const uint32_t _blockData) {
	size_t mempos = 4 + x + 32 * y + 32 * 32 * z;
	blockpos_t bpos(x, y, z);
	if (cyBlocks::m_regBlockTypes[_blockID] != cyBlocks::BLOCKTYPE_MOD) {
		if (cyBlocks::m_regBlockTypes[m_chunkdata[mempos]] == cyBlocks::BLOCKTYPE_MOD) {  //remove mod block entry, if current block is a mod block and new one isn't
			m_cBlocks.erase(bpos);
		}
	} else {
		m_cBlocks[bpos] = _blockData;  //add the new cblock entry here
	}
	if (cyBlocks::m_regBlockTypes[_blockID] != cyBlocks::BLOCKTYPE_TORCH) {
		if (cyBlocks::m_regBlockTypes[m_chunkdata[mempos]] == cyBlocks::BLOCKTYPE_TORCH) {	//remove torch rotation entry, if current block is a torch block and new one isn't
			m_Torches.erase(bpos);
		}
	} else {
		m_Torches[bpos] = _blockData;  //add the new cblock entry here
	}
	m_chunkdata[mempos] = _blockID;
}

void cyChunk::getBlock(const uint8_t x, const uint8_t y, const uint16_t z, uint8_t* _blockID, uint32_t* _blockData) {
	size_t mempos = 4 + x + 32 * y + 32 * 32 * z;
	blockpos_t bpos(x, y, z);
	*_blockID = m_chunkdata[mempos];
	if (cyBlocks::m_regBlockTypes[*_blockID] == cyBlocks::BLOCKTYPE_MOD) {
		auto it = m_cBlocks.find(bpos);
		if (it != m_cBlocks.end()) {
			*_blockData = it->second;
		}
	} else if (cyBlocks::m_regBlockTypes[*_blockID] == cyBlocks::BLOCKTYPE_TORCH) {
		auto it = m_Torches.find(bpos);
		if (it != m_Torches.end()) {
			*_blockData = it->second;
		}
	}
}

void cyChunk::replaceCubeWithAir(const uint8_t x1, const uint8_t y1, const uint16_t z1, const uint8_t x2, const uint8_t y2, const uint16_t z2) {
	for (uint8_t x = x1; x < x2; x++) {
		for (uint8_t y = y1; y < y2; y++) {
			for (uint16_t z = z1; z < z2; z++) {
				m_chunkdata[4 + x + 32 * y + 32 * 32 * z] = cyBlocks::m_voidID;
			}
		}
	}
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

void cyChunk::saveZlimits(const uint16_t _lowestZ, const uint16_t _highestZ) {
	size_t offset = 4 + 32 * 32 * 800 * 2;	//Torch rotations
	offset		  = skipCdataArray(m_chunkdata, offset, 5);
	offset		  = skipCdataArray(m_chunkdata, offset, 5);
	memcpy(m_chunkdata + offset, &_lowestZ, 2);
	offset += 2;
	memcpy(m_chunkdata + offset, &_highestZ, 2);
	m_highestZ = _highestZ;
	m_lowestZ  = _lowestZ;
}

void cyChunk::loadCustomBlocks(void) {
	uint16_t check16;
	uint32_t check32;
	if (_msize(m_chunkdata) > 0x190000) {
		uint32_t dSize = 0;
		size_t offset  = 4 + 32 * 32 * 800 * 2;	 //Torch rotations
		m_torchTmapPos = offset;
		memcpy(&dSize, m_chunkdata + offset, 4);
		offset += 4;
		blockpos_t pos;
		uint8_t rot = 0;
		while (dSize && offset < _msize(m_chunkdata) - 50) {
			memcpy(&(pos.x), m_chunkdata + offset, 1);
			memcpy(&(pos.y), m_chunkdata + offset + 1, 1);
			memcpy(&(pos.z), m_chunkdata + offset + 2, 2);
			memcpy(&(rot), m_chunkdata + offset + 4, 1);
			offset += 5;
			m_Torches[pos] = rot;
			dSize--;
		}
		if (offset >= _msize(m_chunkdata) - 50) {
			m_torchTmapPos = 0;
			return;
		}

		offset = skipCdataArray(m_chunkdata, offset, 5);
		memcpy(&m_lowestZ, m_chunkdata + offset, 2);
		offset += 2;
		memcpy(&m_highestZ, m_chunkdata + offset, 2);
		if (m_highestZ <= m_lowestZ + 2)
			m_highestZ = 750;
		offset += 2;
		memcpy(&check16, m_chunkdata + offset, 2);
		if (check16 > 800)
			return;
		offset += 2;
		memcpy(&check16, m_chunkdata + offset, 2);
		if (check16 > 800)
			return;
		offset += 2;
		memcpy(&dSize, m_chunkdata + offset, 4);
		offset += 4;
		while (dSize) {
			offset += 4;  //skip TMap key
			offset = skipCdataArray(m_chunkdata, offset, 1);
			if (offset == 0)
				return;
			offset = skipCdataArray(m_chunkdata, offset, 4);
			if (offset == 0)
				return;
			dSize--;
		}
		dSize = 8;
		while (dSize) {	 //8 bools to sanity-check (must be 0 or 1)
			memcpy(&check32, m_chunkdata + offset, 4);
			if (check32 > 1)
				return;
			offset += 4;
			dSize--;
		}
		offset += 5;  //unknown
		offset = skipCdataArray(m_chunkdata, offset, 8);
		if (offset == 0)
			return;
		memcpy(&m_version, m_chunkdata + offset, 4);
		offset += 8;
		offset = skipCdataArray(m_chunkdata, offset, 5);
		offset = skipCdataArray(m_chunkdata, offset, 1);  //length here must be 1024
		if (offset == 0)
			return;
		offset = skipCdataArray(m_chunkdata, offset, 4);
		if (offset == 0)
			return;
		m_cBlocksTmapPos = offset;
		loadCblockTmap(offset);
	}
}

void cyChunk::loadMeshes(sqlite3* db) {
	int rc;
	sqlite3_blob* pChunkBlob;
	meshObjects.clear();
	rc = sqlite3_blob_open(db, "main", "MESHOBJECTS", "DATA", m_id, 0, &pChunkBlob);
	while (rc) {
		if (sqlite3_errmsg(db) == std::string("database is locked")) {
			Sleep(2);
			std::string str(sqlite3_errmsg(db));
			wiBackLog::post("retry...");
			wiBackLog::post(str.c_str());
			rc = sqlite3_blob_open(db, "main", "MESHOBJECTS", "DATA", m_id, 0, &pChunkBlob);
		} else {
			break;
		}
	}
	int32_t chunksize = 0;
	int32_t size	  = 0;
	char* meshData;

	if (rc) {
		ifstream file;
		file.open(settings::getWorld()->m_worldFolder.c_str() + to_wstring(m_id) + L".chunkmon", fstream::in | ios::binary | std::ios::ate);
		if (file.is_open()) {
			std::streamsize fsize = file.tellg();
			size				  = (uint32_t)fsize;
			file.seekg(0, std::ios::beg);
			meshData = (char*)malloc(size);
			if (!file.read(meshData, size))
			{
				free(meshData);
				file.close();
				return;
			}
			file.close();

		} else {
			return;
		}
	} else {
		size	 = sqlite3_blob_bytes(pChunkBlob);
		meshData = (char*)malloc(size);
		sqlite3_blob_read(pChunkBlob, meshData, size, 0);
		sqlite3_blob_close(pChunkBlob);
	}

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
		m_treetypeOffset = offset;
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
	uint64_t ret;
	memcpy(&size, memory + startpos, 4);
	ret = startpos + 4 + elementsize * size;
	if (ret > _msize(m_chunkdata) - 4)
		return 0;
	return ret;
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

bool cyChunk::saveChunk(void) {
	int rc;
	bool ret = false;
	sqlite3_blob* pChunkBlob;
	uint32_t i		   = 0;
	uint32_t torchlen  = 0;
	uint32_t cBlocklen = 0;
	int64_t sizeDelta  = 0;
	size_t offsetNew   = 0;
	size_t offsetOld   = 0;
	if (m_isAirChunk || !m_saveable) {	//only fully loaded chunks can be saved
		wiHelper::messageBox("Only fully loaded chunks can be saved, please do a screenshot of this message and report the steps that lead to this to the developer.");
		return ret;
	}
	if (m_cBlocks.size() > 0 && m_cBlocksTmapPos == 0) {  //old chunks not having a custom blocks map will also not be saveable when there were custom blocks inserted (just to ensure not breaking anything)
		wiHelper::messageBox("It seems you want to insert a custom block into a chunk of old data format, please manipulate the chunk in game to update it to the newest data structure and try again.");
		return ret;
	}
	memcpy(&torchlen, m_chunkdata + m_torchTmapPos, 4);
	memcpy(&cBlocklen, m_chunkdata + m_cBlocksTmapPos, 4);
	if (torchlen > 800 * 32 * 32 || cBlocklen > 800 * 32 * 32) {
		wiHelper::messageBox("too many torches or custom blocks found, chunk data corrupt !!");
		return ret;
	}
	sizeDelta = m_cBlocks.size() * 8 + m_Torches.size() * 5;
	sizeDelta -= (torchlen * 5 + cBlocklen * 8);
	uint8_t* newChunk = reinterpret_cast<uint8_t*>(malloc(_msize(m_chunkdata) + sizeDelta));
	memcpy(newChunk, m_chunkdata, m_torchTmapPos);
	offsetOld = m_torchTmapPos + 4 + torchlen * 5;
	offsetNew = m_torchTmapPos + 4 + m_Torches.size() * 5;
	torchlen  = m_Torches.size();
	memcpy(newChunk + m_torchTmapPos, &torchlen, 4);
	i = 0;
	for (auto& it : m_Torches) {
		blockpos_t tpos = it.first;
		uint8_t trot	= it.second;
		memcpy(newChunk + m_torchTmapPos + 4 + i * 5, &(tpos.x), 1);
		memcpy(newChunk + m_torchTmapPos + 5 + i * 5, &(tpos.y), 1);
		memcpy(newChunk + m_torchTmapPos + 6 + i * 5, &(tpos.z), 2);
		memcpy(newChunk + m_torchTmapPos + 8 + i * 5, &(trot), 1);
		i++;
	}

	memcpy(newChunk + offsetNew, m_chunkdata + offsetOld, m_cBlocksTmapPos - offsetOld);
	offsetNew += m_cBlocksTmapPos - offsetOld;
	offsetOld = m_cBlocksTmapPos + 4 + cBlocklen * 8;

	cBlocklen = m_cBlocks.size();
	memcpy(newChunk + offsetNew, &cBlocklen, 4);
	i = 0;
	for (auto& it : m_cBlocks) {
		blockpos_t tpos = it.first;
		uint32_t cID	= it.second;
		memcpy(newChunk + offsetNew + 4 + i * 8, &(tpos.x), 1);
		memcpy(newChunk + offsetNew + 5 + i * 8, &(tpos.y), 1);
		memcpy(newChunk + offsetNew + 6 + i * 8, &(tpos.z), 2);
		memcpy(newChunk + offsetNew + 8 + i * 8, &(cID), 4);
		i++;
	}
	offsetNew += 4 + i * 8;
	memcpy(newChunk + offsetNew, m_chunkdata + offsetOld, _msize(m_chunkdata) - offsetOld);
	offsetNew += _msize(m_chunkdata) - offsetOld;
	if (offsetNew != _msize(newChunk)) {
		wiHelper::messageBox("Something went wrong, manipulation of chunkdata failed-> invalid resulting memory length!");
		free(newChunk);
		return ret;
	}
	//at this point newChunk should contain the newly manipulated valid chunk data offsetNew holds the new uncompressed data size
	sqlite3_stmt* stmt = NULL;
	rc				   = sqlite3_prepare_v2(m_db,
							string("DELETE FROM CHUNKDATA WHERE CHUNKID = ?").c_str(),
							-1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		wiHelper::messageBox("SQLITE deletion failed!");

	} else {
		rc = sqlite3_bind_int(stmt, 1, m_id);
		if (rc != SQLITE_OK) {
			cerr << "INT bind failed: " << sqlite3_errmsg(m_db) << endl;
		} else {
			rc = sqlite3_step(stmt);
			while (rc == SQLITE_BUSY) {
				rc = sqlite3_step(stmt);
			}
			if (rc != SQLITE_DONE)
				cerr << "deletion failed: " << sqlite3_errmsg(m_db) << endl;
		}
	}
	cerr << "deletion done: " << sqlite3_errmsg(m_db) << endl;
	sqlite3_finalize(stmt);

	rc = sqlite3_prepare_v2(m_db,
							string("INSERT INTO CHUNKDATA(CHUNKID, DATA) VALUES(?, ?)").c_str(),
							-1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		wiHelper::messageBox("SQLITE insertion failed!");

	} else {
		char* compressed	 = reinterpret_cast<char*>(malloc(offsetNew));
		size_t compressedLen = LZ4_compress_default(reinterpret_cast<char*>(newChunk), compressed, offsetNew, offsetNew - 4);
		memcpy(compressed + compressedLen, &offsetNew, 4);
		compressedLen += 4;
		// SQLITE_STATIC because the statement is finalized
		// before the buffer is freed:
		rc = sqlite3_bind_blob(stmt, 2, compressed, compressedLen, SQLITE_STATIC);
		if (rc != SQLITE_OK) {
			cerr << "bind failed: " << sqlite3_errmsg(m_db) << endl;
		} else {
			rc = sqlite3_bind_int(stmt, 1, m_id);
			if (rc != SQLITE_OK) {
				cerr << "INT bind failed: " << sqlite3_errmsg(m_db) << endl;
			} else {
				rc = sqlite3_step(stmt);
				while (rc == SQLITE_BUSY) {
					rc = sqlite3_step(stmt);
				}
				if (rc != SQLITE_DONE)
					cerr << "execution failed: " << sqlite3_errmsg(m_db) << endl;
				else
					ret = true;
			}
		}
		free(compressed);
	}
	sqlite3_finalize(stmt);
	cerr << "execution done: " << sqlite3_errmsg(m_db) << endl;
	free(newChunk);
	deleteInstaLoad();
	return ret;
}

void cyChunk::deleteTree(blockpos_t position) {
	int rc;
	sqlite3_blob* pChunkBlob;
	meshObjects.clear();
	rc = sqlite3_blob_open(m_db, "main", "MESHOBJECTS", "DATA", m_id, 0, &pChunkBlob);
	while (rc) {
		if (sqlite3_errmsg(m_db) == std::string("database (mesh) is locked")) {
			Sleep(2);
			std::string str(sqlite3_errmsg(m_db));
			wiBackLog::post("retry...");
			wiBackLog::post(str.c_str());
			rc = sqlite3_blob_open(m_db, "main", "MESHOBJECTS", "DATA", m_id, 0, &pChunkBlob);
		} else {
			break;
		}
	}
	int32_t chunksize = 0;
	int32_t size	  = 0;
	char* meshData = nullptr;
	meshData = (char*)malloc(100);
	if (rc) {
		ifstream file;
		file.open(settings::getWorld()->m_worldFolder.c_str() + to_wstring(m_id) + L".chunkmon", fstream::in | ios::binary | std::ios::ate);
		if (file.is_open()) {
			std::streamsize fsize = file.tellg();
			size				  = (uint32_t)fsize;
			file.seekg(0, std::ios::beg);
			meshData = (char*)realloc(meshData, size);
			if (!file.read(meshData, size))
			{
				free(meshData);
				file.close();
				return;
			}
			file.close();

		} else {
			free(meshData);
			return;
		}
	} else {
		size	 = sqlite3_blob_bytes(pChunkBlob);
		meshData = (char*)realloc(meshData, size);
		sqlite3_blob_read(pChunkBlob, meshData, size, 0);
		sqlite3_blob_close(pChunkBlob);
	}

	uint32_t dSize = 0, treetypes = 0, offset = 4;
	if (size > 32) {
		meshLoc mesh;
		memcpy(&dSize, meshData + offset, 4);
		offset += 4;
		while (dSize) {
			offset += 41;
			offset = skipCdataArray(meshData, offset, 1);
			dSize--;
		}
		m_treetypeOffset = offset;
		memcpy(&treetypes, meshData + offset, 4);
		offset += 4;
		treeLoc tree;
		if (treetypes < 50) {

			for (uint32_t ttype = 0; ttype < treetypes; ttype++) {
				memcpy(&dSize, meshData + offset, 4);
				size_t sizeLoc = offset;
				offset += 4;
				tree.type = ttype;
				for (uint32_t i = 0; i < dSize && (offset + 20) < size; i++) {
					memcpy(&(tree.pos.x), meshData + offset, 1);
					memcpy(&(tree.pos.y), meshData + 1 + offset, 1);
					memcpy(&(tree.pos.z), meshData + 2 + offset, 2);

					if (tree.pos == position) {
						//stringstream ss("");
						--dSize;
						memcpy(meshData + sizeLoc, &dSize, 4);
						memmove(meshData + offset, meshData + offset + 20, size - (offset + 20));
						sqlite3_stmt* stmt = NULL;
						rc				   = sqlite3_prepare_v2(m_db,
												string("DELETE FROM MESHOBJECTS WHERE CHUNKID = ?;").c_str(),
												-1, &stmt, NULL);
						if (rc != SQLITE_OK) {
							wiHelper::messageBox("SQLITE deletion failed!");

						} else {
							rc = sqlite3_bind_int(stmt, 1, m_id);
							if (rc != SQLITE_OK) {
								//ss << "INT bind failed: " << sqlite3_errmsg(m_db) << endl;
							} else {
								rc = sqlite3_step(stmt);
								while (rc == SQLITE_BUSY) {
									rc = sqlite3_step(stmt);
								}
								//if (rc != SQLITE_DONE)
								//	ss << "deletion failed: " << sqlite3_errmsg(m_db) << endl;
							}
						}
						//ss << "deletion done: " << sqlite3_errmsg(m_db) << endl;
						sqlite3_finalize(stmt);

						rc = sqlite3_prepare_v2(m_db,
												string("INSERT INTO MESHOBJECTS(CHUNKID, DATA) VALUES(?, ?)").c_str(),
												-1, &stmt, NULL);
						if (rc != SQLITE_OK) {
							wiHelper::messageBox("SQLITE insertion failed!");

						} else {

							// SQLITE_STATIC because the statement is finalized
							// before the buffer is freed:
							rc = sqlite3_bind_blob(stmt, 2, meshData, size - 20, SQLITE_STATIC);
							if (rc != SQLITE_OK) {
								//	ss << "bind failed: " << sqlite3_errmsg(m_db) << endl;
							} else {
								rc = sqlite3_bind_int(stmt, 1, m_id);
								if (rc != SQLITE_OK) {
									//		ss << "INT bind failed: " << sqlite3_errmsg(m_db) << endl;
								} else {
									rc = sqlite3_step(stmt);
									while (rc == SQLITE_BUSY || rc == SQLITE_ROW) {
										rc = sqlite3_step(stmt);
									}
									//	if (rc != SQLITE_DONE)
									//	ss << "execution failed: " << sqlite3_errmsg(m_db) << endl;
								}
							}
						}
						sqlite3_finalize(stmt);
						deleteInstaLoad();
						//ss << "execution done: " << sqlite3_errmsg(m_db) << endl;
						//wiBackLog::post(ss.str().c_str());
						i = dSize - 1;
						break;
					}
					offset += 20;
				}
			}
		}
	}
	free(meshData);
}

void cyChunk::deleteInstaLoad(void) {
	sqlite3* instaDB;
	stringstream ss("");
	if (sqlite3_open_v2(settings::getWorld()->m_instaLoadDB.c_str(), &(instaDB), SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READWRITE, NULL)) {
		if (sqlite3_open_v2(settings::getWorld()->m_instaLoadDB.c_str(), &(instaDB), SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READWRITE, NULL)) {
			instaDB = nullptr;
			ss << "failed to open instaload DB";
			ss << settings::getWorld()->m_instaLoadDB.c_str();
			if (instaDB != nullptr) {
				sqlite3_close(instaDB);
				instaDB = nullptr;
			}
		}
	}
	if (instaDB != nullptr) {
		sqlite3_stmt* stmt = NULL;
		
		int rc;
		rc = sqlite3_prepare_v2(instaDB,
								  string("DELETE FROM CHUNKDATA WHERE CHUNKID = ?;").c_str(),
								  -1, &stmt, NULL);
		if (rc != SQLITE_OK) {
			wiHelper::messageBox("SQLITE instaload deletion failed!");

		} else {
			rc = sqlite3_bind_int(stmt, 1, m_id);
			if (rc != SQLITE_OK) {
				ss << "instaload INT bind failed: " << sqlite3_errmsg(instaDB) << endl;
			} else {
				rc = sqlite3_step(stmt);
				while (rc == SQLITE_BUSY || rc == SQLITE_ROW) {
					rc = sqlite3_step(stmt);
				}
				if (rc != SQLITE_DONE)
					ss << "instaload deletion failed: " << sqlite3_errmsg(instaDB) << endl;
			}
		}
		ss << "instaload deletion done: " << sqlite3_errmsg(instaDB) << endl;
		sqlite3_finalize(stmt);
		
		sqlite3_close(instaDB);

	} else {
		ss << "InstaLoad DB connection failed!!";
	}
	wiBackLog::post(ss.str().c_str());
}

cyChunk::~cyChunk() {
	free(m_chunkdata);
}
