#pragma once
#include "WickedEngine.h"
#include "SimplexNoise.h"
#define ANTITILE_FACT 8
class meshGen {

public:
	static SimplexNoise m_noise;
	meshGen();
	static wiScene::MeshComponent* AddMesh(wiScene::Scene& scene, wiECS::Entity _material, wiECS::Entity* _newEntity);

	static void inline AddBillboard(wiScene::MeshComponent* mesh, float x, float y, float z) {
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + 0.25f, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + 0.25f, y + 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y + 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x-0.25f, z - 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x-0.25f, z + 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x+0.25f, z + 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x+0.25f, z - 0.25f, y));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(100);
		mesh->vertex_windweights.emplace_back(100);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(100);
		mesh->vertex_windweights.emplace_back(100);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_colors.emplace_back(0xFFFFFF);
		mesh->vertex_colors.emplace_back(0xFFFFFF);
		mesh->vertex_colors.emplace_back(0xFFFFFF);
		mesh->vertex_colors.emplace_back(0xFFFFFF);
		mesh->vertex_colors.emplace_back(0xFFFFFF);
		mesh->vertex_colors.emplace_back(0xFFFFFF);
		mesh->vertex_colors.emplace_back(0xFFFFFF);
		mesh->vertex_colors.emplace_back(0xFFFFFF);
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));

		mesh->indices.emplace_back(start + 2);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start);

		mesh->indices.emplace_back(start + 0);
		mesh->indices.emplace_back(start + 3);
		mesh->indices.emplace_back(start + 2);
		start += 4;
		mesh->indices.emplace_back(start + 2);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start);

		mesh->indices.emplace_back(start + 0);
		mesh->indices.emplace_back(start + 3);
		mesh->indices.emplace_back(start + 2);
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
	}

	static void inline AddFaceTop(wiScene::MeshComponent* mesh, float x, float y, float z, float relx, float rely, bool antitile = false) {
		z			   = z + 0.25f;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z, y + 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z, y + 0.25f));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		
		uint32_t noisev = (roundf((m_noise.fractal(5, (x + relx) / 32, (y + rely) / 32, z / 32) + 1) * 64) + 128);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		
		if (antitile == false) {

			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		} else {
			float uvx = x / (ANTITILE_FACT / 2);
			uvx		  = (uvx - floorf(uvx));
			float uvy = y / (ANTITILE_FACT / 2);
			uvy		  =  (uvy - floorf(uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvy, uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvy, 1.0f / ANTITILE_FACT + uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvy, uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvy, 1.0f / ANTITILE_FACT + uvx));
			
		}
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

	static void inline AddFaceBottom(wiScene::MeshComponent* mesh, float x, float y, float z, float relx, float rely, bool antitile = false) {
		z			   = z - 0.25f;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z, y + 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z, y + 0.25f));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		uint32_t noisev = (roundf((m_noise.fractal(5, (x + relx) / 32, (y + rely) / 32, z / 32) + 1) * 64) + 128);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		if (antitile == false) {
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
			
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		} else {
			float uvx = x / (ANTITILE_FACT / 2);
			uvx		  = uvx - floorf(uvx);
			float uvy = y / (ANTITILE_FACT / 2);
			uvy		  = uvy - floorf(uvy);
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvy, uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvy, 1.0f / ANTITILE_FACT + uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvy, uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvy, 1.0f / ANTITILE_FACT + uvx));
		}
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

	static void inline AddFaceRight(wiScene::MeshComponent* mesh, float x, float y, float z, float relx, float rely, bool antitile = false) {
		x			   = x + 0.25f;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + 0.25f, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + 0.25f, y + 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y + 0.25f));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		uint32_t noisev = (roundf((m_noise.fractal(5, (x + relx) / 32, (y + rely) / 32, z / 32) + 1) * 64) + 128);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);

		if (antitile == false) {
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
			
		} else {
			float uvx = y / (ANTITILE_FACT / 2);
			uvx		  = uvx - floorf(uvx);
			float uvy = z / (ANTITILE_FACT / 2);
			uvy		  = uvy - floorf(uvy);
			uvy		  = 1 - uvy;
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, 1.0f / ANTITILE_FACT + uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvx, uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvx, 1.0f / ANTITILE_FACT + uvy));
			
		}

		//uvSet1 -- vertexAtlas
		mesh->indices.emplace_back(start + 2);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start);

		mesh->indices.emplace_back(start + 0);
		mesh->indices.emplace_back(start + 3);
		mesh->indices.emplace_back(start + 2);
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
	}

	static void inline AddFaceLeft(wiScene::MeshComponent* mesh, float x, float y, float z, float relx, float rely, bool antitile = false) {
		x			   = x - 0.25f;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + 0.25f, y - 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + 0.25f, y + 0.25f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y + 0.25f));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		uint32_t noisev = (roundf((m_noise.fractal(5, (x + relx) / 32, (y + rely) / 32, z / 32) + 1) * 64) + 128);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);

		if (antitile == false) {
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
			
			
		} else {
			float uvx = y / (ANTITILE_FACT / 2);
			uvx		  = uvx - floorf(uvx);
			uvx		  = 1 - uvx;
			float uvy = z / (ANTITILE_FACT / 2);
			uvy		  = uvy - floorf(uvy);
			uvy		  = 1 - uvy;
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvx, 1.0f / ANTITILE_FACT + uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvx, uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, 1.0f / ANTITILE_FACT + uvy));
			
			
		}
		//uvSet1 -- vertexAtlas
		mesh->indices.emplace_back(start);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start + 2);

		mesh->indices.emplace_back(start + 2);
		mesh->indices.emplace_back(start + 3);
		mesh->indices.emplace_back(start);
		mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
		mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
	}

	static void inline AddFaceFront(wiScene::MeshComponent* mesh, float x, float y, float z, float relx, float rely, bool antitile = false) {
		y			   = y + 0.25f;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z - 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z + 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z + 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z - 0.25f, y));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		uint32_t noisev = (roundf((m_noise.fractal(5, (x + relx) / 32, (y + rely) / 32, z / 32) + 1) * 64) + 128);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		if (antitile == false) {
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
			
			
		} else {
			float uvx = x / (ANTITILE_FACT / 2);
			uvx		  = uvx - floorf(uvx);
			uvx		  = 1 - uvx;
			float uvy = z / (ANTITILE_FACT / 2);
			uvy		  = uvy - floorf(uvy);
			uvy		  = 1 - uvy;
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvx, 1.0f / ANTITILE_FACT + uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvx, uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, 1.0f / ANTITILE_FACT + uvy));
		}
		//uvSet1 -- vertexAtlas
		mesh->indices.emplace_back(start);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start+2);

		mesh->indices.emplace_back(start+2);
		mesh->indices.emplace_back(start + 3);
		mesh->indices.emplace_back(start);
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
	}

	static void inline AddFaceBack(wiScene::MeshComponent* mesh, float x, float y, float z, float relx, float rely, bool antitile = false) {
		y			   = y - 0.25f;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z - 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - 0.25f, z + 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z + 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + 0.25f, z - 0.25f, y));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		uint32_t noisev = (roundf((m_noise.fractal(5, (x + relx) / 32, (y + rely) / 32, z / 32) + 1) * 64) + 128);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		mesh->vertex_colors.emplace_back(0xFF0000 | (noisev << 16) | (noisev << 8) | noisev);
		
		if (antitile == false) {
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
		} else {
			float uvx = x / (ANTITILE_FACT / 2);
			uvx		  = uvx - floorf(uvx);
			float uvy = z / (ANTITILE_FACT / 2);
			uvy		  = uvy - floorf(uvy);
			uvy		  = 1 - uvy;
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, 1.0f / ANTITILE_FACT + uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvx, uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvx, 1.0f / ANTITILE_FACT + uvy));
		}
		//uvSet1 -- vertexAtlas
		mesh->indices.emplace_back(start+2);
		mesh->indices.emplace_back(start+1);
		mesh->indices.emplace_back(start );

		mesh->indices.emplace_back(start );
		mesh->indices.emplace_back(start + 3);
		mesh->indices.emplace_back(start + 2);
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
	}
	static void inline newMaterial(wiScene::MeshComponent* mesh, wiECS::Entity materialID) {
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
		mesh->subsets.push_back(wiScene::MeshComponent::MeshSubset());
		mesh->subsets.back().indexOffset = (uint32_t)mesh->indices.size();
		mesh->subsets.back().indexCount	 = 0;
		mesh->subsets.back().materialID	 = materialID;
	}
};
