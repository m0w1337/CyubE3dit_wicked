#pragma once
#include "cyImportant.h"
#include "chunkLoader.h"

class settings {
public:
	static constexpr uint8_t VIEWDIST_MIN = 8;
	static constexpr uint8_t VIEWDIST_MAX = 64;
	static atomic<uint32_t> viewDist;
	static cyImportant world;
	static atomic <size_t> numVisChunks;
	static string newWorld;
	static string thisWorld;
	static bool torchlights;
	static bool clipUnderground;
	static bool collision;
	static float camspeed_slow;
	static float camspeed;
	static bool pauseChunkloader;

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
