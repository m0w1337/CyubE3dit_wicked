#pragma once
#include "cyChunk.h"
#include "cyImportant.h"
#include "meshGen.h"
#include "wiECS.h"
#include "settings.h"

class chunkLoader {
public:
	static constexpr uint8_t MAX_THREADS = 32;

	static constexpr uint32_t THREAD_IDLE = 1;
	static constexpr uint32_t THREAD_BUSY = 2;
	//static constexpr uint32_t THREAD_SHUTDOWN = 3;

	static constexpr float PI = 3.141592653589793238462643383279502884f;

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
	struct torch_t {
		torch_t() :
		  ID(0),
		  x(0),
		  y(0),
		  z(0),
		  rotation(0){};
		torch_t(const uint8_t& _ID, const uint8_t& _x, const uint8_t& _y, const uint16_t& _z, uint8_t _rotation) :
		  ID(_ID),
		  x(_x),
		  y(_y),
		  z(_z),
		  rotation(_rotation){};
		uint8_t ID;
		uint8_t x;
		uint8_t y;
		uint16_t z;
		uint8_t rotation;
	};
	struct chunkobjects_t {
		wiECS::Entity chunkObj;
		std::vector<wiECS::Entity> meshes;
	};
	struct point2d_t {
		int32_t x;
		int32_t y;
	};
	//static void loadWorld(void);
	~chunkLoader(void);
	void shutdown(void);
	void spawnThreads(uint8_t numthreads);
	void checkChunks(void);
	
	wiECS::Entity RenderChunk(const cyChunk& chunk, const cyChunk& northChunk, const cyChunk& eastChunk, const cyChunk& southChunk, const cyChunk& westChunk, const int32_t relX, const int32_t relY);
	void updateDisplayedChunks(void);
	//cyImportant::chunkpos_t spiral(const int32_t iteration);  //Legacy
	unordered_map<cyImportant::chunkpos_t, chunkobjects_t, cyImportant::chunkpos_t> m_visibleChunks;
	atomic<uint32_t> m_threadstate[MAX_THREADS];
	cyImportant::chunkpos_t m_threadChunkPos[MAX_THREADS];
	atomic<uint8_t> m_shutdown;
	thread m_checkThread;
	thread m_thread[MAX_THREADS];
	uint8_t m_numthreads;

private:
	static const uint_fast16_t VIEWDISTANCE = 1024;
	inline void removeFarChunks(cyImportant::chunkpos_t ghostpos, bool cleanAll=false);
	void addChunks(uint8_t threadNum);
	inline void employThread(cyImportant::chunkpos_t coords);
	inline void placeMeshes(const cyChunk& chunk, const int32_t relX, const int32_t relY, wiScene::Scene& tmpScene, const wiECS::Entity parent);
	inline void placeTorches(const std::vector<torch_t>& torches, const int32_t relX, const int32_t relY, wiScene::Scene& tmpScene, const wiECS::Entity parent);
};
