#pragma once
#include "wiECS.h";
#include "json.hpp"
#include <sstream>
#include <string>

using json = nlohmann::json;

class cyBlocks {

public:
	enum blocktype_t {
		BLOCKTYPE_UNKNOWN = 0,	// Unknown must stay at 0 because of array initializations
		BLOCKTYPE_SOLID,
		BLOCKTYPE_MOD,
		BLOCKTYPE_SOLID_THRESH,	 //make sure all solid blocks (completely covering the neighbour surface) are ABOVE this value, all blocks, leaving their neighbour visible will come below!!
		BLOCKTYPE_ALPHA,
		BLOCKTYPE_VOID,
		BLOCKTYPE_FUNCMESH,
		BLOCKTYPE_DECOMESH

	};
	static uint8_t m_regBlockTypes[256];
	static wiECS::Entity m_regBlockMats[256][6];
	static uint8_t m_voidID;
	std::string m_regBlockNames[256];
	wiScene::Scene& m_scene;
	cyBlocks(wiScene::Scene& scene);

	void LoadRegBlocks(void);

private:
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

	void catchRegularBlockSpecs(const json::iterator& it, const size_t i, const blocktype_t blocktype);
};
