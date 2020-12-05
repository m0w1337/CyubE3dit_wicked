#pragma once
#include "cyChunk.h"
#include "meshGen.h"
#include "wiECS.h"

class chunkLoader {
public:
	struct face_t {
		wiECS::Entity material;
		uint8_t face;
		float x;
		float y;
		float z;

		bool operator<(const face_t& a) const {	 //Make it sortable by material
			return material < a.material;
		}
	};

	static void RenderChunk(const cyChunk& chunk, const cyChunk& northChunk, const cyChunk& eastChunk, const cyChunk& southChunk, const cyChunk& westChunk);
};