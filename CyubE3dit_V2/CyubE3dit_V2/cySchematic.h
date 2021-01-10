#pragma once
#include "cyChunk.h"
#include "ChunkLoader.h"

class cySchematic {
public:
	static constexpr float PI		  = 3.141592653589793238462643383279502884f;
	static const uint32_t HEADER_SIZE = 20;
	static const uint32_t MAGICBYTE = 0x13371337;
	struct blockpos_t {
		uint32_t x;
		uint32_t y;
		uint32_t z;
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
	struct torch_t {
		torch_t() :
		  ID(0),
		  x(0),
		  y(0),
		  z(0),
		  rotation(0){};
		torch_t(const uint8_t& _ID, const uint16_t& _x, const uint16_t& _y, const uint16_t& _z, uint8_t _rotation) :
		  ID(_ID),
		  x(_x),
		  y(_y),
		  z(_z),
		  rotation(_rotation){};
		uint8_t ID;
		uint16_t x;
		uint16_t y;
		uint16_t z;
		uint8_t rotation;
	};
	blockpos_t size;
	static unordered_map<wiECS::Entity, cySchematic*> m_schematics;
	unordered_map<blockpos_t, uint32_t, blockpos_t> m_cBlocks;
	unordered_map<blockpos_t, uint8_t, blockpos_t> m_Torches;
	std::vector<cyChunk::meshLoc> meshObjects;
	std::vector<cyChunk::treeLoc> trees;
	bool m_isAirChunk;
	char* m_chunkdata;
	cySchematic(string filename);
	static void addSchematic(std::string filename);
	wiECS::Entity RenderSchematic(const int32_t relX, const int32_t relY, const int32_t relZ);

private:
	void loadCustomBlocks(void);
};
