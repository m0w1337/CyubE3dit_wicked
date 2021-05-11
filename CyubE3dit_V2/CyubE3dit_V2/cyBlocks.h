#pragma once
#include "wiECS.h"
#include "json.hpp"
#include <sstream>
#include <string>
#include <filesystem>
#include <winreg.h>
#include <iostream>
#include <wctype.h>
#include <ctype.h>
#include <shlobj_core.h>
#include "ModelImporter.h"

using json = nlohmann::json;

class cyBlocks {

public:
	static constexpr uint8_t FACE_TOP = 0;
	static constexpr uint8_t FACE_BOTTOM = 1;
	static constexpr uint8_t FACE_LEFT = 2;
	static constexpr uint8_t FACE_RIGHT = 3;
	static constexpr uint8_t FACE_BACK	 = 4;
	static constexpr uint8_t FACE_FRONT = 5;
	static constexpr uint8_t FACE_BILLBOARD = 6;
	
	enum blocktype_t {
		BLOCKTYPE_UNKNOWN = 0,	// Unknown must stay at 0 because of array initializations
		BLOCKTYPE_SOLID,
		BLOCKTYPE_MOD,
		BLOCKTYPE_SOLID_THRESH,	 //make sure all solid blocks (completely covering the neighbour surface) are ABOVE this value, all blocks, leaving their neighbour visible will come below!!
		BLOCKTYPE_ALPHA,
		BLOCKTYPE_BILLBOARD,
		BLOCKTYPE_TORCH,
		BLOCKTYPE_VOID,
		BLOCKTYPE_FUNCMESH,
		BLOCKTYPE_DECOMESH

	};
	struct cBlock_t {
		wiECS::Entity material[6];
		std::string creator;
		std::string name;
	};
	struct mesh_t {
		blocktype_t type;
		wiECS::Entity material[2];
		wiECS::Entity mesh[2];
		std::string name;
	};
	enum blockflags_t {
		BLOCKFLAG_REGULAR	= 0,
		BLOCKFLAG_MISC0		= 2,  //unuseable
		BLOCKFLAG_MISC1		= 4,  //unuseable
		BLOCKFLAG_ANTITILE	= 8  //must be greater than 5 to prevent ovewrlap with the 6 face IDs
	};

	enum toolMeshes_t {
		TOOL_ROT = 0,
		TOOL_ORIGIN,
		TOOL_XAXIS,
		TOOL_YAXIS,
		TOOL_ZAXIS,
		TOOL_PLANE,
		TOOL_CHECK,
		TOOL_CROSS,
		TOOL_NUMTOOLS
	};

	enum foilageMeshes_t {
		FOIL_FLOWER_BLUE = 0,
		FOIL_NUM_MESHES
	};

	static uint8_t m_regBlockTypes[256];
	static uint8_t m_regBlockFlags[256][6];
	static wiECS::Entity m_regBlockMats[256][6];
	static std::unordered_map<uint32_t, cBlock_t> m_cBlockTypes;
	static std::unordered_map<uint32_t, cyBlocks::mesh_t> m_regMeshes;
	static uint8_t m_voidID;
	static uint8_t m_torchID;
	static  std::string m_regBlockNames[256];
	static wiECS::Entity m_fallbackMat;
	static std::vector<wiECS::Entity> m_treeMeshes;
	static wiECS::Entity m_toolMeshes[TOOL_NUMTOOLS];
	static bool m_loaded;
	static std::shared_ptr<wiResource> emitter_dust_material;
	static std::shared_ptr<wiResource> emitter_fire_material;
	static std::shared_ptr<wiResource> emitter_flare_material;
	static std::shared_ptr<wiResource> emitter_smoke_material;
	static std::shared_ptr<wiResource> hair_grass_material;
	static wiECS::Entity toolblock_material;
	static void LoadRegBlocks(void);
	static void loadMeshes(void);
	static void LoadCustomBlocks(void);

private:
	static wiScene::Scene& m_scene;

	struct block_t {
		uint8_t id;
		//uint8_t type;
		std::string name;
		std::string diffuse[6];
		std::string normal[6];
		std::string displace[6];
		std::string glow[6];
		//	float roughness;
		//	float reflectance;
		//	float opacity;
		//	float metalness;
	};
	enum matType_t {
		BLOCKMAT_NONE			  = 0,
		BLOCKMAT_SINGLE			  = 1,
		BLOCKMAT_TOP_REST		  = 2,
		BLOCKMAT_TOP_BOTTOM_SIDES = 3,
		BLOCKMAT_SEPERATE		  = 4,
		BLOCKMAT_FOILAGE		  = 5,
		BLOCKMAT_MESHBLOCK		  = 6
	};

	static bool GetStringRegKey(const std::wstring& key, const std::wstring& strValueName, std::wstring& strValue);
	static bool iequals(std::string str1, std::string str2);
	static void addCustomBlocksPath(std::wstring customPath);
	static void catchRegularMeshSpecs(const json::iterator& it, const size_t i, const blocktype_t blocktype); 
	static void catchRegularBlockSpecs(const json::iterator& it, const size_t i, const blocktype_t blocktype);
};
