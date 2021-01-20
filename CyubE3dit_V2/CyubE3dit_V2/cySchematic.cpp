#include "stdafx.h"
#include "cySchematic.h"
#include <fstream>

using namespace std;
using namespace wiScene;
extern mutex m;
unordered_map<wiECS::Entity, cySchematic*> cySchematic::m_schematics;

cySchematic::cySchematic(string filename) {
	wiScene::Scene tmpScene;
	ifstream file;
	file.open(filename, fstream::in | ios::binary);
	m_chunkdata = nullptr;
	if (file.is_open()) {
		file.read(reinterpret_cast<char*>(&size.x), sizeof(size.x));
		file.read(reinterpret_cast<char*>(&size.y), sizeof(size.y));
		file.read(reinterpret_cast<char*>(&size.z), sizeof(size.z));
		uint32_t uncompressedSize = 0;
		file.seekg(0, ios_base::end);
		int flength = file.tellg();
		file.seekg(flength - 4, ios_base::beg);
		file.read(reinterpret_cast<char*>(&uncompressedSize), sizeof(uncompressedSize));
		m_chunkdata		 = (char*)malloc(uncompressedSize);
		char* compressed = (char*)malloc(flength - HEADER_SIZE);
		file.seekg(12, ios_base::beg);
		uint32_t magic;
		file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
		if (magic == 0x13371337) {
			file.read(compressed, flength - HEADER_SIZE);
			try {
				LZ4_decompress_fast(compressed, m_chunkdata, uncompressedSize);
			}
			catch (...) {
				wiHelper::messageBox("Error while uncompressing schematic file.");
				free(m_chunkdata);
				free(compressed);
				m_chunkdata = nullptr;
				return;
			}
			free(compressed);
			uint32_t dSize = 0;
			size_t offset  = size.x * size.y * size.z;
			memcpy(&dSize, m_chunkdata + offset, 4);
			offset += 4;
			blockpos_t pos;
			uint32_t cID = 0;
			while (dSize) {
				memcpy(&(pos.x), m_chunkdata + offset, 2);
				memcpy(&(pos.y), m_chunkdata + offset + 2, 2);
				memcpy(&(pos.z), m_chunkdata + offset + 4, 2);
				memcpy(&(cID), m_chunkdata + offset + 6, 4);
				offset += 10;
				m_cBlocks[pos] = cID;
				dSize--;
			}

			memcpy(&dSize, m_chunkdata + offset, 4);
			offset += 4;
			uint8_t rot = 0;
			while (dSize) {
				memcpy(&(pos.x), m_chunkdata + offset, 2);
				memcpy(&(pos.y), m_chunkdata + offset + 2, 2);
				memcpy(&(pos.z), m_chunkdata + offset + 4, 2);
				memcpy(&(rot), m_chunkdata + offset + 6, 1);
				offset += 7;
				m_Torches[pos] = rot;
				dSize--;
			}
		} else {
			wiHelper::messageBox("Broken schematic file, sorry.");
			free(m_chunkdata);
			free(compressed);
			m_chunkdata = nullptr;
			return;
		}
	}
}
/*
cySchematic& cySchematic::getSchematic(wiECS::Entity antity) {
	
}*/

void cySchematic::addSchematic(std::string filename) {
	cySchematic* schem	 = new cySchematic(filename);
	wiECS::Entity entity = schem->RenderSchematic(0, 0, 120);
	m_schematics[entity] = schem;
}

wiECS::Entity cySchematic::RenderSchematic(const int32_t relX, const int32_t relY, const int32_t relZ) {
	chunkLoader::face_t tmpface;
	wiECS::Entity entity = wiECS::INVALID_ENTITY;
	vector<chunkLoader::face_t> faces;
	vector<torch_t> torches;
	uint8_t stepsize = 1;
	if (m_chunkdata == nullptr)
		return wiECS::INVALID_ENTITY;
	std::vector<wiECS::Entity> groupChilds;
	wiScene::Scene tmpScene;
	for (int_fast16_t z = 0; z < size.z; z++) {
		for (uint_fast16_t x = 0; x < size.x; x++) {
			for (uint_fast16_t y = 0; y < size.y; y++) {
				uint8_t blocktype = (uint8_t) * (m_chunkdata + x + size.x * y + size.x * size.y * z);
				if (cyBlocks::m_regBlockTypes[blocktype] <= cyBlocks::BLOCKTYPE_ALPHA) {
					uint8_t neighbour[6];
					if (z < size.z - 1)
						neighbour[0] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata + x + size.x * y + size.x * size.y * (z + 1))];
					else
						neighbour[0] = cyBlocks::BLOCKTYPE_VOID;

					if (z > 1)
						neighbour[1] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata + x + size.x * y + size.x * size.y * (z - 1))];
					else
						neighbour[1] = cyBlocks::BLOCKTYPE_VOID;

					if (y < size.y - 1)
						neighbour[4] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata + x + size.x * (y + 1) + size.x * size.y * z)];
					else
						neighbour[4] = cyBlocks::BLOCKTYPE_VOID;

					if (y >= 1)
						neighbour[5] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata + x + size.x * (y - 1) + size.x * size.y * z)];
					else
						neighbour[5] = cyBlocks::BLOCKTYPE_VOID;

					if (x < size.x - 1)
						neighbour[3] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata + 1 + x + size.x * y + size.x * size.y * z)];
					else
						neighbour[3] = cyBlocks::BLOCKTYPE_VOID;

					if (x >= 1)
						neighbour[2] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata - 1 + x + size.x * y + size.x * size.y * z)];
					else
						neighbour[2] = cyBlocks::BLOCKTYPE_VOID;

					uint8_t antitile = 0;

					if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_MOD) {
						blockpos_t pos(x, y, z);
						std::unordered_map<blockpos_t, uint32_t>::const_iterator cBlock = m_cBlocks.find(pos);
						if (cBlock != m_cBlocks.end()) {
							for (uint8_t ft = 0; ft < 6; ft++) {
								if (neighbour[ft] > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
									tmpface.x		 = relX + x / 2.0f;
									tmpface.y		 = relY + 16 - y / 2.0f;
									tmpface.z		 = relZ + z / 2.0f;
									tmpface.antitile = false;
									tmpface.face	 = ft;
									tmpface.material = cyBlocks::m_cBlockTypes[cBlock->second].material[ft];
									faces.emplace_back(tmpface);
								}
							}
						}
					} else {
						for (uint8_t ft = 0; ft < 6; ft++) {
							if (neighbour[ft] != cyBlocks::m_regBlockTypes[blocktype]) {
								tmpface.x		 = relX + x / 2.0f;
								tmpface.y		 = relY + 16 - y / 2.0f;
								tmpface.z		 = relZ + z / 2.0f;
								tmpface.antitile = cyBlocks::m_regBlockFlags[blocktype][ft] & cyBlocks::BLOCKFLAG_ANTITILE;
								tmpface.face	 = ft;
								tmpface.material = cyBlocks::m_regBlockMats[blocktype][ft];
								faces.emplace_back(tmpface);
							}
						}
					}
				} else if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_BILLBOARD) {
					tmpface.x		 = relX + x / 2.0f;
					tmpface.y		 = relY + 16 - y / 2.0f;
					tmpface.z		 = relZ + z / 2.0f;
					tmpface.material = cyBlocks::m_regBlockMats[blocktype][0];
					tmpface.face	 = cyBlocks::FACE_BILLBOARD;
					faces.emplace_back(tmpface);
				} else if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_TORCH) {
					blockpos_t pos(x, y, z);
					auto it = m_Torches.find(pos);
					if (it != m_Torches.end()) {
						torches.push_back(torch_t(blocktype, x, y, z, it->second));
					}
				}
			}
		}
	}
	if (faces.size()) {
		SimplexNoise noise;
		sort(faces.begin(), faces.end());
		MeshComponent* mesh;
		string meshname = to_string(relX) + to_string(relY);
		if (wiScene::GetScene().materials.GetComponent(faces[0].material) != nullptr) {
			mesh = meshGen::AddMesh(tmpScene, "Schematic", faces[0].material, &entity);
		} else {
			mesh = meshGen::AddMesh(tmpScene, "Schematic", cyBlocks::m_fallbackMat, &entity);
		}
		tmpScene.objects.GetComponent(entity)->SetUserStencilRef(chunkLoader::STENCIL_HIGHLIGHT_OBJ);
		LayerComponent* layer = tmpScene.layers.GetComponent(entity);
		layer->layerMask	  = LAYER_SCHEMATIC;
		wiECS::Entity currMat = faces[0].material;
		mesh->SetDoubleSided(true);
		for (unsigned i = 0; i < faces.size(); ++i)
		{
			if (faces[i].material != currMat) {
				if (wiScene::GetScene().materials.GetComponent(faces[i].material) != nullptr) {
					meshGen::newMaterial(mesh, faces[i].material);
				}
				currMat = faces[i].material;
			}
			switch (faces[i].face) {
				case cyBlocks::FACE_TOP:
					meshGen::AddFaceTop(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_BOTTOM:
					meshGen::AddFaceBottom(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_LEFT:
					meshGen::AddFaceLeft(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_RIGHT:
					meshGen::AddFaceRight(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_BACK:
					meshGen::AddFaceBack(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_FRONT:
					meshGen::AddFaceFront(mesh, faces[i].x, faces[i].y, faces[i].z, stepsize, faces[i].antitile);
					break;
				case cyBlocks::FACE_BILLBOARD:
					if (stepsize == 1) {
						meshGen::AddBillboard(mesh, faces[i].x, faces[i].y, faces[i].z);
					}
					break;
			}
		}
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;

		mesh->SetDynamic(false);
		mesh->CreateRenderData();
		float arrowsize = (float)min(size.x, size.y);// / 100;
		wiECS::Entity objEnt	= tmpScene.Entity_CreateObject("cwArrow");
		groupChilds.push_back(objEnt);
		ObjectComponent* object = tmpScene.objects.GetComponent(objEnt);
		TransformComponent* tf	= tmpScene.transforms.GetComponent(objEnt);
		layer	= tmpScene.layers.GetComponent(objEnt);
		layer->layerMask			= LAYER_SCHEMATIC;
		object->meshID			= cyBlocks::m_toolMeshes[0];
		object->parentObject		= entity;
		tf->Translate(XMFLOAT3(relX + 5 + size.x / 4, relZ + 3 + size.z/2, relY + size.y/4));
		tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
		tf->RotateRollPitchYaw(XMFLOAT3(0, 0, 0));
		tf->UpdateTransform();
		objEnt				= tmpScene.Entity_CreateObject("ccArrow");
		groupChilds.push_back(objEnt);
		object				= tmpScene.objects.GetComponent(objEnt);
		layer				= tmpScene.layers.GetComponent(objEnt);
		layer->layerMask	 = LAYER_SCHEMATIC;
		tf					= tmpScene.transforms.GetComponent(objEnt);
		object->meshID		= cyBlocks::m_treeMeshes[0];
		object->parentObject = entity;
		tf->Translate(XMFLOAT3(relX + 5 + size.x / 4, relZ - 3, relY + size.y / 4));
		tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
		tf->RotateRollPitchYaw(XMFLOAT3(PI, 0, 0));
		tf->UpdateTransform();
		for (size_t i = 0; i < trees.size(); i++) {
			if (trees[i].scale.x > 2 || trees[i].scale.y > 2 || trees[i].scale.z > 2) {
				wiHelper::messageBox("Tree scaling out of bounds!", "Error!");
				wiBackLog::post("Weird tree data: in Schematic.");
			} else {
				objEnt = tmpScene.Entity_CreateObject("tree");
				//groupChilds.push_back(objEnt);
				object = tmpScene.objects.GetComponent(objEnt);
				object->SetUserStencilRef(chunkLoader::STENCIL_HIGHLIGHT_OBJ);
				layer			= tmpScene.layers.GetComponent(objEnt);
				layer->layerMask = LAYER_SCHEMATIC;
				tf				= tmpScene.transforms.GetComponent(objEnt);
				switch (trees[i].type) {
					case 0:	 //leaf trees (light wood)
						object->meshID = cyBlocks::m_treeMeshes[((trees[i].pos.x + trees[i].pos.y + trees[i].pos.z) % 3)];
						break;
					case 1:	 //needle trees (dark wood)
						object->meshID = cyBlocks::m_treeMeshes[3];
						break;
					case 2:	 //cactus
					case 3:
					case 4:
					case 5:
						object->meshID = cyBlocks::m_treeMeshes[4];
						break;
					default:  //desert grass 6,7
						object->meshID = cyBlocks::m_treeMeshes[5];
						break;
				}
				tf->Translate(XMFLOAT3(relX + trees[i].pos.x / 2, relZ + trees[i].pos.z / 2 - 0.25, relY + 16 - trees[i].pos.y / 2));
				tf->Scale(XMFLOAT3(trees[i].scale.x, trees[i].scale.z, trees[i].scale.y));
				tf->RotateRollPitchYaw(XMFLOAT3(0, trees[i].yaw, 0));
				tf->UpdateTransform();
				object->parentObject = entity;
			}
		}
		for (size_t i = 0; i < torches.size(); i++) {
			torch_t torch = torches[i];
			wiECS::Entity lightEnt;
			LightComponent* light = nullptr;
			wiECS::Entity meshID = 0;
			TransformComponent transform;
			XMFLOAT3 color((float)cyBlocks::m_regBlockFlags[torch.ID][0] / 255.f, (float)cyBlocks::m_regBlockFlags[torch.ID][1] / 255.f, (float)cyBlocks::m_regBlockFlags[torch.ID][2] / 255.f);
			try {
				switch (torch.rotation) {
					case 0:
						meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
						transform.Translate(XMFLOAT3(relX + torch.x / 2.0f + 0.248, relZ + torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f));
						transform.RotateRollPitchYaw(XMFLOAT3(0, PI / 2, 0));
						lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f + 0.1, relZ + torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f), color, 5, 4);
						light	 = tmpScene.lights.GetComponent(lightEnt);
						break;
					case 1:
						meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
						transform.Translate(XMFLOAT3(relX + torch.x / 2.0f - 0.248, relZ + torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f));
						transform.RotateRollPitchYaw(XMFLOAT3(0, -PI / 2, 0));
						lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f - 0.1, relZ + torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f), color, 5, 4);
						light	 = tmpScene.lights.GetComponent(lightEnt);
						break;
					case 2:
						meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
						transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, relZ + torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f + 0.248));
						lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f, relZ + torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f + 0.1), color, 5, 4);
						light	 = tmpScene.lights.GetComponent(lightEnt);
						break;
					case 3:
						meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
						transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, relZ + torch.z / 2.0f - 0.14, relY + 16 - torch.y / 2.0f - 0.248));
						transform.RotateRollPitchYaw(XMFLOAT3(0, PI, 0));
						lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f, relZ + torch.z / 2.0f + 0.02, relY + 16 - torch.y / 2.0f - 0.1), color, 5, 4);
						light	 = tmpScene.lights.GetComponent(lightEnt);
						break;
					case 4:
						meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[1];
						transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, relZ + torch.z / 2.0f - 0.135f, relY + 16 - torch.y / 2.0f));
						lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f, relZ + torch.z / 2.0f + 0.08, relY + 16 - torch.y / 2.0f), color, 5, 4);
						light	 = tmpScene.lights.GetComponent(lightEnt);
						break;
					default:
						meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[1];
						transform.Translate(XMFLOAT3(relX + torch.x / 2.0f, relZ + torch.z / 2.0f + 0.135f, relY + 16 - torch.y / 2.0f));
						transform.RotateRollPitchYaw(XMFLOAT3(PI, 0, 0));
						lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(relX + torch.x / 2.0f, relZ + torch.z / 2.0f - 0.08, relY + 16 - torch.y / 2.0f), XMFLOAT3(1.0, 0.3, 0.0f), 5, 4);
						light	 = tmpScene.lights.GetComponent(lightEnt);
						break;
				}
				if (light != nullptr) {
					layer			 = tmpScene.layers.GetComponent(lightEnt);
					layer->layerMask = LAYER_SCHEMATIC;
					groupChilds.push_back(lightEnt);
					light->SetCastShadow(true);
					if (settings::torchlights == false) {
						light->SetStatic(true);
					}
					objEnt = tmpScene.Entity_CreateObject("torch" + to_string(lightEnt));
					groupChilds.push_back(objEnt);
					object = tmpScene.objects.GetComponent(objEnt);
					layer	= tmpScene.layers.GetComponent(objEnt);
					layer->layerMask		= LAYER_SCHEMATIC;
					tf	= tmpScene.transforms.GetComponent(objEnt);
					object->meshID			= meshID;
					object->SetUserStencilRef(chunkLoader::STENCIL_HIGHLIGHT_OBJ);
					object->emissiveColor = XMFLOAT4(color.x, color.y, color.z, 1.0f);
					*tf					 = transform;
					tf->UpdateTransform();
					object->parentObject = entity;
					light->parentObject = entity;
				}
			}
			catch (...) {
			}
		}
		for (size_t i = 0; i < groupChilds.size(); i++) {
			tmpScene.Component_Detach(groupChilds[i]);
			tmpScene.Component_Attach(groupChilds[i], entity);
		}

		m.lock();
		wiScene::GetScene().Merge(tmpScene);
		m.unlock();
	}  //else	wiBackLog::post("Chunk has no blocks");
	return entity;
}
