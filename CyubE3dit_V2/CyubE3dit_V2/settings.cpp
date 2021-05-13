#include "stdafx.h"
#include "settings.h"

atomic<uint32_t> settings::viewDist = 32;  //64;
cyImportant settings::world;
atomic<size_t> settings::numVisChunks = 0;
string settings::newWorld			  = "";
string settings::thisWorld			  = "";
bool settings::torchlights			  = false;
bool settings::clipUnderground		  = true;
float settings::camspeed			  = 2.0f;
bool settings::pauseChunkloader		  = false;
bool settings::sound				  = false;
uint32_t settings::rendermask		  = LAYER_CHUNKMESH | LAYER_GIZMO | LAYER_SCHEMATIC | LAYER_TORCH | LAYER_FOILAGE | LAYER_TREE | LAYER_MESH;
uint32_t settings::camMode			  = 0;
float settings::musicVol			  = 1.;
float settings::effectVol			  = 0.5;
bool settings::volClouds			  = false;
bool settings::tempAA			  = false;
bool settings::treeDeletion			  = false;
uint32_t settings::pickType			  = 0x00000000;

void settings::save(void) {
	uint8_t* serial;
	uint8_t data[prefsLen];
	uint8_t i = 0;
	data[i++] = torchlights;
	data[i++] = 0xAA;
	data[i++] = clipUnderground;
	data[i++] = 0xAA;
	serial	  = reinterpret_cast<uint8_t*>(&camspeed);
	data[i++] = *serial;
	data[i++] = *(serial + 1);
	data[i++] = *(serial + 2);
	data[i++] = *(serial + 3);
	data[i++] = 0xAA;
	data[i++] = sound;
	data[i++] = 0xAA;
	serial	  = reinterpret_cast<uint8_t*>(&rendermask);
	data[i++] = *serial;
	data[i++] = *(serial + 1);
	data[i++] = *(serial + 2);
	data[i++] = *(serial + 3);
	data[i++] = 0xAA;
	serial	  = reinterpret_cast<uint8_t*>(&camMode);
	data[i++] = *serial;
	data[i++] = *(serial + 1);
	data[i++] = *(serial + 2);
	data[i++] = *(serial + 3);
	data[i++] = 0xAA;
	serial	  = reinterpret_cast<uint8_t*>(&musicVol);
	for (size_t ii = 0; ii < sizeof(float); ii++) {
		data[i++] = *(serial + ii);
	}
	data[i++] = 0xAA;
	serial	  = reinterpret_cast<uint8_t*>(&effectVol);
	for (size_t ii = 0; ii < sizeof(float); ii++) {
		data[i++] = *(serial + ii);
	}
	data[i++] = 0xAA;
	data[i++] = tempAA;
	data[i++] = 0xAA;
	data[i++] = volClouds;
	data[i++] = 0xAA;
	data[i++] = treeDeletion;
	data[i++] = 0xAA;
	wiHelper::FileWrite("prefs.bin", data, i);
}

void settings::load(void) {
	std::vector<uint8_t> data;
	uint8_t i = 0;
	if (wiHelper::FileRead("prefs.bin", data))
	{
		if (data.size() == prefsLen) {
		torchlights = data[i++];
		i++;
		clipUnderground = data[i++];
		i++;
		camspeed		= *(reinterpret_cast<float*>(&data[i]));
		if (camspeed > 10 || camspeed < 0.1)
			camspeed = 2;
		i += sizeof(float);
		i++;
		sound	   = data[i++];
		i++;
		rendermask = *(reinterpret_cast<uint32_t*>(&data[i]));
		i += sizeof(uint32_t);
		i++;
		camMode = *(reinterpret_cast<uint32_t*>(&data[i]));
		if (camMode > 1)
			camMode = 0;
		i += sizeof(uint32_t);
		i++;
		musicVol = *(reinterpret_cast<float*>(&data[i]));
		if (musicVol > 1. || musicVol < 0.)
			musicVol = 1.;
		i += sizeof(float);
		i++;
		effectVol = *(reinterpret_cast<float*>(&data[i]));
		if (effectVol > 1. || effectVol < 0.)
			effectVol = 0.5;
		i += sizeof(float);
		i++;
		tempAA = data[i++];
		i++;
		volClouds = data[i++];
		i++;
		treeDeletion = data[i++];
		i++;
		rendermask |= (LAYER_CHUNKMESH | LAYER_SCHEMATIC | LAYER_GIZMO);	//at least enable the most basic layers after restart to prevent confusion
		}
	}
}
