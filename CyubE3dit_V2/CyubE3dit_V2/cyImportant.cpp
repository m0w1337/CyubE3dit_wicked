#include "stdafx.h"
#include "cyImportant.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

cyImportant::cyImportant(void) {
	m_valid = false;
	m_stopped = true;
	for (uint8_t i = 0; i < 32; i++) {
		db[i] = nullptr;
	}
}

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

void cyImportant::loadWorldInfo(const std::string Worldname) {
	PWSTR path	  = NULL;
	string dbpath = "";
	m_filename.clear();

	if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &path) == S_OK)
	{
		m_filename = wstring(path) + L"\\cyubeVR\\Saved\\WorldData\\" + utf8_decode(Worldname);
		m_filename = find_importantFile(m_filename);
		dbpath	   = utf8_encode(path) + "\\cyubeVR\\Saved\\WorldData\\" + Worldname + "\\chunkdata.sqlite";
	}
	if (m_filename.size() > 1) {
		cyImportant::loadData(dbpath);
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

void cyImportant::loadData(const std::string dbpath) {
	ifstream file;
	m_stopped = true;
	cleaned = false;
	while (!cleaned) {
		Sleep(10);
	}
	m_valid = false;
	cleaned = false;
	m_chunkMap.clear();
	file.open(m_filename, fstream::in | ios::binary);
	if (file.is_open()) {
		uint32_t maplen;
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
		for (uint8_t i = 0; i < wiJobSystem::GetThreadCount(); i++) {
			if (db[i] != nullptr) {
				sqlite3_close(db[i]);
				db[i] = nullptr;
			}
			if (sqlite3_open_v2(dbpath.c_str(), &(db[i]), SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READONLY, NULL)) {
				if (sqlite3_open_v2(dbpath.c_str(), &(db[i]), SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READONLY, NULL)) {
					MessageBox(NULL, L"ERROR", L"World database not found.", MB_ICONWARNING);
					db[i] = nullptr;
					while (i < wiJobSystem::GetThreadCount()) {
						if (db[i] != nullptr) {
							sqlite3_close(db[i]);
							db[i] = nullptr;
						}
						i++;
					}
				}
			}
		}
		m_valid = true;
		m_stopped = false;
		file.close();
	}
}
bool cyImportant::isValid(void) {
	return m_valid;
}

bool cyImportant::isStopped(void) {
	return m_stopped;
}
// Convert a wide Unicode string to an UTF8 string
std::string cyImportant::utf8_encode(const std::wstring& wstr) {
	if (wstr.empty())
		return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring cyImportant::utf8_decode(const std::string& str) {
	if (str.empty())
		return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
