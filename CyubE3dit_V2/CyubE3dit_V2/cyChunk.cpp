#include "stdafx.h"
#include "cyChunk.h"


using namespace std;

cyChunk::cyChunk(void) {
	m_chunkdata = (char*)malloc(32 * 32 * 800);
	memset(m_chunkdata, cyBlocks::m_voidID, 32 * 32 * 800);
}

void cyChunk::loadChunk(sqlite3* db, uint32_t chunkID, bool fast){
	int rc;
	sqlite3_blob* pChunkBlob;
	rc = sqlite3_blob_open(db, "main", "CHUNKDATA", "DATA", chunkID, 0, &pChunkBlob);
	if (rc) {
		return;
	} else {
		int32_t chunksize	   = 0;
		int32_t compressedSize = sqlite3_blob_bytes(pChunkBlob) - 4;
		sqlite3_blob_read(pChunkBlob, &chunksize, 4, compressedSize);
		if (chunksize) {
			char* compressedData = (char*)malloc(compressedSize);

			sqlite3_blob_read(pChunkBlob, compressedData, compressedSize, 0);
			if (fast) {	 //only decompress the block data, if fast is chosen
				m_chunkdata = (char*)malloc(32 * 32 * 800 + 4);
				LZ4_decompress_fast(compressedData, m_chunkdata, 32 * 32 * 800 + 4);
			} else {
				m_chunkdata = (char*)malloc(chunksize);
				LZ4_decompress_fast(compressedData, m_chunkdata, chunksize);
				loadCustomBlocks();
			}
		}
	}
}
void cyChunk::airChunk(void) {
	m_chunkdata = (char*)malloc(32 * 32 * 800);
	memset(m_chunkdata, cyBlocks::m_voidID, 32 * 32 * 800);
}

void cyChunk::loadCustomBlocks(void) {

}
