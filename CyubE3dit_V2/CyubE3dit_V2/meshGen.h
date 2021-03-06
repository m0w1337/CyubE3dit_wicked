#pragma once
#include "WickedEngine.h"
#include "SimplexNoise.h"
#include "cyRender.h"
#define ANTITILE_FACT 8



class meshGen {

public:
	static wiECS::Entity toolBlockFaces[6];
	static XMFLOAT3 toolblockSize;
	static SimplexNoise m_noise;
	meshGen();
	static wiScene::MeshComponent* AddMesh(wiScene::Scene& scene, std::string _chunkID, wiECS::Entity _material, wiECS::Entity* _newEntity);
	static void ResizeToolBlock(float _x, float _y, float _z, bool _antitile = false);
	static wiECS::Entity AddToolBoxFace(wiECS::Entity _material, uint8_t _face);
	/*static wiECS::Entity AddToolBoxFront(wiECS::Entity _material);
	static wiECS::Entity AddToolBoxBack(wiECS::Entity _material);
	static wiECS::Entity AddToolBoxLeft(wiECS::Entity _material);
	static wiECS::Entity AddToolBoxRight(wiECS::Entity _material);
	static wiECS::Entity AddToolBoxBottom(wiECS::Entity _material);*/

	static void inline AddBillboard(wiScene::MeshComponent* mesh, float x, float y, float z) {
		float noisev   = m_noise.fractal(5, x * 0.1f, y * 0.1f, z * 0.1f);
		uint32_t color  = 255 | (uint32_t)(255 - 32 + noisev * 32) << 8 | (uint32_t)(255 - 96 + noisev * 96) << 16 | 0xFF000000;
		noisev   = 0.5 + 0.3 * noisev;
		float noiseh   = 0.5 + 0.35 * m_noise.noise(x * 0.5f, y * 0.5f, z * 0.5f);
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y - noisev));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + noiseh, y - noisev));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + noiseh, y + noisev));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - 0.25f, y + noisev));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - noisev, z - 0.25f, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - noisev, z + noiseh, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + noisev, z + noiseh, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + noisev, z - 0.25f, y));
		noisev *= 1.4f;
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - noisev * 0.7f, z - 0.25f, y - noisev * 0.7f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - noisev * 0.7f, z + noiseh, y - noisev * 0.7f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + noisev * 0.7f, z + noiseh, y + noisev * 0.7f));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + noisev * 0.7f, z - 0.25f, y + noisev * 0.7f));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(130);
		mesh->vertex_windweights.emplace_back(130);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(130);
		mesh->vertex_windweights.emplace_back(130);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(130);
		mesh->vertex_windweights.emplace_back(130);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));

		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));

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
		start += 4;
		mesh->indices.emplace_back(start + 2);
		mesh->indices.emplace_back(start + 1);
		mesh->indices.emplace_back(start);

		mesh->indices.emplace_back(start + 0);
		mesh->indices.emplace_back(start + 3);
		mesh->indices.emplace_back(start + 2);
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));
		mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0.5f, -0.5f));

	}

	static void inline AddFaceTop(wiScene::MeshComponent* mesh, float x, float y, float z, bool antitile = false, float len = 0) {
		float offs	   = 0.25f;
		z			   = z + offs;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - offs, z, y - offs));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + offs, z, y - offs));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - offs, z, y + offs + len));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + offs, z, y + offs + len));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		uint32_t color = 0xFFFFFFFF;
		len += 0.5;
		if (antitile == false) {
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1*len*2, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * len * 2, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		} else {
			float noisev = m_noise.fractal(5, x * 0.03f, y * 0.03f, z * 0.03f);
			color		 = 255 | (uint32_t)(255 - 64 + noisev * 64) << 8 | (uint32_t)(255 - 64 + noisev * 64) << 16 | 0xFF000000;
			float uvx = x / (ANTITILE_FACT / 2);
			uvx		  = (uvx - floorf(uvx));
			float uvy = y / (ANTITILE_FACT / 2);
			uvy		  =  (uvy - floorf(uvy));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvy, uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvy, 1.0f / ANTITILE_FACT + uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2((1.0f * len * 2) / ANTITILE_FACT + uvy, uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2((1.0f * len * 2) / ANTITILE_FACT + uvy, 1.0f / ANTITILE_FACT + uvx));
			
		}
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
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

	static void inline AddFaceBottom(wiScene::MeshComponent* mesh, float x, float y, float z, bool antitile = false, float len = 0) {
		float offs	   = 0.25f;
		z			   = z - offs;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - offs, z, y - offs));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + offs, z, y - offs));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - offs, z, y + offs + len));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + offs, z, y + offs + len));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		uint32_t color = 0xFFFFFFFF;
		len += 0.5;
		if (antitile == false) {
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * len * 2, 1));
			
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * len * 2, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1 ));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		} else {
			float noisev = m_noise.fractal(5, x * 0.03f, y * 0.03f, z * 0.03f);
			color		 = 255 | (uint32_t)(255 - 64 + noisev * 64) << 8 | (uint32_t)(255 - 64 + noisev * 64) << 16 | 0xFF000000;
			float uvx = x / (ANTITILE_FACT / 2);
			uvx		  = uvx - floorf(uvx);
			float uvy = y / (ANTITILE_FACT / 2);
			uvy		  = uvy - floorf(uvy);
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvy * len * 2, uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvy * len * 2, 1.0f / ANTITILE_FACT + uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvy, uvx));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1.0f / ANTITILE_FACT + uvy, 1.0f / ANTITILE_FACT + uvx));
		}
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
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

	static void inline AddFaceRight(wiScene::MeshComponent* mesh, float x, float y, float z, bool antitile = false) {
		float offs	   = 0.25f;
		x			   = x + offs;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - offs, y - offs));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + offs, y - offs));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + offs, y + offs));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - offs, y + offs));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		uint32_t color	= 0xFFFFFFFF;
		if (antitile == false) {
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
			
		} else {
			float noisev = m_noise.fractal(5, x * 0.03f, y * 0.03f, z * 0.03f);
			color		 = 255 | (uint32_t)(255 - 64 + noisev * 64) << 8 | (uint32_t)(255 - 64 + noisev * 64) << 16 | 0xFF000000;
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
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
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

	static void inline AddFaceLeft(wiScene::MeshComponent* mesh, float x, float y, float z, bool antitile = false) {
		float offs	   = 0.25f;
		x			   = x - offs;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - offs, y - offs));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + offs, y - offs));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z + offs, y + offs));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x, z - offs, y + offs));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		uint32_t color = 0xFFFFFFFF;
		if (antitile == false) {
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
			
			
		} else {
			float noisev = m_noise.fractal(5, x * 0.03f, y * 0.03f, z * 0.03f);
			color		 = 255 | (uint32_t)(255 - 64 + noisev * 64) << 8 | (uint32_t)(255 - 64 + noisev * 64) << 16 | 0xFF000000;
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
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
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

	static void inline AddFaceFront(wiScene::MeshComponent* mesh, float x, float y, float z, bool antitile = false) {
		float offs	   = 0.25f;
		y			   = y + offs;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - offs, z - offs, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - offs, z + offs, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + offs, z + offs, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + offs, z - offs, y));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		uint32_t color = 0xFFFFFFFF;
		if (antitile == false) {
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
			
			
		} else {
			float noisev = m_noise.fractal(5, x * 0.03f, y * 0.03f, z * 0.03f);
			color		 = 255 | (uint32_t)(255 - 64 + noisev * 64) << 8 | (uint32_t)(255 - 64 + noisev * 64) << 16 | 0xFF000000;
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
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
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

	static void inline AddFaceBack(wiScene::MeshComponent* mesh, float x, float y, float z, bool antitile = false) {
		float offs	   = 0.25f;
		y			   = y - offs;
		uint32_t start = (uint32_t)mesh->vertex_positions.size();
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - offs, z - offs, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x - offs, z + offs, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + offs, z + offs, y));
		mesh->vertex_positions.emplace_back(XMFLOAT3(x + offs, z - offs, y));
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		mesh->vertex_windweights.emplace_back(0);
		uint32_t color = 0xFFFFFFFF;
		if (antitile == false) {
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
			mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
		} else {
			float noisev = m_noise.fractal(5, x * 0.03f, y * 0.03f, z * 0.03f);
			color		 = 255 | (uint32_t)(255 - 64 + noisev * 64) << 8 | (uint32_t)(255 - 64 + noisev * 64) << 16 | 0xFF000000;
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
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
		mesh->vertex_colors.emplace_back(color);
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
