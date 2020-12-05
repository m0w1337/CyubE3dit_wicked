#include "stdafx.h"
#include "chunkLoader.h"

using namespace std;
using namespace wiScene;


void chunkLoader::RenderChunk(const cyChunk& chunk, const cyChunk& northChunk, const cyChunk& eastChunk, const cyChunk& southChunk, const cyChunk& westChunk) {

	face_t tmpface;
	vector<face_t> faces;
	for (uint_fast8_t x = 0; x < 32; x++) {
		for (uint_fast8_t y = 0; y < 32; y++) {
			for (uint_fast16_t z = 0; z < 800; z++) {
				uint8_t blocktype = (uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * z);
				if (cyBlocks::m_regBlockTypes[blocktype] < cyBlocks::BLOCKTYPE_SOLID_THRESH) {
					uint8_t upperB, lowerB, leftB, rightB, frontB, backB;
					if (z < 799)
						upperB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * (z + 1))];
					else
						upperB = cyBlocks::BLOCKTYPE_VOID;

					if (z > 1)
						lowerB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * y + 32 * 32 * (z - 1))];
					else
						lowerB = cyBlocks::BLOCKTYPE_VOID;

					if (y < 31)
						frontB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * (y + 1) + 32 * 32 * z)];
					else
						frontB = cyBlocks::m_regBlockTypes[(uint8_t) * (northChunk.m_chunkdata + 4 + x + 32 * 32 * z)];

					if (y > 1)
						backB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 4 + x + 32 * (y - 1) + 32 * 32 * z)];
					else
						backB = cyBlocks::m_regBlockTypes[(uint8_t) * (southChunk.m_chunkdata + 4 + x + 32*31 + 32 * 32 * z)];

					if (x < 31)
						rightB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 5 + x + 32 * y + 32 * 32 * z)];
					else
						rightB = cyBlocks::m_regBlockTypes[(uint8_t) * (eastChunk.m_chunkdata + 4 + 32 * y + 32 * 32 * z)];

					if (x > 0)
						leftB = cyBlocks::m_regBlockTypes[(uint8_t) * (chunk.m_chunkdata + 3 + x + 32 * y + 32 * 32 * z)];
					else
						leftB = cyBlocks::m_regBlockTypes[(uint8_t) * (westChunk.m_chunkdata + 4 + 31 + 32 * y + 32 * 32 * z)];

					tmpface.x = x / 2.0f;
					tmpface.y = y / 2.0f;
					tmpface.z = z / 2.0f;
					if (upperB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.face	 = 0;
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][0];
						faces.emplace_back(tmpface);
					}
					if (lowerB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][1];
						tmpface.face	 = 1;
						faces.emplace_back(tmpface);
					}
					if (leftB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][2];
						tmpface.face	 = 2;
						faces.emplace_back(tmpface);
					}
					if (rightB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][3];
						tmpface.face	 = 3;
						faces.emplace_back(tmpface);
					}
					if (frontB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][4];
						tmpface.face	 = 4;
						faces.emplace_back(tmpface);
					}
					if (backB > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
						tmpface.material = cyBlocks::m_regBlockMats[blocktype][5];
						tmpface.face	 = 5;
						faces.emplace_back(tmpface);
					}
				}
			}
		}
	}
	sort(faces.begin(), faces.end());
	MeshComponent* mesh;
	mesh				  = meshGen::AddMesh(wiScene::GetScene(), faces[0].material);
	wiECS::Entity currMat = faces[0].material;
	for (unsigned i = 0; i < faces.size(); ++i)
	{
		if (faces[i].material != currMat) {
			if (wiScene::GetScene().materials.GetComponent(faces[i].material) != nullptr) {
				meshGen::newMaterial(mesh, faces[i].material);
			}
			currMat = faces[i].material;
		}
		switch (faces[i].face) {
			case 0:
				meshGen::AddFaceTop(mesh, faces[i].x, faces[i].y, faces[i].z, false);
				break;
			case 1:
				meshGen::AddFaceBottom(mesh, faces[i].x, faces[i].y, faces[i].z, false);
				break;
			case 2:
				meshGen::AddFaceLeft(mesh, faces[i].x, faces[i].y, faces[i].z, false);
				break;
			case 3:
				meshGen::AddFaceRight(mesh, faces[i].x, faces[i].y, faces[i].z, false);
				break;
			case 4:
				meshGen::AddFaceFront(mesh, faces[i].x, faces[i].y, faces[i].z, false);
				break;
			case 5:
				meshGen::AddFaceBack(mesh, faces[i].x, faces[i].y, faces[i].z, false);
				break;
		}
	}
	mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
	for (auto& pos : mesh->vertex_positions)
	{
		pos.x += 17;
		pos.y += 17;
		pos.z += 17;
	}
	mesh->CreateRenderData();
}