#pragma once
#include "sqlite3.h"
#include "lz4.h"
#include "cyBlocks.h"

using namespace std;


class cyChunk {
public:
	struct blockpos_t {
		uint8_t x;
		uint8_t y;
		uint16_t z;
		blockpos_t() :
		  x(0),
		  y(0),
		  z(0){};
		blockpos_t(const uint8_t& _x, const uint8_t& _y, const uint16_t& _z) :
		  x(_x),
		  y(_y),
		  z(_z){};
		blockpos_t(const blockpos_t& other) {
			x = other.x;
			y = other.y;
			z = other.z;
		};

		blockpos_t& operator=(const blockpos_t& other) {
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		};

		bool operator==(const blockpos_t& other) const {
			if (x == other.x && y == other.y && z == other.z)
				return true;
			return false;
		};

		blockpos_t& operator+(const blockpos_t& other) {
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		};
		blockpos_t& operator-(const blockpos_t& other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		};

		bool operator<(const blockpos_t& other) {
			if (x < other.x)
				return true;
			return false;
		};
		size_t operator()(const blockpos_t& pointToHash) const noexcept {
			return ((((uint64_t)pointToHash.x) << 24) | (((uint64_t)pointToHash.y) << 16) | ((uint64_t)pointToHash.z));
		};
	};
	struct meshLoc {
		XMFLOAT3 scale;
		XMFLOAT4 qRot;
		uint8_t type;
		XMFLOAT3 pos;
	};
	struct treeLoc {
		XMFLOAT3 scale;
		float yaw;
		uint8_t type;
		blockpos_t pos;
	};
	uint16_t m_lowestZ;
	uint16_t m_highestZ;
	uint32_t m_id;
	uint16_t m_surfaceheight;
	unordered_map<blockpos_t, uint32_t, blockpos_t> m_cBlocks;
	unordered_map<blockpos_t, uint8_t, blockpos_t> m_Torches;
	std::vector<meshLoc> meshObjects;
	std::vector<treeLoc> trees;
	bool m_isAirChunk;
	char* m_chunkdata;
	explicit cyChunk(void);
	void loadChunk(sqlite3* db, uint32_t chunkID, bool fast = false);
	void airChunk(void);
	void solidChunk(void);
	void addMesh(meshLoc mesh);
	~cyChunk(void);

protected:
	sqlite3* m_db;
	uint64_t skipCdataArray(char* memory, uint64_t startpos, uint8_t elementsize);
	void loadCblockTmap(uint64_t startpos);
	void loadCustomBlocks(void);
	void loadMeshes(sqlite3* db);
};
