#pragma once
#include "cyImportant.h"
#include "chunkLoader.h"

class settings {
public:
	struct worldOffset_t {
		double x;
		double y;
	};
	static constexpr uint8_t VIEWDIST_MIN = 8;
	static constexpr uint8_t VIEWDIST_MAX = 64;
	static atomic<uint32_t> viewDist;
	static cyImportant world;
	static atomic <size_t> numVisChunks;
	static string newWorld;
	static string thisWorld;
	static uint32_t camMode;
	static bool torchlights;
	static bool clipUnderground;
	static bool collision;
	static float camspeed_slow;
	static float camspeed;
	static bool sound;
	static uint32_t rendermask;
	static bool pauseChunkloader;
	
	static void save(void);
	static void load(void);

static inline cyImportant* getWorld() {
	return &world;
}

static inline uint32_t getViewDist() {
	return viewDist;
}
static inline void setViewDist(uint32_t _viewDist) {
	if (_viewDist >= VIEWDIST_MIN && _viewDist <= VIEWDIST_MAX *3)	 //leave some space at the upper limit for the brave users ;)
		viewDist = _viewDist;
}

};
