#pragma once
#include "WickedEngine.h"

class meshGen
{
	
public:
	meshGen();
	wiScene::MeshComponent* AddMesh(wiScene::Scene& scene, wiECS::Entity materialID);
	static void inline AddFaceTop(wiScene::MeshComponent* mesh, float x, float y, float z) {
		z = z + 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.push_back(XMFLOAT3(x - 0.25f, z, y - 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x + 0.25f, z, y - 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x - 0.25f, z, y + 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x + 0.25f, z, y + 0.25f));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 1));
		//uvSet1 -- vertexAtlas
		mesh->indices.push_back(start + 2);
		mesh->indices.push_back(start);
		mesh->indices.push_back(start + 1);

		mesh->indices.push_back(start + 2);
		mesh->indices.push_back(start + 1);
		mesh->indices.push_back(start + 3);
		mesh->vertex_normals.push_back(XMFLOAT3(0, 1, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(0, 1, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(0, 1, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(0, 1, 0));
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size();
	}

	static void inline AddFaceBottom(wiScene::MeshComponent* mesh, float x, float y, float z) {
		z = z - 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.push_back(XMFLOAT3(x - 0.25f, z, y - 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x + 0.25f, z, y - 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x - 0.25f, z, y + 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x + 0.25f, z, y + 0.25f));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 1));
		//uvSet1 -- vertexAtlas
		mesh->indices.push_back(start + 1);
		mesh->indices.push_back(start);
		mesh->indices.push_back(start + 2);

		mesh->indices.push_back(start + 3);
		mesh->indices.push_back(start + 1);
		mesh->indices.push_back(start + 2);
		mesh->vertex_normals.push_back(XMFLOAT3(0, -1, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(0, -1, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(0, -1, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(0, -1, 0));
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size();
	}

	static void inline AddFaceRight(wiScene::MeshComponent* mesh, float x, float y, float z) {
		x = x + 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.push_back(XMFLOAT3(x, z - 0.25f, y - 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x, z + 0.25f, y - 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x, z - 0.25f, y + 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x, z + 0.25f, y + 0.25f));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 1));
		//uvSet1 -- vertexAtlas
		mesh->indices.push_back(start + 1);
		mesh->indices.push_back(start);
		mesh->indices.push_back(start + 2);

		mesh->indices.push_back(start + 3);
		mesh->indices.push_back(start + 1);
		mesh->indices.push_back(start + 2);
		mesh->vertex_normals.push_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(1, 0, 0));
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size();
	}

	static void inline AddFaceLeft(wiScene::MeshComponent* mesh, float x, float y, float z) {
		x = x - 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.push_back(XMFLOAT3(x, z - 0.25f, y - 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x, z + 0.25f, y - 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x, z - 0.25f, y + 0.25f));
		mesh->vertex_positions.push_back(XMFLOAT3(x, z + 0.25f, y + 0.25f));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 1));
		//uvSet1 -- vertexAtlas
		mesh->indices.push_back(start + 2);
		mesh->indices.push_back(start);
		mesh->indices.push_back(start + 1);

		mesh->indices.push_back(start + 2);
		mesh->indices.push_back(start + 1);
		mesh->indices.push_back(start + 3);
		mesh->vertex_normals.push_back(XMFLOAT3(-1, 0, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(-1, 0, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(-1, 0, 0));
		mesh->vertex_normals.push_back(XMFLOAT3(-1, 0, 0));
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size();
	}

	static void inline AddFaceFront(wiScene::MeshComponent* mesh, float x, float y, float z) {
		y = y + 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.push_back(XMFLOAT3(x - 0.25f, z - 0.25f, y));
		mesh->vertex_positions.push_back(XMFLOAT3(x - 0.25f, z + 0.25f, y));
		mesh->vertex_positions.push_back(XMFLOAT3(x + 0.25f, z - 0.25f, y));
		mesh->vertex_positions.push_back(XMFLOAT3(x + 0.25f, z + 0.25f, y));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 1));
		//uvSet1 -- vertexAtlas
		mesh->indices.push_back(start + 2);
		mesh->indices.push_back(start);
		mesh->indices.push_back(start + 1);

		mesh->indices.push_back(start + 2);
		mesh->indices.push_back(start + 1);
		mesh->indices.push_back(start + 3);
		mesh->vertex_normals.push_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.push_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.push_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.push_back(XMFLOAT3(0, 0, 1));
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size();
	}

	static void inline AddFaceBack(wiScene::MeshComponent* mesh, float x, float y, float z) {
		y = y - 0.25f;
		uint32_t start = (uint32_t) mesh->vertex_positions.size();
		mesh->vertex_positions.push_back(XMFLOAT3(x - 0.25f, z - 0.25f, y));
		mesh->vertex_positions.push_back(XMFLOAT3(x - 0.25f, z + 0.25f, y));
		mesh->vertex_positions.push_back(XMFLOAT3(x + 0.25f, z - 0.25f, y));
		mesh->vertex_positions.push_back(XMFLOAT3(x + 0.25f, z + 0.25f, y));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.push_back(XMFLOAT2(1, 1));
		//uvSet1 -- vertexAtlas
		mesh->indices.push_back(start + 1);
		mesh->indices.push_back(start);
		mesh->indices.push_back(start + 2);

		mesh->indices.push_back(start + 3);
		mesh->indices.push_back(start + 1);
		mesh->indices.push_back(start + 2);
		mesh->vertex_normals.push_back(XMFLOAT3(0, 0, -1));
		mesh->vertex_normals.push_back(XMFLOAT3(0, 0, -1));
		mesh->vertex_normals.push_back(XMFLOAT3(0, 0, -1));
		mesh->vertex_normals.push_back(XMFLOAT3(0, 0, -1));
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size();
	}

};

