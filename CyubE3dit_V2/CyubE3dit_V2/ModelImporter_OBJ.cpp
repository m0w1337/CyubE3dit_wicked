#include "stdafx.h"
#include "wiScene.h"
#include "ModelImporter.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <istream>
#include <streambuf>

using namespace std;
using namespace wiGraphics;
using namespace wiScene;
using namespace wiECS;

struct membuf : std::streambuf {
	membuf(char* begin, char* end) {
		this->setg(begin, begin, end);
	}
};

// Custom material file reader:
class MaterialFileReader : public tinyobj::MaterialReader {
public:
	explicit MaterialFileReader(const std::string& mtl_basedir) :
	  m_mtlBaseDir(mtl_basedir) {}
	virtual ~MaterialFileReader() {}
	virtual bool operator()(const std::string& matId,
							std::vector<tinyobj::material_t>* materials,
							std::map<std::string, int>* matMap, std::string* err) {
		std::string filepath;

		if (!m_mtlBaseDir.empty()) {
			filepath = std::string(m_mtlBaseDir) + matId;
		} else {
			filepath = matId;
		}

		std::vector<uint8_t> filedata;
		if (!wiHelper::FileRead(filepath, filedata))
		{
			std::stringstream ss;
			ss << "WARN: Material file [ " << filepath << " ] not found." << std::endl;
			if (err) {
				(*err) += ss.str();
			}
			return false;
		}

		membuf sbuf((char*)filedata.data(), (char*)filedata.data() + filedata.size());
		std::istream matIStream(&sbuf);

		std::string warning;
		LoadMtl(matMap, materials, &matIStream, &warning);

		if (!warning.empty()) {
			if (err) {
				(*err) += warning;
			}
		}

		return true;
	}

private:
	std::string m_mtlBaseDir;
};

// Transform the data from OBJ space to engine-space:
static const bool transform_to_LH = true;

wiECS::Entity ImportModel_OBJ(const std::string& fileName, Scene& scene, uint8_t windMode) {
	string directory, name;
	wiHelper::SplitPath(fileName, directory, name);

	tinyobj::attrib_t obj_attrib;
	vector<tinyobj::shape_t> obj_shapes;
	vector<tinyobj::material_t> obj_materials;
	string obj_errors;
	Entity meshEntity = wiECS::INVALID_ENTITY;

	std::vector<uint8_t> filedata;
	bool success = wiHelper::FileRead(fileName, filedata);

	if (success)
	{
		membuf sbuf((char*)filedata.data(), (char*)filedata.data() + filedata.size());
		std::istream in(&sbuf);
		MaterialFileReader matFileReader(directory);
		success = tinyobj::LoadObj(&obj_attrib, &obj_shapes, &obj_materials, &obj_errors, &in, &matFileReader, true);
	} else
	{
		obj_errors = "Failed to read file: " + fileName;
	}

	if (!obj_errors.empty())
	{
		wiBackLog::post(obj_errors.c_str());
	}

	if (success)
	{
		// Load material library:
		vector<Entity> materialLibrary = {};
		for (auto& obj_material : obj_materials)
		{
			Entity materialEntity		= scene.Entity_CreateMaterial(obj_material.name);
			MaterialComponent& material = *scene.materials.GetComponent(materialEntity);
			material.emissiveMapName	= obj_material.emissive_texname;
			material.emissiveColor		= XMFLOAT4(obj_material.emission[0], obj_material.emission[1], obj_material.emission[2], 1);
			material.baseColor		  = XMFLOAT4(obj_material.diffuse[0], obj_material.diffuse[1], obj_material.diffuse[2], 1);
			material.baseColorMapName = obj_material.diffuse_texname;
			material.occlusionMapName = obj_material.displacement_texname;
			//material.emissiveColor.x = obj_material.emission[0];
			//material.emissiveColor.y = obj_material.emission[1];
			//material.emissiveColor.z = obj_material.emission[2];
			//material.emissiveColor.w = max(obj_material.emission[0], max(obj_material.emission[1], obj_material.emission[2]));
			material.refraction = obj_material.ior;
			material.metalness		 = 0.01f;  //obj_material.metallic;
			material.normalMapName	 = obj_material.normal_texname;
			material.surfaceMapName	 = obj_material.specular_texname;
			material.roughness		 = 0.8f;  //obj_material.roughness;
			material.reflectance	 = 0.0f;
			if (obj_material.alpha_texname != "") {
				//material.userBlendMode = BLENDMODE_ALPHA;
				material.SetAlphaRef(0.5f);
			}
			material.SetUseWind(true);
			if (material.normalMapName.empty())
			{
				material.normalMapName = obj_material.bump_texname;
			}
			if (material.surfaceMapName.empty())
			{
				material.surfaceMapName = obj_material.specular_highlight_texname;
			}

			if (!material.surfaceMapName.empty())
			{
				material.surfaceMap = wiResourceManager::Load(directory + material.surfaceMapName);
				material.surfaceMapName.clear();
			}
			if (!material.baseColorMapName.empty())
			{
				material.baseColorMap = wiResourceManager::Load(directory + material.baseColorMapName);
				material.baseColorMapName.clear();
			}
			if (!material.normalMapName.empty())
			{
				material.normalMap = wiResourceManager::Load(directory + material.normalMapName);
				material.normalMapName.clear();
			}
			if (!material.displacementMapName.empty())
			{
				material.displacementMap = wiResourceManager::Load(directory + material.displacementMapName);
				material.displacementMapName.clear();
			}
			if (!material.occlusionMapName.empty())
			{
				material.occlusionMap = wiResourceManager::Load(directory + material.occlusionMapName);
				material.occlusionMapName.clear();
				material.SetCustomShaderID(MaterialComponent::SHADERTYPE_PBR_PARALLAXOCCLUSIONMAPPING);
				material.SetParallaxOcclusionMapping(1.0f);
			}
			if (!material.emissiveMapName.empty())
			{
				material.emissiveMap = wiResourceManager::Load(directory + material.emissiveMapName);
				material.emissiveMapName.clear();
				material.SetEmissiveStrength(15.0);
			}

			materialLibrary.push_back(materialEntity);	// for subset-indexing...
		}

		if (materialLibrary.empty())
		{
			// Create default material if nothing was found:
			Entity materialEntity		= scene.Entity_CreateMaterial("OBJImport_defaultMaterial");
			MaterialComponent& material = *scene.materials.GetComponent(materialEntity);
			materialLibrary.push_back(materialEntity);	// for subset-indexing...
		}

		// Load objects, meshes:
		if (obj_shapes.size() > 0) {
			//Entity objectEntity		= scene.Entity_CreateObject("Mesh");
			meshEntity = scene.Entity_CreateMesh("Mesh");
			//ObjectComponent& object = *scene.objects.GetComponent(objectEntity);
			MeshComponent& mesh = *scene.meshes.GetComponent(meshEntity);
			mesh.SetDoubleSided(true);
			//scene.impostors.Create(meshEntity);
			//object.meshID = meshEntity;
			for (auto& shape : obj_shapes)
			{
				unordered_map<int, int> registered_materialIndices = {};
				unordered_map<size_t, uint32_t> uniqueVertices	   = {};

				for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
				{
					tinyobj::index_t reordered_indices[] = {
						shape.mesh.indices[i + 0],
						shape.mesh.indices[i + 1],
						shape.mesh.indices[i + 2],
					};

					// todo: option param would be better
					bool flipCulling = false;
					if (flipCulling)
					{
						reordered_indices[1] = shape.mesh.indices[i + 2];
						reordered_indices[2] = shape.mesh.indices[i + 1];
					}

					for (auto& index : reordered_indices)
					{
						XMFLOAT3 pos = XMFLOAT3(
							obj_attrib.vertices[index.vertex_index * 3 + 0],
							obj_attrib.vertices[index.vertex_index * 3 + 1],
							obj_attrib.vertices[index.vertex_index * 3 + 2]);

						XMFLOAT3 nor = XMFLOAT3(0, 0, 0);
						if (!obj_attrib.normals.empty())
						{
							nor = XMFLOAT3(
								obj_attrib.normals[index.normal_index * 3 + 0],
								obj_attrib.normals[index.normal_index * 3 + 1],
								obj_attrib.normals[index.normal_index * 3 + 2]);
						}

						XMFLOAT2 tex = XMFLOAT2(0, 0);
						if (index.texcoord_index >= 0 && !obj_attrib.texcoords.empty())
						{
							tex = XMFLOAT2(
								obj_attrib.texcoords[index.texcoord_index * 2 + 0],
								1 - obj_attrib.texcoords[index.texcoord_index * 2 + 1]);
						}

						int materialIndex = max(0, shape.mesh.material_ids[i / 3]);	 // this indexes the material library
						if (registered_materialIndices.count(materialIndex) == 0)
						{
							registered_materialIndices[materialIndex] = (int)mesh.subsets.size();
							mesh.subsets.push_back(MeshComponent::MeshSubset());
							mesh.subsets.back().materialID	= materialLibrary[materialIndex];
							mesh.subsets.back().indexOffset = (uint32_t)mesh.indices.size();
						}

						if (transform_to_LH)
						{
							pos.z *= -1;
							nor.z *= -1;
						}

						// eliminate duplicate vertices by means of hashing:
						size_t vertexHash = 0;
						wiHelper::hash_combine(vertexHash, index.vertex_index);
						wiHelper::hash_combine(vertexHash, index.normal_index);
						wiHelper::hash_combine(vertexHash, index.texcoord_index);
						wiHelper::hash_combine(vertexHash, materialIndex);

						if (uniqueVertices.count(vertexHash) == 0)
						{
							uniqueVertices[vertexHash] = (uint32_t)mesh.vertex_positions.size();
							mesh.vertex_positions.push_back(pos);
							mesh.vertex_normals.push_back(nor);
							mesh.vertex_uvset_0.push_back(tex);
							switch (windMode) {
								case 0:
									mesh.vertex_windweights.push_back(0);
									break;
								case 1:
									if (pos.y > 0.2f) {
										mesh.vertex_windweights.push_back(50);
									} else {
										mesh.vertex_windweights.push_back(0);
									}
									break;
								case 2:
									if (pos.y > 1.0f && abs(pos.x) > 0.5f && abs(pos.z) > 0.5f) {
										if (obj_materials[materialIndex].alpha_texname != "") {
											mesh.vertex_windweights.push_back(100);// * pos.z);
										} else {
											mesh.vertex_windweights.push_back(20);// * pos.x);
										}

									} else {
										mesh.vertex_windweights.push_back(0);
									}
									break;
							}
							if (windMode == 1) {
								
							}
						}
						mesh.indices.push_back(uniqueVertices[vertexHash]);
						mesh.subsets.back().indexCount++;
					}
				}
				//mesh.ComputeNormals(MeshComponent::COMPUTE_NORMALS_SMOOTH);
				//scene.impostors.Create(meshEntity);
				mesh.CreateRenderData();
			}
		}
		//scene.Update(0);

	} else
	{
		wiHelper::messageBox("OBJ import failed! Check backlog for errors!", "Error!");
	}
	return meshEntity;
}
