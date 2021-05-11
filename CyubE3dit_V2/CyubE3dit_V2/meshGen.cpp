#include "stdafx.h"
#include "meshGen.h"

using namespace std;
using namespace wiECS;
using namespace wiScene;
SimplexNoise meshGen::m_noise;
wiECS::Entity meshGen::toolBlockFaces[] = {wiECS::INVALID_ENTITY};
XMFLOAT3 meshGen::toolblockSize(0.501f, 0.501f, 0.501f);
meshGen::meshGen() {
}

MeshComponent* meshGen::AddMesh(Scene& scene, string _chunkID, wiECS::Entity _material, Entity* _newEntity) {
	*_newEntity				= scene.Entity_CreateObject(_chunkID);
	ObjectComponent& object = *scene.objects.GetComponent(*_newEntity);
	object.emissiveColor	= XMFLOAT4(0, 0, 0, 0);
	object.meshID			= scene.Entity_CreateMesh("");
	LayerComponent& layer	= *scene.layers.GetComponent(*_newEntity);
	layer.layerMask			= LAYER_CHUNKMESH;
	//scene.impostors.Create(object.meshID);
	MeshComponent* mesh = scene.meshes.GetComponent(object.meshID);
	mesh->subsets.emplace_back();
	mesh->subsets.back().materialID	 = _material;
	mesh->subsets.back().indexOffset = 0;
	return mesh;
}
void meshGen::ResizeToolBlock(float _x, float _y, float _z, bool _antitile) {
	float x				  = 0;
	float y				  = 0;
	wiScene::Scene& scene = wiScene::GetScene();
	for (size_t i = 0; i < 6; i++) {
		wiScene::MeshComponent* mesh = scene.meshes.GetComponent(toolBlockFaces[i]);

		for (size_t ii = 0; ii < 4; ii++) {
			mesh->vertex_positions[ii].x = mesh->vertex_positions[ii].x / (toolblockSize.x - 0.001) * (_x - 0.001);
			mesh->vertex_positions[ii].y = mesh->vertex_positions[ii].y / (toolblockSize.y - 0.001) * (_y - 0.001);
			mesh->vertex_positions[ii].z = mesh->vertex_positions[ii].z / (toolblockSize.z - 0.001) * (_z - 0.001);
		}
		switch (i) {
			case cyBlocks::FACE_TOP:
			case cyBlocks::FACE_BOTTOM:
				for (size_t ii = 0; ii < 4; ii++) {
					mesh->vertex_uvset_0[ii].x = mesh->vertex_uvset_0[ii].x / (toolblockSize.z * 2) * _z * 2;
					mesh->vertex_uvset_0[ii].y = mesh->vertex_uvset_0[ii].y / (toolblockSize.x * 2) * _x * 2;
				}
				break;
			case cyBlocks::FACE_LEFT:
			case cyBlocks::FACE_RIGHT:
				for (size_t ii = 0; ii < 4; ii++) {
					mesh->vertex_uvset_0[ii].x = mesh->vertex_uvset_0[ii].x / (toolblockSize.z * 2) * _z * 2;
					mesh->vertex_uvset_0[ii].y = mesh->vertex_uvset_0[ii].y / (toolblockSize.y * 2) * _y * 2;
				}
				break;
			case cyBlocks::FACE_FRONT:
			case cyBlocks::FACE_BACK:
				for (size_t ii = 0; ii < 4; ii++) {
					mesh->vertex_uvset_0[ii].x = mesh->vertex_uvset_0[ii].x / (toolblockSize.x * 2) * _x * 2;
					mesh->vertex_uvset_0[ii].y = mesh->vertex_uvset_0[ii].y / (toolblockSize.y * 2) * _y * 2;
				}
				break;
		}
		
		mesh->CreateRenderData();
	}
	toolblockSize.x = _x;
	toolblockSize.y = _y;
	toolblockSize.z = _z;
	
}

wiECS::Entity meshGen::AddToolBoxFace(wiECS::Entity _material, uint8_t _face) {
	toolblockSize		  = XMFLOAT3(0.5f, 0.5f, 0.5f);
	float x				  = 0;
	float y				  = 0;
	wiScene::Scene& scene = wiScene::GetScene();
	wiECS::Entity ret	  = scene.Entity_CreateMesh("");
	MeshComponent* mesh	  = scene.meshes.GetComponent(ret);
	mesh->subsets.emplace_back();
	mesh->subsets.back().materialID	 = _material;
	mesh->subsets.back().indexOffset = 0;
	switch (_face) {
		case cyBlocks::FACE_TOP:
			AddFaceTop(mesh, 0.25, 0.25, 0.25);
			break;
		case cyBlocks::FACE_BOTTOM:
			AddFaceBottom(mesh, 0.25, 0.25, 0.25);
			break;
		case cyBlocks::FACE_LEFT:
			AddFaceLeft(mesh, 0.25, 0.25, 0.25);
			break;
		case cyBlocks::FACE_RIGHT:
			AddFaceRight(mesh, 0.25, 0.25, 0.25);
			break;
		case cyBlocks::FACE_FRONT:
			AddFaceFront(mesh, 0.25, 0.25, 0.25);
			break;
		case cyBlocks::FACE_BACK:
			AddFaceBack(mesh, 0.25, 0.25, 0.25);
			break;
	}
	/*
	//top
	mesh->vertex_positions.emplace_back(XMFLOAT3(0, 0, 0));
	mesh->vertex_positions.emplace_back(XMFLOAT3(0, offsZ, 0));
	mesh->vertex_positions.emplace_back(XMFLOAT3(offsX, offsZ, 0));
	mesh->vertex_positions.emplace_back(XMFLOAT3(offsX, 0, 0));
	mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 1));
	mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1, 0));
	mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
	mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1));

	mesh->indices.emplace_back(0);
	mesh->indices.emplace_back(1);
	mesh->indices.emplace_back(2);

	mesh->indices.emplace_back(2);
	mesh->indices.emplace_back(3);
	mesh->indices.emplace_back(0);
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));*/
	mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
	mesh->SetDynamic(false);
	mesh->CreateRenderData();
	return ret;
}

/*
wiECS::Entity meshGen::addToolBoxBottom(Scene& scene, wiECS::Entity _material, float x, float y, float z, float sizeX, float sizeY, float sizeZ, bool antitile) {
	wiECS::Entity ret	= scene.Entity_CreateMesh("");
	MeshComponent* mesh = scene.meshes.GetComponent(ret);
	float offsX			= 0.5f * sizeX;
	float offsY			= 0.5f * sizeY;
	float offsZ			= 0.5f * sizeZ;
	mesh->subsets.emplace_back();
	mesh->subsets.back().materialID	 = _material;
	mesh->subsets.back().indexOffset = 0;
	//bottom
	mesh->vertex_positions.emplace_back(XMFLOAT3(x - offsX, z - offsZ, y - offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x + offsX, z - offsZ, y - offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x - offsX, z - offsZ, y + offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x + offsX, z - offsZ, y + offsY));
	if (antitile == false) {
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * sizeY * 2, 1 * sizeX * 2));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * sizeY * 2, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1 * sizeX * 2));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
	} else {
		float uvx = x / (ANTITILE_FACT / 2);
		uvx		  = (uvx - floorf(uvx));
		float uvy = y / (ANTITILE_FACT / 2);
		uvy		  = (uvy - floorf(uvy));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvy, uvx));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvy, (1.0f * sizeX * 2) / ANTITILE_FACT + uvx));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2((1.0f * sizeY * 2) / ANTITILE_FACT + uvy, uvx));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2((1.0f * sizeY * 2) / ANTITILE_FACT + uvy, (1.0f * sizeX * 2) / ANTITILE_FACT + uvx));
	}
	mesh->indices.emplace_back(1);
	mesh->indices.emplace_back(0);
	mesh->indices.emplace_back(2);

	mesh->indices.emplace_back(3);
	mesh->indices.emplace_back(1);
	mesh->indices.emplace_back(2);
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, -1, 0));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, -1, 0));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, -1, 0));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, -1, 0));
	mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
	mesh->SetDynamic(false);
	mesh->CreateRenderData();
	return ret;
}
wiECS::Entity meshGen::addToolBoxRight(Scene& scene, wiECS::Entity _material, float x, float y, float z, float sizeX, float sizeY, float sizeZ, bool antitile) {
	wiECS::Entity ret	= scene.Entity_CreateMesh("");
	MeshComponent* mesh = scene.meshes.GetComponent(ret);
	float offsX			= 0.5f * sizeX;
	float offsY			= 0.5f * sizeY;
	float offsZ			= 0.5f * sizeZ;
	mesh->subsets.emplace_back();
	mesh->subsets.back().materialID	 = _material;
	mesh->subsets.back().indexOffset = 0;
	//right
	mesh->vertex_positions.emplace_back(XMFLOAT3(x + offsX, z - offsZ, y - offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x + offsX, z + offsZ, y - offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x + offsX, z + offsZ, y + offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x + offsX, z - offsZ, y + offsY));
	if (antitile == false) {
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1 * sizeZ * 2));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * sizeY * 2, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * sizeY * 2, 1 * sizeZ * 2));

	} else {
		float uvx = y / (ANTITILE_FACT / 2);
		uvx		  = uvx - floorf(uvx);
		float uvy = z / (ANTITILE_FACT / 2);
		uvy		  = uvy - floorf(uvy);
		uvy		  = 1 - uvy;
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, (1.0f * sizeZ * 2) / ANTITILE_FACT + uvy));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, uvy));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2((1.0f * sizeY * 2) / ANTITILE_FACT + uvx, uvy));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2((1.0f * sizeY * 2) / ANTITILE_FACT + uvx, (1.0f * sizeZ * 2) / ANTITILE_FACT + uvy));
	}
	mesh->indices.emplace_back(2);
	mesh->indices.emplace_back(1);
	mesh->indices.emplace_back(0);

	mesh->indices.emplace_back(0);
	mesh->indices.emplace_back(3);
	mesh->indices.emplace_back(2);
	mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
	mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
	mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
	mesh->vertex_normals.emplace_back(XMFLOAT3(1, 0, 0));
	mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
	mesh->SetDynamic(false);
	mesh->CreateRenderData();
	return ret;
}
wiECS::Entity meshGen::addToolBoxLeft(Scene& scene, wiECS::Entity _material, float x, float y, float z, float sizeX, float sizeY, float sizeZ, bool antitile) {
	wiECS::Entity ret	= scene.Entity_CreateMesh("");
	MeshComponent* mesh = scene.meshes.GetComponent(ret);
	float offsX			= 0.5f * sizeX;
	float offsY			= 0.5f * sizeY;
	float offsZ			= 0.5f * sizeZ;
	mesh->subsets.emplace_back();
	mesh->subsets.back().materialID	 = _material;
	mesh->subsets.back().indexOffset = 0;
	mesh->vertex_positions.emplace_back(XMFLOAT3(x - offsX, z - offsZ, y - offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x - offsX, z + offsZ, y - offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x - offsX, z + offsZ, y + offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x - offsX, z - offsZ, y + offsY));
	if (antitile == false) {
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1 * sizeZ * 2));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * sizeY * 2, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * sizeY * 2, 1 * sizeZ * 2));

	} else {
		float uvx = y / (ANTITILE_FACT / 2);
		uvx		  = uvx - floorf(uvx);
		float uvy = z / (ANTITILE_FACT / 2);
		uvy		  = uvy - floorf(uvy);
		uvy		  = 1 - uvy;
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, (1.0f * sizeZ * 2) / ANTITILE_FACT + uvy));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(uvx, uvy));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2((1.0f * sizeY * 2) / ANTITILE_FACT + uvx, uvy));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2((1.0f * sizeY * 2) / ANTITILE_FACT + uvx, (1.0f * sizeZ * 2) / ANTITILE_FACT + uvy));
	}
	mesh->indices.emplace_back();
	mesh->indices.emplace_back(1);
	mesh->indices.emplace_back(2);

	mesh->indices.emplace_back(2);
	mesh->indices.emplace_back(3);
	mesh->indices.emplace_back();
	mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
	mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
	mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
	mesh->vertex_normals.emplace_back(XMFLOAT3(-1, 0, 0));
	mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
	mesh->SetDynamic(false);
	mesh->CreateRenderData();
	return ret;
}
wiECS::Entity meshGen::addToolBoxFront(Scene& scene, wiECS::Entity _material, float x, float y, float z, float sizeX, float sizeY, float sizeZ, bool antitile) {
	wiECS::Entity ret	= scene.Entity_CreateMesh("");
	MeshComponent* mesh = scene.meshes.GetComponent(ret);
	float offsX			= 0.5f * sizeX;
	float offsY			= 0.5f * sizeY;
	float offsZ			= 0.5f * sizeZ;
	mesh->subsets.emplace_back();
	mesh->subsets.back().materialID	 = _material;
	mesh->subsets.back().indexOffset = 0;
	mesh->vertex_positions.emplace_back(XMFLOAT3(x - offsX, z - offsZ, y + offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x - offsX, z + offsZ, y + offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x + offsX, z + offsZ, y + offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x + offsX, z - offsZ, y + offsY));
	if (antitile == false) {
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * sizeX * 2, 1 * sizeZ * 2));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * sizeX * 2, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1 * sizeZ * 2));

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
	mesh->indices.emplace_back();
	mesh->indices.emplace_back(1);
	mesh->indices.emplace_back(2);

	mesh->indices.emplace_back(2);
	mesh->indices.emplace_back(3);
	mesh->indices.emplace_back();
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, 1));
	mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
	mesh->SetDynamic(false);
	mesh->CreateRenderData();
	return ret;
}
wiECS::Entity meshGen::addToolBoxBack(Scene& scene, wiECS::Entity _material, float x, float y, float z, float sizeX, float sizeY, float sizeZ, bool antitile) {
	wiECS::Entity ret	= scene.Entity_CreateMesh("");
	MeshComponent* mesh = scene.meshes.GetComponent(ret);
	float offsX			= 0.5f * sizeX;
	float offsY			= 0.5f * sizeY;
	float offsZ			= 0.5f * sizeZ;
	mesh->subsets.emplace_back();
	mesh->subsets.back().materialID	 = _material;
	mesh->subsets.back().indexOffset = 0;
	//back
	mesh->vertex_positions.emplace_back(XMFLOAT3(x - offsX, z - offsZ, y - offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x - offsX, z + offsZ, y - offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x + offsX, z + offsZ, y - offsY));
	mesh->vertex_positions.emplace_back(XMFLOAT3(x + offsX, z - offsZ, y - offsY));
	if (antitile == false) {
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * sizeX * 2, 1 * sizeZ * 2));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(1 * sizeX * 2, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 0));
		mesh->vertex_uvset_0.emplace_back(XMFLOAT2(0, 1 * sizeZ * 2));

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
	mesh->indices.emplace_back(2);
	mesh->indices.emplace_back(1);
	mesh->indices.emplace_back();

	mesh->indices.emplace_back();
	mesh->indices.emplace_back(3);
	mesh->indices.emplace_back(2);
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
	mesh->vertex_normals.emplace_back(XMFLOAT3(0, 0, -1));
	mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;
	mesh->SetDynamic(false);
	mesh->CreateRenderData();
	return ret;
}*/
/*
MeshComponent* meshGen::AddMesh(Scene& scene,int32_t x, int32_t y){
	Entity entity = scene.Entity_CreateObject("editorTerrain");
	ObjectComponent& object = *scene.objects.GetComponent(entity);
	object.meshID = scene.Entity_CreateMesh("terrainMesh");
	MeshComponent* mesh = scene.meshes.GetComponent(object.meshID);
	//mesh->SetTerrain(true);
	mesh->subsets.emplace_back();
	mesh->subsets.back().materialID = scene.Entity_CreateMaterial("terrainMaterial");
	mesh->subsets.back().indexOffset = 0;
	MaterialComponent* material = scene.materials.GetComponent(mesh->subsets.back().materialID);
	material->baseColorMap = wiResourceManager::Load("images/floor.jpg");
	material->normalMap = wiResourceManager::Load("images/floor_norm.jpg");
	material->baseColorMapName = "images/floor.jpg";
	material->normalMapName = "images/floor_norm.jpg";
	material->displacementMap = wiResourceManager::Load("images/floor_disp.jpg");
	material->SetMetalness(0.1);
	material->SetRoughness(0.5);
	material->displacementMapName = "images/floor_disp.jpg";
	material->SetDisplacementMapping(true);
	material->SetUVSet_DisplacementMap(0);
	material->SetUVSet_NormalMap(0);
	material->SetNormalMapStrength(1);
	//material->SetReflectance(100);
	material->SetDirty();
	return mesh;
}*/
