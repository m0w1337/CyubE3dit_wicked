#pragma once
#include "cyChunk.h"
 #include "cyImportant.h"
#include "meshGen.h"
#include "wiECS.h"
#include "settings.h"


class chunkLoader {
public:
	static constexpr uint8_t MAX_THREADS = 16;
	struct face_t {
		wiECS::Entity material;
		uint8_t face;
		bool antitile;
		float x;
		float y;
		float z;
		uint8_t size;
		bool operator<(const face_t& a) const {	 //Make it sortable by material
			return material < a.material;
		}
	};
	struct chunkobjects_t {
		wiECS::Entity chunkObj;
		std::vector<wiECS::Entity> trees;
	};
	struct point2d_t {
		int32_t x;
		int32_t y;
	};
	static void loadWorld(void);
	~chunkLoader(void);
	void shutdown(void);
	void spawnThreads(uint8_t numthreads);
	void checkChunks(void);
	void addChunks(uint8_t threadNum); 
	chunkobjects_t RenderChunk(const cyChunk& chunk, const cyChunk& northChunk, const cyChunk& eastChunk, const cyChunk& southChunk, const cyChunk& westChunk, const int32_t relX, const int32_t relY);
	void updateDisplayedChunks(void);
	cyImportant::chunkpos_t spiral(const int32_t iteration);	//Legacy
	unordered_map<cyImportant::chunkpos_t, chunkobjects_t, cyImportant::chunkpos_t> m_visibleChunks;
	atomic<uint8_t> m_threadstate[MAX_THREADS];
	atomic<int32_t> m_threadChunkX[MAX_THREADS];
	atomic<int32_t> m_threadChunkY[MAX_THREADS];
	atomic<uint8_t> m_shutdown;
	thread m_checkThread;
	thread m_thread[MAX_THREADS];
	uint8_t m_numthreads;

private:
	static const uint_fast16_t VIEWDISTANCE = 1024;

};
