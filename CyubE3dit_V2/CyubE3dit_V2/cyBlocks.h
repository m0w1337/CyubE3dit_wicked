#pragma once
#include "wiECS.h";

class cyBlocks
{
public:
	uint8_t regBlockTypes[256];
	wiECS::Entity regBlockMats[256][6];
	
	void LoadRegBlocks(void);
};

