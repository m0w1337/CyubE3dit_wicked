#include "stdafx.h"
#include "settings.h"

atomic<uint32_t> settings::viewDist = 32;  //64;
cyImportant settings::world;
atomic<size_t> settings::numVisChunks = 0;
string settings::newWorld			  = "";
string settings::thisWorld			  = "";
bool settings::torchlights			  = false;
bool settings::clipUnderground		  = false;
float settings::camspeed			  = 2.0f;
bool settings::pauseChunkloader		  = false;
bool settings::sound				  = false;
uint32_t settings::rendermask		  = 0xFFFFFFFF;
uint32_t settings::camMode			  = 0;

void settings::save(void) {
	uint8_t* serial;
	uint8_t data[15];
	uint8_t i = 0;
	data[i++] = torchlights;
	data[i++] = clipUnderground;
	serial	  = reinterpret_cast<uint8_t*>(&camspeed);
	data[i++] = *serial;
	data[i++] = *(serial + 1);
	data[i++] = *(serial + 2);
	data[i++] = *(serial + 3);
	data[i++] = sound;
	serial	  = reinterpret_cast<uint8_t*>(&rendermask);
	data[i++] = *serial;
	data[i++] = *(serial + 1);
	data[i++] = *(serial + 2);
	data[i++] = *(serial + 3);
	serial	  = reinterpret_cast<uint8_t*>(&camMode);
	data[i++] = *serial;
	data[i++] = *(serial + 1);
	data[i++] = *(serial + 2);
	data[i++] = *(serial + 3);
	wiHelper::FileWrite("prefs.bin", data, 15);
}

void settings::load(void) {
	std::vector<uint8_t> data;
	uint8_t i = 0;
	if (wiHelper::FileRead("prefs.bin", data))
	{
		if (data.size() == 15) {
			torchlights = data[i++];
			clipUnderground = data[i++];
		}
	}
}
