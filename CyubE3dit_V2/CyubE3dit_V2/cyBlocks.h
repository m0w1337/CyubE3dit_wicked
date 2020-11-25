#pragma once
#include "wiECS.h";

class cyBlocks
{
public:
	enum {
		BLOCKTYPE_BLOCK = 0
	};
	uint8_t m_regBlockTypes[256];
	wiECS::Entity m_regBlockMats[256][6];
	
	void LoadRegBlocks(void);
};

