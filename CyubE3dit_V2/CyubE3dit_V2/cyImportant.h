#pragma once
#include <shlobj_core.h>
#include "sqlite3.h"

using namespace std;

constexpr uint32_t POS_WORLDSEED = 4;
constexpr uint32_t POS_NUMCHUNKS = 8;

class cyImportant {
public:
	static constexpr uint8_t MAX_THREADS   = 16;
	static constexpr uint8_t DBHANDLE_MAIN = MAX_THREADS;
	wstring m_worldFolder;
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
	atomic<bool> cleaned;
	atomic<bool> m_stopped;
	uint32_t m_seed;
	uint32_t m_numChunks;
	unordered_map<chunkpos_t, uint32_t, chunkpos_t> m_chunkMap;
	sqlite3* db[MAX_THREADS + 1];
	void loadWorldInfo(const std::string Worldname, bool cleanWorld = true);
	bool isValid(void);
	bool isStopped(void);
	bool getChunkID(const double x, const double y, uint32_t* chunkID);
	inline bool getChunkIDFast(const chunkpos_t chunkPos, uint32_t* chunkID);
	chunkpos_t getChunkPos(const double x, const double y);

	std::string utf8_encode(const std::wstring& wstr);
	std::wstring utf8_decode(const std::string& str);

protected:
	void loadData(const std::string dbpath, bool cleanWorld);
	wstring find_importantFile(const std::wstring path);
	wstring m_filename;
	
};
