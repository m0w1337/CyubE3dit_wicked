#pragma once

constexpr uint32_t POS_WORLDSEED = 4;
constexpr uint32_t POS_NUMCHUNKS = 8;

class cyImportant {
public:
	struct playerpos_t {
		double x;
		double y;
		double z;
	};
	struct chunkpos_t {
		int32_t x;
		int32_t y;
		bool operator==(chunkpos_t i) {
			if (i.x == x/16 && i.y == y/16) {
				return true;
			} else {
				return false;
			}
		}
	};
	playerpos_t m_playerpos;
	bool m_valid;
	uint32_t m_seed;
	uint32_t m_numChunks;
	unordered_map<chunkpos_t, uint32_t> m_chunkMap;
	void loadWorldInfo(const std::string Worldname);
	void loadData(const std::string filename);
	bool isValid(void);

protected:
	

	string m_filename;
};
