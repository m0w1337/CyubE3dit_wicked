#include "stdafx.h"
#include "cyWorlds.h"
#include <shlobj_core.h>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

std::vector<std::string> cyWorlds::worlds;

void cyWorlds::getWorlds(void) {
	PWSTR path = NULL;
	wstring worldpath;
	if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &path) == S_OK)
	{
		worldpath = wstring(path) + L"\\cyubeVR\\Saved\\WorldData\\";
	}
	try {
		for (const auto& entry : fs::directory_iterator(worldpath)) {
			if (entry.is_directory()) {
				worlds.push_back(entry.path().filename().string());
			}
		}
	}
	catch (...) {
	}
}