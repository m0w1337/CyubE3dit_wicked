#include "stdafx.h"
#include "cyBlocks.h"

using namespace std;
using namespace wiECS;
using namespace wiScene;

uint8_t cyBlocks::m_regBlockTypes[256]	= {0};
Entity cyBlocks::m_regBlockMats[256][6] = {{0}};
uint8_t cyBlocks::m_voidID				= 255;

cyBlocks::cyBlocks(wiScene::Scene& scene) :
  m_scene(scene) {
}

void cyBlocks::LoadRegBlocks(void) {
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
	m_regBlockTypes[id] = (uint8_t)blocktype;
	if (blocktype == BLOCKTYPE_VOID) {
		m_voidID = id;
	}
	try {
		materialType = (matType_t)it.value().at(i).at("textype");
	}
	catch (...) {
		materialType = BLOCKMAT_SINGLE;
	}
	try {
		m_regBlockNames[id] = it.value().at(i).at("text");
	}
	catch (...) {
		m_regBlockNames[id] = "";
	}
	wiScene::MaterialComponent* material;

	for (uint8_t ft = 0; ft < 6; ft++) {
		try {
			tex					   = it.value().at(i).at("texture" + std::to_string(ft));
			m_regBlockMats[id][ft] = m_scene.Entity_CreateMaterial("bMat" + std::to_string(id) + std::to_string(ft));
			material			   = m_scene.materials.GetComponent(m_regBlockMats[id][ft]);
			material->baseColorMapName = "images/" + tex;
			material->SetReflectance(0.001);
			material->SetMetalness(0.001);
			material->SetEmissiveStrength(0);
			material->SetRoughness(0.9);
			material->SetNormalMapStrength(.1);
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
					tex = it.value().at(i).at("occlusion" + std::to_string(ft));
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

	for (uint8_t ft = 1; ft < 6; ft++) {
		if (m_regBlockMats[id][ft] == 0)
			m_regBlockMats[id][ft] = m_regBlockMats[id][ft - 1];
	}
}
