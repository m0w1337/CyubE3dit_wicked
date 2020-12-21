#include "stdafx.h"
#include "settings.h"

atomic<uint32_t> settings::viewDist = 64;//64;
cyImportant settings::world;
atomic<uint32_t> settings::numVisChunks = 0;