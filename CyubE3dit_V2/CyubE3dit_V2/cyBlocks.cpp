#include "stdafx.h"
#include "cyBlocks.h"

namespace fs = std::filesystem;
using namespace std;
using namespace wiECS;
using namespace wiScene;

uint8_t cyBlocks::m_regBlockTypes[256]	  = {wiECS::INVALID_ENTITY};
uint8_t cyBlocks::m_regBlockFlags[256][6] = {{wiECS::INVALID_ENTITY}};
Entity cyBlocks::m_regBlockMats[256][6]	  = {{wiECS::INVALID_ENTITY}};
std::unordered_map<uint32_t, cyBlocks::mesh_t> cyBlocks::m_regMeshes;
uint8_t cyBlocks::m_voidID	   = 255;
uint8_t cyBlocks::m_torchID	   = 0;
Entity cyBlocks::m_fallbackMat = wiECS::INVALID_ENTITY;
bool cyBlocks::m_loaded		   = false;
std::vector<Entity> cyBlocks::m_treeMeshes;
Entity cyBlocks::m_toolMeshes[];
std::string cyBlocks::m_regBlockNames[256] = {""};
wiScene::Scene& cyBlocks::m_scene		   = wiScene::GetScene();
std::unordered_map<uint32_t, cyBlocks::cBlock_t> cyBlocks::m_cBlockTypes;

void cyBlocks::LoadRegBlocks(void) {
	m_fallbackMat = m_scene.Entity_CreateMaterial("fallbackMat");
	ifstream file;
	file.open("data/blocks.json", fstream::in);
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
			} else if (it.key() == "Torch") {
				for (size_t i = 0; i < it.value().size(); i++) {
					catchRegularMeshSpecs(it, i, BLOCKTYPE_TORCH);
				}
			} else if (it.key() == "ModBlock") {
				for (size_t i = 0; i < it.value().size(); i++) {
					catchRegularBlockSpecs(it, i, BLOCKTYPE_MOD);
				}
			} else if (it.key() == "FunctionalMesh") {
				//for (size_t i = 0; i < it.value().size(); i++) {
				//catchRegularBlockSpecs(it, i, BLOCKTYPE_FUNCMESH);
				//}
			} else if (it.key() == "DecoMesh") {
				for (size_t i = 0; i < it.value().size(); i++) {
					catchRegularMeshSpecs(it, i, BLOCKTYPE_DECOMESH);
				}
			}
		}
	}
	m_loaded = true;
}

void cyBlocks::loadMeshes(void) {
	m_toolMeshes[TOOL_ROT]= ImportModel_OBJ("data\\meshes\\rotCW.obj", wiScene::GetScene(), 0);
	m_toolMeshes[TOOL_ORIGIN] = ImportModel_OBJ("data\\meshes\\originOnly.obj", wiScene::GetScene(), 0);
	m_toolMeshes[TOOL_XAXIS] = ImportModel_OBJ("data\\meshes\\xAxis.obj", wiScene::GetScene(), 0);
	m_toolMeshes[TOOL_YAXIS] = ImportModel_OBJ("data\\meshes\\yAxis.obj", wiScene::GetScene(), 0);
	m_toolMeshes[TOOL_ZAXIS] = ImportModel_OBJ("data\\meshes\\zAxis.obj", wiScene::GetScene(), 0);
	m_toolMeshes[TOOL_PLANE] = ImportModel_OBJ("data\\meshes\\plane.obj", wiScene::GetScene(), 0);

	m_treeMeshes.push_back(ImportModel_OBJ("data\\trees\\tree1.obj", wiScene::GetScene(), 2));
	m_treeMeshes.push_back(ImportModel_OBJ("data\\trees\\tree2.obj", wiScene::GetScene(), 2));
	m_treeMeshes.push_back(ImportModel_OBJ("data\\trees\\tree2b_lod0.obj", wiScene::GetScene(), 2));
	m_treeMeshes.push_back(ImportModel_OBJ("data\\trees\\tree3.obj", wiScene::GetScene(), 2));
	m_treeMeshes.push_back(ImportModel_OBJ("data\\trees\\cactus.obj", wiScene::GetScene(), 0));
	m_treeMeshes.push_back(ImportModel_OBJ("data\\trees\\dgrass.obj", wiScene::GetScene(), 1));

}

void cyBlocks::catchRegularMeshSpecs(const json::iterator& it, const size_t i, const blocktype_t blocktype) {
	string meshObj = "";
	mesh_t mesh;
	uint8_t id = 0;
	try {
		id = (matType_t)it.value().at(i).at("id");
	}
	catch (...) {
		return;
	}
	m_regBlockTypes[id] = blocktype;
	if (blocktype != BLOCKTYPE_TORCH || m_torchID == 0) {
		mesh.type	 = blocktype;
		mesh.mesh[0] = 0;
		mesh.mesh[1] = 0;
		try {
			mesh.name = it.value().at(i).at("text");
		}
		catch (...) {
		}
		try {
			uint8_t meshAttr = it.value().at(i).at("textype");
			meshObj			 = it.value().at(i).at("mesh0");
			mesh.mesh[0]	 = ImportModel_OBJ("data\\meshes\\" + meshObj, wiScene::GetScene(), 0);
			mesh.material[0] = mesh.mesh[0] - 1;
			if (meshAttr == 5) {
				wiScene::MaterialComponent* mat = wiScene::GetScene().materials.GetComponent(mesh.material[0]);
				mat->SetUseWind(true);
				mat->SetCastShadow(false);
			}
			meshObj			 = it.value().at(i).at("mesh1");
			mesh.mesh[1]	 = ImportModel_OBJ("data\\meshes\\" + meshObj, wiScene::GetScene(), 0);
			mesh.material[1] = mesh.mesh[1] - 1;
			if (blocktype == BLOCKTYPE_TORCH)
				m_torchID = id;
		}
		catch (...) {
		}
		if (mesh.mesh[0]) {
			m_regMeshes[id] = mesh;
		}
	}
	try {  //catch torch color in any case
		m_regBlockFlags[id][0] = it.value().at(i).at("LightR");
		m_regBlockFlags[id][1] = it.value().at(i).at("LightG");
		m_regBlockFlags[id][2] = it.value().at(i).at("LightB");
	}
	catch (...) {
	}
}

void cyBlocks::catchRegularBlockSpecs(const json::iterator& it, const size_t i, const blocktype_t blocktype) {
	json::iterator bSpec;
	matType_t materialType;
	string tex				= "";
	uint8_t id				= 0;
	wiScene::Scene& m_scene = wiScene::GetScene();
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
			material->SetBaseColor(XMFLOAT4(1.0f, 1.0f, 1.0f, .0f));
			material->textures[MaterialComponent::BASECOLORMAP].name = "images/" + tex;
			material->textures[MaterialComponent::BASECOLORMAP].resource = wiResourceManager::Load("images/" + tex);
			/*if ((id == 1 && ft == 0) || blocktype == 0 || blocktype == 13 || blocktype == 12 || blocktype == 25 || blocktype == 26 || blocktype == 67) {
				material->SetUseVertexColors(true);
			}*/
			if (blocktype == BLOCKTYPE_ALPHA) {
				material->SetReflectance(0.09);
				//material->userBlendMode = BLENDMODE_ALPHA;
				material->SetTransmissionAmount(0.99);
				material->SetRefractionAmount(0.01f);
				material->SetMetalness(0.03f);
				material->SetRoughness(0.05f);
				material->SetCastShadow(true);
			} else {
				material->SetReflectance(0);
				material->SetMetalness(0);
				material->SetEmissiveStrength(0);
				material->SetRoughness(1);
				material->SetBaseColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
			}
			try {
				bool antitile = it.value().at(i).at("antitile" + std::to_string(ft));
				if (antitile) {
					m_regBlockFlags[id][ft] |= cyBlocks::BLOCKFLAG_ANTITILE;
					material->SetUseVertexColors(true);
				} else {
					m_regBlockFlags[id][ft] &= ~cyBlocks::BLOCKFLAG_ANTITILE;
				}
			}
			catch (...) {
				m_regBlockFlags[id][ft] &= ~cyBlocks::BLOCKFLAG_ANTITILE;
			}
			try {
				tex						= it.value().at(i).at("normal" + std::to_string(ft));
				material->textures[MaterialComponent::NORMALMAP].name	 = "images/" + tex;
				material->textures[MaterialComponent::NORMALMAP].resource = wiResourceManager::Load("images/" + tex);
				material->SetNormalMapStrength(1.0);
				tex						 = it.value().at(i).at("surface" + std::to_string(ft));
				material->textures[MaterialComponent::SURFACEMAP].name	  = "images/" + tex;
				material->textures[MaterialComponent::SURFACEMAP].resource = wiResourceManager::Load("images/" + tex);
				material->SetCustomShaderID(MaterialComponent::SHADERTYPE_PBR_PARALLAXOCCLUSIONMAPPING);
				material->SetParallaxOcclusionMapping(1.0);
				material->SetClearcoatFactor(1.);
				material->SetClearcoatRoughness(.5);
				material->SetMetalness(1.0f);
				material->SetRoughness(1.0f);
				material->SetReflectance(1.0f);
			}
			catch (...) {
			}
			try {
				tex						   = it.value().at(i).at("occlusion" + std::to_string(ft));
				material->textures[MaterialComponent::OCCLUSIONMAP].name	   = "images/" + tex;
				material->textures[MaterialComponent::OCCLUSIONMAP].resource = wiResourceManager::Load("images/" + tex);
				material->SetParallaxOcclusionMapping(1.0);
				material->SetCustomShaderID(MaterialComponent::SHADERTYPE_PBR_PARALLAXOCCLUSIONMAPPING);
			}
			catch (...) {
			}
			try {
				tex						  = it.value().at(i).at("glow" + std::to_string(ft));
				material->textures[MaterialComponent::EMISSIVEMAP].name	   = "images/" + tex;
				material->textures[MaterialComponent::EMISSIVEMAP].resource = wiResourceManager::Load("images/" + tex);
				material->SetEmissiveStrength(15.0f);
			}
			catch (...) {
			}
			material->SetDirty();
		}
		catch (...) {
			if (ft == 0) {	//Support blocks without texture at all
				m_regBlockMats[id][ft] = m_scene.Entity_CreateMaterial("bMat" + std::to_string(id) + std::to_string(ft));
				material			   = m_scene.materials.GetComponent(m_regBlockMats[id][ft]);
				if (blocktype == BLOCKTYPE_ALPHA) {
					material->SetReflectance(0.09);
					material->SetTransmissionAmount(0.99);
					material->SetRefractionAmount(0.01f);
					material->SetBaseColor(XMFLOAT4(1., 1., 1., 1.f));
					material->SetMetalness(0.03f);
					material->SetRoughness(0.05f);
					material->SetCastShadow(true);
				} else {
					material->SetReflectance(0);
					material->SetMetalness(0);
					material->SetEmissiveStrength(0);
					material->SetRoughness(1);
					material->SetBaseColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
				}
			}
		}
	}

	if (blocktype != BLOCKTYPE_BILLBOARD) {
		for (uint8_t ft = 1; ft < 6; ft++) {
			if (m_regBlockMats[id][ft] == 0) {
				m_regBlockMats[id][ft]	= m_regBlockMats[id][ft - 1];
				m_regBlockFlags[id][ft] = m_regBlockFlags[id][ft - 1];
			}
		}
	} else {
		//m_scene.materials.GetComponent(m_regBlockMats[id][0])->userBlendMode = BLENDMODE_ALPHA;
		m_scene.materials.GetComponent(m_regBlockMats[id][0])->SetAlphaRef(0.2f);
		m_scene.materials.GetComponent(m_regBlockMats[id][0])->SetReceiveShadow(false);
		m_scene.materials.GetComponent(m_regBlockMats[id][0])->SetUseWind(true);
		m_scene.materials.GetComponent(m_regBlockMats[id][0])->SetCastShadow(false);
		m_scene.materials.GetComponent(m_regBlockMats[id][0])->SetUseVertexColors(false);
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
	bool found = false;
	struct _stat64i32 info;
	wstring workshopdir = SteamApps + L"\\steamapps\\workshop\\content\\619500\\";
	if (_wstat(workshopdir.c_str(), &info) == 0) {
		if (info.st_mode & _S_IFDIR) {	//workshof folder not found in standard steam install directory --> search in secondary library paths
			found = true;
		}
	}
	if (!found) {  //extremely dirty vdf file parser to extract the library paths for steam installs searching for cyubeVR
		wifstream file;
		wstring linebuffer;
		file.open(SteamApps + L"\\steamapps\\libraryfolders.vdf", fstream::in);
		if (file.is_open()) {
			while (!file.eof()) {
				getline(file, linebuffer);
				size_t key_start = linebuffer.find('"');
				if (key_start != wstring::npos) {
					size_t key_end = linebuffer.find('"', ++key_start);
					int64_t entry  = wcstol(linebuffer.substr(key_start, key_end - key_start).c_str(), nullptr, 10);
					if (entry) {
						size_t val_start = linebuffer.find('"', key_end + 1);
						if (val_start != wstring::npos) {
							size_t val_end = linebuffer.find('"', ++val_start);
							workshopdir	   = linebuffer.substr(val_start, val_end - val_start) + L"\\steamapps\\workshop\\content\\619500\\";
							if (_wstat(workshopdir.c_str(), &info) == 0) {
								if (info.st_mode & _S_IFDIR) {	//workshof folder not found in standard steam install directory --> search in secondary library paths
									SteamApps = linebuffer.substr(val_start, val_end - val_start);
									found	  = true;
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	if (!found) {
		wiHelper::messageBox("CyubeVR install directory was not found, your custom blocks might look not so nice.");
	}

	wstring customPath = SteamApps + L"\\steamapps\\workshop\\content\\619500\\";
	//&&entry.path().extension().wstring() == L".important"
	addCustomBlocksPath(customPath);
	//Add the local custom blocks to the list afterwards to overwrite possible with the local version
	customPath = SteamApps + L"\\steamapps\\common\\cyubeVR\\cyubeVR\\Mods\\Blocks\\";
	//&&entry.path().extension().wstring() == L".important"
	addCustomBlocksPath(customPath);
}

void cyBlocks::addCustomBlocksPath(wstring customPath) {
	wiScene::Scene& m_scene = wiScene::GetScene();
	try {
		for (const auto& entry : fs::directory_iterator(customPath)) {
			if (entry.is_directory()) {
				string tmpname;
				string tmpcreator;
				uint32_t tmpID	= 0;
				uint8_t tmpMode = 0;
				wiScene::MaterialComponent* material;
				bool tmpNormals = false;
				uint32_t tmpAnimation = 0;
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
						} else if (iequals(it.key(),"animationspeed")) {
							tmpAnimation = it.value();
						} else if (iequals(it.key(), "textures")) {
							for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2) {
								if (iequals(it2.key(), "mode")) {
									tmpMode = it2.value();
									ok |= 4;
								} else if (iequals(it2.key(), "withnormals")) {
									tmpNormals = it2.value();
								}
							}
						}
					}
				}
				if (ok == 7) {
					m_cBlockTypes[tmpID].name	 = tmpname;
					m_cBlockTypes[tmpID].creator = tmpcreator;
					try {
						for (const auto& texdir : fs::directory_iterator(entry.path().wstring() + L"\\Textures")) {
							if (texdir.is_regular_file()) {
								if (iequals(texdir.path().filename().string(), "all.dds") || iequals(texdir.path().filename().string(), "up.dds") || iequals(texdir.path().filename().string(), "updown.dds")) {
									m_cBlockTypes[tmpID].material[0] = m_scene.Entity_CreateMaterial("");
									material						 = m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[0]);
									material->textures[MaterialComponent::BASECOLORMAP].name	   = texdir.path().string();
									material->textures[MaterialComponent::BASECOLORMAP].resource = wiResourceManager::Load(texdir.path().string());
									if (tmpAnimation) {
										material->texAnimDirection = XMFLOAT2(0.25, 0.25);
										material->texMulAdd		   = XMFLOAT4(0.25, 0.25, 0, 0);
										material->texAnimFrameRate = (float)tmpAnimation/30;
									}
									material->SetReflectance(0.0f);
									material->SetMetalness(0.0f);
									material->SetEmissiveStrength(0.0f);
									material->SetRoughness(1.0f);
								} else if (iequals(texdir.path().filename().string(), "down.dds")) {
									m_cBlockTypes[tmpID].material[1] = m_scene.Entity_CreateMaterial("");
									material						 = m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[1]);
									material->textures[MaterialComponent::BASECOLORMAP].name	 = texdir.path().string();
									material->textures[MaterialComponent::BASECOLORMAP].resource = wiResourceManager::Load(texdir.path().string());
									material->SetReflectance(0.0f);
									material->SetMetalness(0.0f);
									material->SetEmissiveStrength(0.0f);
									material->SetRoughness(1.0f);
								} else if (iequals(texdir.path().filename().string(), "sides.dds")) {
									m_cBlockTypes[tmpID].material[2] = m_scene.Entity_CreateMaterial("");
									material						 = m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[2]);
									material->textures[MaterialComponent::BASECOLORMAP].name	 = texdir.path().string();
									material->textures[MaterialComponent::BASECOLORMAP].resource = wiResourceManager::Load(texdir.path().string());
									material->SetReflectance(0.0f);
									material->SetMetalness(0.0f);
									material->SetEmissiveStrength(0.0f);
									material->SetRoughness(1.0f);
								} else if (iequals(texdir.path().filename().string(), "left.dds")) {
									m_cBlockTypes[tmpID].material[3] = m_scene.Entity_CreateMaterial("");
									material						 = m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[3]);
									material->textures[MaterialComponent::BASECOLORMAP].name	 = texdir.path().string();
									material->textures[MaterialComponent::BASECOLORMAP].resource = wiResourceManager::Load(texdir.path().string());
									material->SetReflectance(0.0f);
									material->SetMetalness(0.0f);
									material->SetEmissiveStrength(0.0f);
									material->SetRoughness(1.0f);
								} else if (iequals(texdir.path().filename().string(), "right.dds")) {
									m_cBlockTypes[tmpID].material[2] = m_scene.Entity_CreateMaterial("");
									material						 = m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[2]);
									material->textures[MaterialComponent::BASECOLORMAP].name	 = texdir.path().string();
									material->textures[MaterialComponent::BASECOLORMAP].resource = wiResourceManager::Load(texdir.path().string());
									material->SetReflectance(0.0f);
									material->SetMetalness(0.0f);
									material->SetEmissiveStrength(0.0f);
									material->SetRoughness(1.0f);
								} else if (iequals(texdir.path().filename().string(), "front.dds")) {
									m_cBlockTypes[tmpID].material[5] = m_scene.Entity_CreateMaterial("");
									material						 = m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[5]);
									material->textures[MaterialComponent::BASECOLORMAP].name	 = texdir.path().string();
									material->textures[MaterialComponent::BASECOLORMAP].resource = wiResourceManager::Load(texdir.path().string());
									material->SetReflectance(0.0f);
									material->SetMetalness(0.0f);
									material->SetEmissiveStrength(0.0f);
									material->SetRoughness(1.0f);
								} else if (iequals(texdir.path().filename().string(), "back.dds")) {
									m_cBlockTypes[tmpID].material[4] = m_scene.Entity_CreateMaterial("");
									material						 = m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[4]);
									material->textures[MaterialComponent::BASECOLORMAP].name	 = texdir.path().string();
									material->textures[MaterialComponent::BASECOLORMAP].resource = wiResourceManager::Load(texdir.path().string());
									material->SetReflectance(0.0f);
									material->SetMetalness(0.0f);
									material->SetEmissiveStrength(0.0f);
									material->SetRoughness(1.0f);
								}
							}
						}
					}
					catch (...) {
					}
					for (const auto& texdir : fs::directory_iterator(entry.path().wstring() + L"\\Textures")) {
						if (texdir.is_regular_file()) {
							if (iequals(texdir.path().filename().string(), "all_normal.dds") || iequals(texdir.path().filename().string(), "up_normal.dds") || iequals(texdir.path().filename().string(), "updown_normal.dds")) {
								if (m_cBlockTypes[tmpID].material[0]) {
									material				= m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[0]);
									material->textures[MaterialComponent::NORMALMAP].name	 = texdir.path().string();
									material->textures[MaterialComponent::NORMALMAP].resource = wiResourceManager::Load(texdir.path().string());
									//material->SetNormalMapStrength(-3.0);
								}
							} else if (iequals(texdir.path().filename().string(), "down_normal.dds")) {
								if (m_cBlockTypes[tmpID].material[1]) {
									material				= m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[1]);
									material->textures[MaterialComponent::NORMALMAP].name	 = texdir.path().string();
									material->textures[MaterialComponent::NORMALMAP].resource = wiResourceManager::Load(texdir.path().string());
									//material->SetNormalMapStrength(-3.0);
								}
							} else if (iequals(texdir.path().filename().string(), "sides_normal.dds")) {
								if (m_cBlockTypes[tmpID].material[2]) {
									material				= m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[2]);
									material->textures[MaterialComponent::NORMALMAP].name	  = texdir.path().string();
									material->textures[MaterialComponent::NORMALMAP].resource = wiResourceManager::Load(texdir.path().string());
									//material->SetNormalMapStrength(-3.0);
								}
							} else if (iequals(texdir.path().filename().string(), "left_normal.dds")) {
								if (m_cBlockTypes[tmpID].material[3]) {
									material				= m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[3]);
									material->textures[MaterialComponent::NORMALMAP].name	  = texdir.path().string();
									material->textures[MaterialComponent::NORMALMAP].resource = wiResourceManager::Load(texdir.path().string());
									//material->SetNormalMapStrength(-3.0);
								}
							} else if (iequals(texdir.path().filename().string(), "right_normal.dds")) {
								if (m_cBlockTypes[tmpID].material[2]) {
									material				= m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[2]);
									material->textures[MaterialComponent::NORMALMAP].name	  = texdir.path().string();
									material->textures[MaterialComponent::NORMALMAP].resource = wiResourceManager::Load(texdir.path().string());
									//material->SetNormalMapStrength(-3.0);
								}
							} else if (iequals(texdir.path().filename().string(), "front_normal.dds")) {
								if (m_cBlockTypes[tmpID].material[5]) {
									material				= m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[5]);
									material->textures[MaterialComponent::NORMALMAP].name	  = texdir.path().string();
									material->textures[MaterialComponent::NORMALMAP].resource = wiResourceManager::Load(texdir.path().string());
									//material->SetNormalMapStrength(-3.0);
								}
							} else if (iequals(texdir.path().filename().string(), "back_normal.dds")) {
								if (m_cBlockTypes[tmpID].material[4]) {
									material				= m_scene.materials.GetComponent(m_cBlockTypes[tmpID].material[4]);
									material->textures[MaterialComponent::NORMALMAP].name	  = texdir.path().string();
									material->textures[MaterialComponent::NORMALMAP].resource = wiResourceManager::Load(texdir.path().string());
									//material->SetNormalMapStrength(-3.0);
								}
							}
						}
					}
					if (m_cBlockTypes[tmpID].material[0] == 0) {
						m_cBlockTypes[tmpID].material[0] = m_scene.Entity_CreateMaterial("");
					}

					for (uint8_t ft = 1; ft < 6; ft++) {
						if (m_cBlockTypes[tmpID].material[ft] == 0)
							m_cBlockTypes[tmpID].material[ft] = m_cBlockTypes[tmpID].material[ft - 1];
					}
				}
			}
		}
	}
	catch (...) {
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
