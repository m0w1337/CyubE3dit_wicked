#pragma once
#include "cyImportant.h"
#include "chunkLoader.h"

class settings {
public:
	static atomic<uint32_t> viewDist;
	static cyImportant world;


static inline cyImportant* getWorld() {
	return &world;
}

static inline uint32_t getViewDist() {
	return viewDist;
}
static inline uint32_t setViewDist(uint32_t _viewDist) {
	viewDist = _viewDist;
}

};
