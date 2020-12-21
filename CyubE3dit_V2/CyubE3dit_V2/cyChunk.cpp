#include "stdafx.h"
#include "cyChunk.h"

using namespace std;

cyChunk::cyChunk(void) {
	m_chunkdata = (char*)malloc(32 * 32 * 800 + 4);
	memset(m_chunkdata, cyBlocks::m_voidID, 32 * 32 * 800 + 4);
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
			//std::string str(sqlite3_errmsg(db));
			//wiBackLog::post(str.c_str());
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
	if (chunksize) {
		char* compressedData = (char*)malloc(compressedSize);

		sqlite3_blob_read(pChunkBlob, compressedData, compressedSize, 0);
		if (fast) {	 //only decompress the block data, if fast is chosen
			m_chunkdata = (char*)realloc(m_chunkdata, 32 * 32 * 800 + 4);
			LZ4_decompress_fast(compressedData, m_chunkdata, 32 * 32 * 800 + 4);
		} else {
			m_chunkdata = (char*)realloc(m_chunkdata, chunksize);
			LZ4_decompress_fast(compressedData, m_chunkdata, chunksize);
			loadCustomBlocks();
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
		wiBackLog::post("Empty BLOB");
	}
	sqlite3_blob_close(pChunkBlob);
}
void cyChunk::airChunk(void) {
	m_chunkdata = (char*)realloc(m_chunkdata, 32 * 32 * 800 + 4);
	memset(m_chunkdata, cyBlocks::m_voidID, 32 * 32 * 800 + 4);
}
void cyChunk::solidChunk(void) {
	m_chunkdata = (char*)realloc(m_chunkdata, 32 * 32 * 800 + 4);
	memset(m_chunkdata, 0, 32 * 32 * 800 + 4);
}

void cyChunk::loadCustomBlocks(void) {
	if (_msize(m_chunkdata) > 0x190000) {
		uint32_t dSize	= 0;
		uint64_t offset = skipCdataArray(4 + 32 * 32 * 800 * 2, 5);	//Torch rotations ---> TBD
		offset			= skipCdataArray(offset, 5);
		memcpy(&m_lowestZ, m_chunkdata + offset, 2);
		offset += 2;
		memcpy(&m_highestZ, m_chunkdata + offset, 2);
		offset += 6;
		memcpy(&dSize, m_chunkdata + offset, 4);
		offset += 4;
		while (dSize) {
			offset += 4;	//skip TMap key
			offset = skipCdataArray(offset, 1);
			offset = skipCdataArray(offset, 4);
			dSize--;
		}
		offset += 8*4;	//4 bools to samity-check in the future (must be 0 or 1)
		offset += 5;	//unknown
		offset = skipCdataArray(offset, 8);
		offset += 8;
		offset = skipCdataArray(offset, 5);
		offset = skipCdataArray(offset, 1);	//length here must be 1024
		offset = skipCdataArray(offset, 4);
		loadCblockTmap(offset);

	}
	/*
	If(MemorySize(*destbuff) > $190000);Only look for custom blocks if the chunk was correctly loaded!
            NewMap customBlocks.xy(16)
            NewMap TorchRot.xy(16)
            varOffset = PeekL(*destbuff+4+32*32*800*2)
            For i=8+32*32*800*2 To 8+32*32*800*2+(varOffset*5)-1 Step 5
              TorchRot(Str(PeekA(*destbuff+ i))+","+Str(PeekA(*destbuff+ i+1))+","+Str(PeekW(*destbuff+ i+2)))\vis = PeekB(*destbuff+ i+4)
            Next
            varOffset = 8+32*32*800*2 + varOffset*5
            varOffset = varOffset + PeekL(*destbuff+ varoffset)*5+4
            LowestZ = PeekW(*destbuff+ varoffset)
            If(lowestZ > 0)
              lowestZ = LowestZ-1
            EndIf
            HighestZ = PeekW(*destbuff+ varoffset+2)
            varOffset = varoffset+8
            lightmapsize = PeekL(*destbuff+ varoffset)
            varoffset = varoffset + 4
            For i = 0 To lightmapsize-1
              varoffset = varoffset + 4
              varoffset = varoffset + PeekL(*destbuff+ varoffset)+4
              varoffset = varoffset + PeekL(*destbuff+ varoffset)*4+4
            Next
            For i=0 To 7
              If(PeekL(*destbuff+ varoffset) = 0 Or PeekL(*destbuff+ varoffset) = 1)
                varoffset = varoffset + 4
              Else
                varOffset = 0
                Break
              EndIf
            Next
            If(varOffset)
              varOffset = varOffset+5
              varOffset = varOffset + PeekL(*destbuff+varoffset)*8+4
              WorldFormat = PeekL(*destbuff+varoffset)
              If( WorldFormat < 51)
                varoffset = varoffset + 8
                varOffset = varOffset + PeekL(*destbuff+varoffset)*5+4
                If(PeekL(*destbuff+varoffset) = 1024)
                  varOffset = varOffset + PeekL(*destbuff+varoffset)*1+4
                  varOffset = varOffset + PeekL(*destbuff+varoffset)*4+4
                  CBlockLen = PeekL(*destbuff+varoffset)
                  varOffset = varOffset + 4
                  For i = varoffset To varoffset+(CBlockLen*8)-1 Step 8
                    customBlocks(Str(PeekA(*destbuff+ i))+","+Str(PeekA(*destbuff+ i+1))+","+Str(PeekW(*destbuff+ i+2)))\vis = PeekL(*destbuff+ i+4)
                  Next
                EndIf
              EndIf
            EndIf
          EndIf*/
}

uint64_t cyChunk::skipCdataArray(const uint64_t startpos, const uint8_t elementsize) {
	uint32_t size = 0;
	memcpy(&size, m_chunkdata + startpos, 4);
	return startpos + 4 + elementsize * size;
}

void cyChunk::loadCblockTmap(const uint64_t startpos) {
	uint32_t size = 0;
	uint32_t block = 0;
	blockpos_t pos;
	memcpy(&size, m_chunkdata + startpos, 4);

	for (uint32_t i = 0; i < size; i++) {
		memcpy(&(pos.x), m_chunkdata + startpos + 4 + i * 8,1);
		memcpy(&(pos.y), m_chunkdata + startpos + 5 + i * 8, 1);
		memcpy(&(pos.z), m_chunkdata + startpos + 6 + i * 8, 2);
		memcpy(&(block), m_chunkdata + startpos + 8 + i * 8, 4);
		m_cBlocks[pos] = block;
	}
}

cyChunk::~cyChunk() {
	free(m_chunkdata);
}
