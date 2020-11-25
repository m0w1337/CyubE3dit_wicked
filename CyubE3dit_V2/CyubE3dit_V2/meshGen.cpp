#include "stdafx.h"
#include "meshGen.h"

using namespace std;
using namespace wiECS;
using namespace wiScene;

meshGen::meshGen() {

}

MeshComponent* meshGen::AddMesh(Scene& scene, Entity materialID) {
	Entity entity = scene.Entity_CreateObject("editorTerrain");
	ObjectComponent& object = *scene.objects.GetComponent(entity);
	object.meshID = scene.Entity_CreateMesh("terrainMesh");
	MeshComponent* mesh = scene.meshes.GetComponent(object.meshID);
	mesh->subsets.emplace_back();
	mesh->subsets.back().materialID = materialID;
	mesh->subsets.back().indexOffset = 0;
	return mesh;
}
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