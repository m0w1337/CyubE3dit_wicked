#pragma once
#include <shlobj_core.h>
#include "sqlite3.h"

using namespace std;

constexpr uint32_t POS_WORLDSEED = 4;
constexpr uint32_t POS_NUMCHUNKS = 8;

class cyImportant {
public:
	explicit cyImportant(void);
	struct playerpos_t {
		double x;
		double y;
		double z;
	};

	struct chunkpos_t {
		int32_t x;
		int32_t y;
		chunkpos_t() :
		  x(0),
		  y(0){};
		chunkpos_t(const int32_t& _x, const int32_t& _y) :
		  x(_x),
		  y(_y){};
		chunkpos_t(const chunkpos_t& other) {
			x = other.x;
			y = other.y;
		};

		chunkpos_t& operator=(const chunkpos_t& other) {
			x = other.x;
			y = other.y;
			return *this;
		};

		bool operator==(const chunkpos_t& other) const {
			if (x == other.x && y == other.y)
				return true;
			return false;
		};

		chunkpos_t& operator+(const chunkpos_t& other) {
			x += other.x;
			y += other.y;
			return *this;
		};
		chunkpos_t& operator-(const chunkpos_t& other) {
			x -= other.x;
			y -= other.y;
			return *this;
		};

		bool operator<(const chunkpos_t& other) {
			if (x < other.x)
				return true;
			else if (x == other.x && y == other.y)
				return true;

			return false;
		};
		size_t operator()(const chunkpos_t& pointToHash) const noexcept {
			return ((uint64_t)pointToHash.x) << 32 | (uint64_t)pointToHash.y;
		};
	};
	playerpos_t m_playerpos;
	
	bool m_valid;
	uint32_t m_seed;
	uint32_t m_numChunks;
	unordered_map<chunkpos_t, uint32_t, chunkpos_t> m_chunkMap;
	sqlite3* db[32];
	void loadWorldInfo(const std::wstring Worldname);
	void loadData(const std::wstring filename);
	bool isValid(void);
	bool getChunkID(const double x, const double y, uint32_t* chunkID);

protected:
	wstring find_importantFile(const std::wstring path);
	wstring m_filename;
};
