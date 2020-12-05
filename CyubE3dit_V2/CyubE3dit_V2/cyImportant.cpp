#include "stdafx.h"
#include "cyImportant.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

wstring cyImportant::find_importantFile(const std::wstring path)  // placing path here if found
{
	fs::file_time_type filetime;
	wstring importantPath = L"";
	for (const auto& entry : fs::directory_iterator(path)) {
		if (entry.is_regular_file() && entry.path().extension().wstring() == L".important") {

			if (entry.last_write_time().time_since_epoch() > filetime.time_since_epoch()) {
				importantPath = entry.path().wstring();
				filetime	  = entry.last_write_time();
			}
		}
	}
	return importantPath;
}

void cyImportant::loadWorldInfo(const std::wstring Worldname) {
	PWSTR path = NULL;
	m_filename.clear();

	if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &path) == S_OK)
	{
		m_filename = wstring(path) + L"\\cyubeVR\\Saved\\WorldData\\" + Worldname;
		m_filename = find_importantFile(m_filename);
	}
	if (m_filename.size() > 1) {
		cyImportant::loadData(m_filename);
	}
}

bool cyImportant::getChunkID(const double x, const double y, uint32_t* chunkID) {
	chunkpos_t pos;
	pos.x = round(x / 16) * 16 - 8;
	pos.y = round(y / 16) * 16 - 8;
	if (x - pos.x > 15.5)
		pos.x += 16;
	if (y - pos.y > 15.5)
		pos.y += 16;
	std::unordered_map<chunkpos_t, uint32_t>::const_iterator chunk = m_chunkMap.find(pos);
	if (chunk == m_chunkMap.end())
		return false;
	*chunkID = chunk->second;
	return true;
}

void cyImportant::loadData(const std::wstring filename) {
	ifstream file;
	m_filename.clear();
	m_valid = false;
	m_chunkMap.clear();
	file.open(filename, fstream::in | ios::binary);
	if (file.is_open()) {
		uint32_t maplen;
		m_filename = filename;
		file.seekg(POS_WORLDSEED);
		file.read(reinterpret_cast<char*>(&m_seed), sizeof(m_seed));
		file.read(reinterpret_cast<char*>(&maplen), sizeof(maplen));
		for (uint32_t i = 0; i < maplen; i++) {
			chunkpos_t pos;
			file.read((char*)&(pos.x), 4);
			file.read((char*)&(pos.y), 4);
			pos.x /= 100;
			pos.y /= 100;
			m_chunkMap[pos] = i;
		}
		file.read(reinterpret_cast<char*>(&maplen), sizeof(maplen));  //unknown map
		file.seekg(maplen * 12 + 24 + 28, ios_base::cur);
		file.read(reinterpret_cast<char*>(&(m_playerpos.x)), sizeof(m_playerpos.x));
		file.read(reinterpret_cast<char*>(&(m_playerpos.y)), sizeof(m_playerpos.y));
		file.read(reinterpret_cast<char*>(&(m_playerpos.z)), sizeof(m_playerpos.z));
		file.close();
	}
}
bool cyImportant::isValid(void) {

	return false;
}
