#include "stdafx.h"
#include "cyBlocks.h"

namespace fs = std::filesystem;
using namespace std;
using namespace wiECS;
using namespace wiScene;

uint8_t cyBlocks::m_regBlockTypes[256]	= {0};
Entity cyBlocks::m_regBlockMats[256][6] = {{0}};
Entity cyBlocks::m_variationMat			= 0;
uint8_t cyBlocks::m_voidID				= 255;
std::unordered_map<uint32_t, cyBlocks::cBlock_t> cyBlocks::m_cBlockTypes;

cyBlocks::cyBlocks(wiScene::Scene& scene) :
  m_scene(scene) {
}

void cyBlocks::LoadRegBlocks(void) {
	ifstream file;
	file.open("data/blocks.json", fstream::in);
	wiScene::MaterialComponent* material;

	m_variationMat = m_scene.Entity_CreateMaterial("variation");
	material	   = m_scene.materials.GetComponent(m_variationMat);
	material->SetBaseColor(XMFLOAT4(1, 0, 0, 0));
	material->SetDirty(true);

	if (file.is_open()) {
		json j_complete = json::parse(file);

		for (json::iterator it = j_complete.begin(); it != j_complete.end(); ++it) {
			if (it.key() == "SolidBlock") {
				for (size_t i = 0; i < it.value().size(); i++) {
					catchRegularBlockSpecs(it, i, BLOCKTYPE_SOLID);
				}
			} else if (it.key() == "AlphaBlock") {
				for (size_t i = 0; i < it.value().size(); i++) {
					catchRegularBlockSpecs(it, i, BLOCKTYPE_ALPHA);
				}
			} else if (it.key() == "Billboard") {
				for (size_t i = 0; i < it.value().size(); i++) {
					catchRegularBlockSpecs(it, i, BLOCKTYPE_BILLBOARD);
				}
			} else if (it.key() == "VoidBlock") {
				for (size_t i = 0; i < it.value().size(); i++) {
					catchRegularBlockSpecs(it, i, BLOCKTYPE_VOID);
				}
			} else if (it.key() == "ModBlock") {
				for (size_t i = 0; i < it.value().size(); i++) {
					catchRegularBlockSpecs(it, i, BLOCKTYPE_MOD);
				}
			} else if (it.key() == "FunktionalMesh") {
				for (size_t i = 0; i < it.value().size(); i++) {
					catchRegularBlockSpecs(it, i, BLOCKTYPE_FUNCMESH);
				}
			} else if (it.key() == "DecorationalMesh") {
				for (size_t i = 0; i < it.value().size(); i++) {
					catchRegularBlockSpecs(it, i, BLOCKTYPE_DECOMESH);
				}
			}
		}
	}
}

void cyBlocks::catchRegularBlockSpecs(const json::iterator& it, const size_t i, const blocktype_t blocktype) {
	json::iterator bSpec;
	matType_t materialType;
	string tex = "";
	uint8_t id = 0;
	try {
		id = (matType_t)it.value().at(i).at("id");
	}
	catch (...) {
		return;
	}
	m_regBlockTypes[id] = blocktype;
	if (blocktype == BLOCKTYPE_VOID) {
		m_voidID = id;
	}
	/*try {
		materialType = (matType_t)it.value().at(i).at("textype");
	}
	catch (...) {
		materialType = BLOCKMAT_SINGLE;
	}*/
	try {
		m_regBlockNames[id] = it.value().at(i).at("text");
	}
	catch (...) {
		m_regBlockNames[id] = "";
	}
	wiScene::MaterialComponent* material;
	
	
	for (uint8_t ft = 0; ft < 6; ft++) {
		try {
			tex						   = it.value().at(i).at("texture" + std::to_string(ft));
			m_regBlockMats[id][ft]	   = m_scene.Entity_CreateMaterial("bMat" + std::to_string(id) + std::to_string(ft));
			material				   = m_scene.materials.GetComponent(m_regBlockMats[id][ft]);
			material->baseColorMapName = "images/" + tex;
			if ((id == 1 && ft == 0) || (id == 0 && ft == 0)) {
				material->SetUseVertexColors(true);
			}

			material->SetReflectance(0.001);
			material->SetMetalness(0.001);
			material->SetEmissiveStrength(0);
			material->SetRoughness(0.9);

			try {
				tex						= it.value().at(i).at("normal" + std::to_string(ft));
				material->normalMapName = "images/" + tex;
				material->SetNormalMapStrength(1.0);
				tex						 = it.value().at(i).at("surface" + std::to_string(ft));
				material->surfaceMapName = "images/" + tex;
				material->SetMetalness(0.9);
				material->SetRoughness(0.3);
				material->SetReflectance(0.4);
			}
			catch (...) {
				try {
					tex						   = it.value().at(i).at("occlusion" + std::to_string(ft));
					material->occlusionMapName = "images/" + tex;
					material->SetParallaxOcclusionMapping(1.0);
				}
				catch (...) {
				}
			}
			material->SetDirty();
		}
		catch (...) {
		}
	}

	if (blocktype != BLOCKTYPE_BILLBOARD) {
		for (uint8_t ft = 1; ft < 6; ft++) {
			if (m_regBlockMats[id][ft] == 0)
				m_regBlockMats[id][ft] = m_regBlockMats[id][ft - 1];
		}
	} else {
		m_scene.materials.GetComponent(m_regBlockMats[id][0])->SetAlphaRef(0.5);
		m_scene.materials.GetComponent(m_regBlockMats[id][0])->SetUseWind(true);
	}
}

void cyBlocks::LoadCustomBlocks(void) {
	fs::file_time_type filetime;
	wstring SteamApps;
	if (GetStringRegKey(L"SOFTWARE\\Valve\\Steam", L"InstallPath", SteamApps) == false) {
		if (GetStringRegKey(L"SOFTWARE\\Wow6432Node\\Valve\\Steam", L"InstallPath", SteamApps) == false) {
			SteamApps = L"C:\\Program Files (x86)\\Steam";
		}
	}

	wstring customPath = SteamApps + L"\\steamapps\\workshop\\content\\619500\\";
	//&&entry.path().extension().wstring() == L".important"
	for (const auto& entry : fs::directory_iterator(customPath)) {
		if (entry.is_directory()) {
			string tmpname;
			string tmpcreator;
			uint32_t tmpID	= 0;
			uint8_t tmpMode = 0;
			bool tmpNormals = false;
			uint8_t ok		= 0;
			ifstream file;
			file.open(entry.path().wstring() + L"\\Properties.json", fstream::in);
			if (file.is_open()) {
				json j_complete = json::parse(file);
				for (json::iterator it = j_complete.begin(); it != j_complete.end(); ++it) {
					if (iequals(it.key(), "name")) {
						tmpname = it.value();
						ok |= 1;
					} else if (iequals(it.key(), "creatorname")) {
						tmpcreator = it.value();
					} else if (iequals(it.key(), "uniqueid")) {
						tmpID = it.value();
						ok |= 2;
					} else if (iequals(it.key(), "textures")) {
						for (json::iterator it2 = j_complete.begin(); it2 != j_complete.end(); ++it2) {
							if (iequals(it2.key(), "mode")) {
								tmpMode = it.value();
								ok |= 4;
							} else if (iequals(it2.key(), "withnormals")) {
								tmpNormals = it.value();
							}
						}
					}
				}
			}
			if (ok == 7) {
				m_cBlockTypes[tmpID].name	 = tmpname;
				m_cBlockTypes[tmpID].creator = tmpcreator;
				for (const auto& texdir : fs::directory_iterator(entry.path().wstring() + L"\\Textures")) {
					if (texdir.is_regular_file() && iequals(texdir.path().extension().string(), ".dds")) {
					}
				}
			}
		}
	}
}

bool cyBlocks::GetStringRegKey(const std::wstring& key, const std::wstring& strValueName, std::wstring& strValue) {
	HKEY hKey;
	LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &hKey);
	strValue  = L"";
	if (lRes == ERROR_SUCCESS) {
		WCHAR szBuffer[1024];
		DWORD dwBufferSize = sizeof(szBuffer);
		ULONG nError;
		nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
		if (ERROR_SUCCESS == nError)
		{
			strValue = szBuffer;
			return true;
		}
	}
	return false;
}

bool cyBlocks::iequals(string str1, string str2) {
	//convert s1 and s2 into lower case strings
	transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
	transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
	if (str1.compare(str2) == 0)
		return true;  //The strings are same
	return false;	  //not matched
}

/*
bool cyBlocks::iequals(const string& a, const string& b) {
	return std::equal(a.begin(), a.end(), b.begin(),
					  [](char a, char b) {
						  return tolower(a) == tolower(b);
					  });
}*/
