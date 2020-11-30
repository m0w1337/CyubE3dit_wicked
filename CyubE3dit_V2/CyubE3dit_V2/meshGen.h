#pragma once
#include "WickedEngine.h"

class meshGen
{
	
public:
	meshGen();
	static wiScene::MeshComponent* AddMesh(wiScene::Scene& scene, wiECS::Entity materialID);
	static void inline AddFaceTop(wiScene::MeshComponent* mesh, float x, float y, float z) {
		z = z + 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z, y + 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z, y + 0.25f));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
		//uvSet1 -- vertexAtlas
		mesh->indices.emplace_back(start + 2);
		mesh->indices.emplace_back(start);
		mesh->indices.emplace_back(start + 1);

		mesh->indices.emplace_back(start + 2);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start + 3);
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 1, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 1, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 1, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 1, 0));
	}

	static void inline AddFaceBottom(wiScene::MeshComponent* mesh, float x, float y, float z) {
		z = z - 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z, y + 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z, y + 0.25f));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
		//uvSet1 -- vertexAtlas
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start);
		mesh->indices.emplace_back(start + 2);

		mesh->indices.emplace_back(start + 3);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start + 2);
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, -1, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, -1, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, -1, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, -1, 0));
	}

	static void inline AddFaceRight(wiScene::MeshComponent* mesh, float x, float y, float z) {
		x = x + 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + 0.25f, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y + 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + 0.25f, y + 0.25f));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
		
		
		//uvSet1 -- vertexAtlas
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start);
		mesh->indices.emplace_back(start + 2);

		mesh->indices.emplace_back(start + 3);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start + 2);
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
	}

	static void inline AddFaceLeft(wiScene::MeshComponent* mesh, float x, float y, float z) {
		x = x - 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + 0.25f, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y + 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + 0.25f, y + 0.25f));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
		//uvSet1 -- vertexAtlas
		mesh->indices.emplace_back(start + 2);
		mesh->indices.emplace_back(start);
		mesh->indices.emplace_back(start + 1);

		mesh->indices.emplace_back(start + 2);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start + 3);
		mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
	}

	static void inline AddFaceFront(wiScene::MeshComponent* mesh, float x, float y, float z) {
		y = y + 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z - 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z + 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z - 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z + 0.25f, y));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
		//uvSet1 -- vertexAtlas
		mesh->indices.emplace_back(start + 2);
		mesh->indices.emplace_back(start);
		mesh->indices.emplace_back(start + 1);

		mesh->indices.emplace_back(start + 2);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start + 3);
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
	}

	static void inline AddFaceBack(wiScene::MeshComponent* mesh, float x, float y, float z) {
		y = y - 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z - 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z + 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z - 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z + 0.25f, y));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
		//uvSet1 -- vertexAtlas
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start);
		mesh->indices.emplace_back(start + 2);

		mesh->indices.emplace_back(start + 3);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start + 2);
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
	}
	static void inline newMaterial(wiScene::MeshComponent* mesh, wiECS::Entity materialID) {
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
		mesh->subsets.push_back(wiScene::MeshComponent::MeshSubset());
		mesh->subsets.back().indexOffset = (uint32_t) mesh->indices.size();
		mesh->subsets.back().indexCount	 = 0;
		mesh->subsets.back().materialID	 = materialID;
	}

};

