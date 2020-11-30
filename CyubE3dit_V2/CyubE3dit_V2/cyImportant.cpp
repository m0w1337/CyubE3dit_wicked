#include "stdafx.h"
#include "cyImportant.h"

using namespace std;

void cyImportant::loadWorldInfo(const std::string Worldname) {

	cyImportant::loadData(const std::string filename)
}

void cyImportant::loadData(const std::string filename) {
	ifstream file;
	m_filename = "";
	m_valid	   = false;
	m_chunkMap.clear();
	file.open(filename, fstream::in);
	if (file.is_open()) {
		uint32_t maplen;
		m_filename = filename;
		file.seekg(POS_WORLDSEED);
		file.read((char*)&m_seed, sizeof(m_seed));
		file.seekg(POS_NUMCHUNKS);
		file.read((char*)&maplen, sizeof(maplen));
		for (uint32_t i = 0; i < maplen; i++) {
			chunkpos_t pos;
			file.read((char*)&(pos.x), 4);
			file.read((char*)&(pos.y), 4);
			m_chunkMap[pos] = i;
		}
		file.read((char*)&maplen, sizeof(maplen));	//unknown map
		file.seekg(maplen * 12, ios_base::cur);
		file.read((char*)&(m_playerpos.x), sizeof(m_playerpos.x));
		file.read((char*)&(m_playerpos.y), sizeof(m_playerpos.y));
		file.read((char*)&(m_playerpos.z), sizeof(m_playerpos.z));
		file.close();
	}
}
bool cyImportant::isValid(void) {
}
