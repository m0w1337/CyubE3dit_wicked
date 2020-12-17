#pragma once
#include "sqlite3.h"
#include "lz4.h"
#include "cyBlocks.h"

class cyChunk {
public:


	uint32_t m_id;
	char* m_chunkdata;
	explicit cyChunk(void);
	void loadChunk(sqlite3* db, uint32_t chunkID, bool fast = false);
	void airChunk(void);

	~cyChunk(void);
protected:
	sqlite3* m_db;
	

	void loadCustomBlocks(void);
	//static void loadChunk(sqlite3* db, uint32_t chunkID, uint32_t neightbourIDs[4]);
};
