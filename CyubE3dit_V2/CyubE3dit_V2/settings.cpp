#include "stdafx.h"
#include "settings.h"

atomic<uint32_t> settings::viewDist = 32;  //64;
cyImportant settings::world;
atomic<size_t> settings::numVisChunks = 0;
string settings::newWorld			  = "";
string settings::thisWorld			  = "";
bool settings::torchlights			  = false;
bool settings::clipUnderground		  = false;
bool settings::collision			  = true;
float settings::camspeed				  = 4.0f;
bool settings::pauseChunkloader		  = false;
uint32_t settings::camMode				  = 0;