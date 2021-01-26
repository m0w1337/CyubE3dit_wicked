#include "stdafx.h"
#include "cySchematic.h"
#include <fstream>

using namespace std;
using namespace wiScene;
extern mutex m;
std::vector<cySchematic*> cySchematic::m_schematics;

cySchematic::cySchematic(string filename) {
	wiScene::Scene tmpScene;
	ifstream file;
	file.open(filename, fstream::in | ios::binary);
	m_chunkdata = nullptr;
	if (file.is_open()) {
		blockpos_t bsize;
		file.read(reinterpret_cast<char*>(&bsize.x), sizeof(bsize.x));
		file.read(reinterpret_cast<char*>(&bsize.y), sizeof(bsize.y));
		file.read(reinterpret_cast<char*>(&bsize.z), sizeof(bsize.z));
		size.x = bsize.x / 2.f;
		size.y = bsize.y / 2.f;
		size.z = bsize.z / 2.f;

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
			size_t offset  = bsize.x * bsize.y * bsize.z;
			memcpy(&dSize, m_chunkdata + offset, 4);
			offset += 4;
			blockpos_t bpos;
			uint32_t cID = 0;
			while (dSize) {
				memcpy(&(bpos.x), m_chunkdata + offset, 2);
				memcpy(&(bpos.y), m_chunkdata + offset + 2, 2);
				memcpy(&(bpos.z), m_chunkdata + offset + 4, 2);
				memcpy(&(cID), m_chunkdata + offset + 6, 4);
				offset += 10;
				m_cBlocks[bpos] = cID;
				dSize--;
			}

			memcpy(&dSize, m_chunkdata + offset, 4);
			offset += 4;
			uint8_t rot = 0;
			while (dSize) {
				memcpy(&(bpos.x), m_chunkdata + offset, 2);
				memcpy(&(bpos.y), m_chunkdata + offset + 2, 2);
				memcpy(&(bpos.z), m_chunkdata + offset + 4, 2);
				memcpy(&(rot), m_chunkdata + offset + 6, 1);
				offset += 7;
				m_Torches[bpos] = rot;
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
	for (size_t i = 0; i < HOVER_NUMELEMENTS; i++) {
		hoverEntities[i].entity = wiECS::INVALID_ENTITY;
	}
}
/*
cySchematic& cySchematic::getSchematic(wiECS::Entity antity) {
	
}*/

void cySchematic::addSchematic(std::string filename) {
	cySchematic* schem = new cySchematic(filename);
	schem->RenderSchematic(0, 0, 10);
	m_schematics.push_back(schem);
}

cySchematic::hovertype_t cySchematic::hoverGizmo(const wiECS::Entity entity) {
	hovertype_t ret = HOVER_NONE;
	for (size_t i = 0; i < HOVER_NUMELEMENTS; i++) {
		if (hoverEntities[i].entity != wiECS::INVALID_ENTITY) {

			if (hoverEntities[i].entity == entity) {
				wiScene::GetScene().objects.GetComponent(entity)->color = hoverEntities[i].hovercolor;
				ret														= (hovertype_t)i;
			} else {
				wiScene::GetScene().objects.GetComponent(hoverEntities[i].entity)->color = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.f);
			}
		}
	}
	if (ret == HOVER_ROTCC || ret == HOVER_ROTCW) {	 //Disable drag on rotation gizmos
		ret = HOVER_NONE;
	}
	return ret;
}

void cySchematic::RenderSchematic(const float relX, const float relY, const float relZ) {
	chunkLoader::face_t tmpface;
	vector<chunkLoader::face_t> faces;
	vector<torch_t> torches;
	uint8_t stepsize = 1;
	pos.x			 = relX;
	pos.y			 = relY;
	pos.z			 = relZ;
	blockpos_t bsize(size.x * 2, size.y * 2, size.z * 2);
	if (m_chunkdata == nullptr) {
		mainEntity = wiECS::INVALID_ENTITY;
		return;
	}

	wiScene::Scene tmpScene;
	for (int_fast16_t z = 0; z < bsize.z; z++) {
		for (uint_fast16_t x = 0; x < bsize.x; x++) {
			for (uint_fast16_t y = 0; y < bsize.y; y++) {
				uint8_t blocktype = (uint8_t) * (m_chunkdata + x + bsize.x * y + bsize.x * bsize.y * z);
				if (cyBlocks::m_regBlockTypes[blocktype] <= cyBlocks::BLOCKTYPE_ALPHA) {
					uint8_t neighbour[6];
					if (z < bsize.z - 1)
						neighbour[0] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata + x + bsize.x * y + bsize.x * bsize.y * (z + 1))];
					else
						neighbour[0] = cyBlocks::BLOCKTYPE_VOID;

					if (z > 1)
						neighbour[1] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata + x + bsize.x * y + bsize.x * bsize.y * (z - 1))];
					else
						neighbour[1] = cyBlocks::BLOCKTYPE_VOID;

					if (y < bsize.y - 1)
						neighbour[4] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata + x + bsize.x * (y + 1) + bsize.x * bsize.y * z)];
					else
						neighbour[4] = cyBlocks::BLOCKTYPE_VOID;

					if (y >= 1)
						neighbour[5] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata + x + bsize.x * (y - 1) + bsize.x * bsize.y * z)];
					else
						neighbour[5] = cyBlocks::BLOCKTYPE_VOID;

					if (x < bsize.x - 1)
						neighbour[3] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata + 1 + x + bsize.x * y + bsize.x * bsize.y * z)];
					else
						neighbour[3] = cyBlocks::BLOCKTYPE_VOID;

					if (x >= 1)
						neighbour[2] = cyBlocks::m_regBlockTypes[(uint8_t) * (m_chunkdata - 1 + x + bsize.x * y + bsize.x * bsize.y * z)];
					else
						neighbour[2] = cyBlocks::BLOCKTYPE_VOID;

					uint8_t antitile = 0;

					if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_MOD) {
						blockpos_t bpos(x, y, z);
						std::unordered_map<blockpos_t, uint32_t>::const_iterator cBlock = m_cBlocks.find(bpos);
						if (cBlock != m_cBlocks.end()) {
							for (uint8_t ft = 0; ft < 6; ft++) {
								if (neighbour[ft] > cyBlocks::BLOCKTYPE_SOLID_THRESH) {
									tmpface.x		 = x / 2.0f;
									tmpface.y		 = size.y - y / 2.0f;
									tmpface.z		 = z / 2.0f;
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
								tmpface.x		 = x / 2.0f;
								tmpface.y		 = size.y - y / 2.0f;
								tmpface.z		 = z / 2.0f;
								tmpface.antitile = cyBlocks::m_regBlockFlags[blocktype][ft] & cyBlocks::BLOCKFLAG_ANTITILE;
								tmpface.face	 = ft;
								tmpface.material = cyBlocks::m_regBlockMats[blocktype][ft];
								faces.emplace_back(tmpface);
							}
						}
					}
				} else if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_BILLBOARD) {
					tmpface.x		 = x / 2.0f;
					tmpface.y		 = size.y - y / 2.0f;
					tmpface.z		 = z / 2.0f;
					tmpface.material = cyBlocks::m_regBlockMats[blocktype][0];
					tmpface.face	 = cyBlocks::FACE_BILLBOARD;
					faces.emplace_back(tmpface);
				} else if (cyBlocks::m_regBlockTypes[blocktype] == cyBlocks::BLOCKTYPE_TORCH) {
					blockpos_t bpos(x, y, z);
					auto it = m_Torches.find(bpos);
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
		if (wiScene::GetScene().materials.GetComponent(faces[0].material) != nullptr) {
			mesh = meshGen::AddMesh(tmpScene, "Schematic", faces[0].material, &mainEntity);
		} else {
			mesh = meshGen::AddMesh(tmpScene, "Schematic", cyBlocks::m_fallbackMat, &mainEntity);
		}
		tmpScene.objects.GetComponent(mainEntity)->SetUserStencilRef(chunkLoader::STENCIL_HIGHLIGHT_OBJ);
		LayerComponent* layer = tmpScene.layers.GetComponent(mainEntity);
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

		for (size_t i = 0; i < trees.size(); i++) {
			if (trees[i].scale.x > 2 || trees[i].scale.y > 2 || trees[i].scale.z > 2) {
				wiHelper::messageBox("Tree scaling out of bounds!", "Error!");
				wiBackLog::post("Weird tree data: in Schematic.");
			} else {
				objEnt = tmpScene.Entity_CreateObject("tree");
				object = tmpScene.objects.GetComponent(objEnt);
				object->SetUserStencilRef(chunkLoader::STENCIL_HIGHLIGHT_OBJ);
				layer			 = tmpScene.layers.GetComponent(objEnt);
				layer->layerMask = LAYER_SCHEMATIC;
				tf				 = tmpScene.transforms.GetComponent(objEnt);
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
				tf->Translate(XMFLOAT3(relX + trees[i].pos.x / 2.0f, relZ + trees[i].pos.z / 2.0f - 0.25, relY + size.y - trees[i].pos.y / 2.0f));
				tf->Scale(XMFLOAT3(trees[i].scale.x, trees[i].scale.z, trees[i].scale.y));
				tf->RotateRollPitchYaw(XMFLOAT3(0, trees[i].yaw, 0));
				tf->UpdateTransform();
				object->parentObject = mainEntity;
				tmpScene.Component_Attach(objEnt, mainEntity);
			}
		}
		placeTorches(torches, tmpScene);
		attachGizmos();
		tf = tmpScene.transforms.GetComponent(mainEntity);
		tf->Translate(XMFLOAT3(relX, relZ, relY));
		tf->UpdateTransform();
		m.lock();
		wiScene::GetScene().Merge(tmpScene);
		m.unlock();
	}  //else	wiBackLog::post("Chunk has no blocks");
}

inline void cySchematic::placeTorches(const std::vector<torch_t>& torches, Scene& tmpScene) {
	for (size_t i = 0; i < torches.size(); i++) {
		torch_t torch = torches[i];
		wiECS::Entity lightEnt;
		LightComponent* light = nullptr;
		wiECS::Entity meshID  = 0;
		TransformComponent transform;
		XMFLOAT3 color((float)cyBlocks::m_regBlockFlags[torch.ID][0] / 255.f, (float)cyBlocks::m_regBlockFlags[torch.ID][1] / 255.f, (float)cyBlocks::m_regBlockFlags[torch.ID][2] / 255.f);
		try {
			switch (torch.rotation) {
				case 0:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(torch.x / 2.0f + 0.248, torch.z / 2.0f - 0.14, size.y - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(0, PI / 2, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f + 0.1, torch.z / 2.0f + 0.02, size.y - torch.y / 2.0f), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 1:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(torch.x / 2.0f - 0.248, torch.z / 2.0f - 0.14, size.y - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(0, -PI / 2, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f - 0.1, torch.z / 2.0f + 0.02, size.y - torch.y / 2.0f), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 2:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f - 0.14, size.y - torch.y / 2.0f + 0.248));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f + 0.02, size.y - torch.y / 2.0f + 0.1), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 3:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f - 0.14, size.y - torch.y / 2.0f - 0.248));
					transform.RotateRollPitchYaw(XMFLOAT3(0, PI, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f + 0.02, size.y - torch.y / 2.0f - 0.1), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 4:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[1];
					transform.Translate(XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f - 0.135f, size.y - torch.y / 2.0f));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f + 0.08, size.y - torch.y / 2.0f), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				default:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[1];
					transform.Translate(XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f + 0.135f, size.y - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(PI, 0, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f - 0.08, size.y - torch.y / 2.0f), XMFLOAT3(1.0, 0.3, 0.0f), 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
			}
			if (light != nullptr) {
				wiScene::LayerComponent* layer = tmpScene.layers.GetComponent(lightEnt);
				layer->layerMask			   = LAYER_SCHEMATIC;
				light->SetCastShadow(true);
				if (settings::torchlights == false) {
					light->SetStatic(true);
				}
				light->parentObject = mainEntity;
				tmpScene.Component_Attach(lightEnt, mainEntity);
				wiECS::Entity objEnt			 = tmpScene.Entity_CreateObject("torch" + to_string(lightEnt));
				wiScene::ObjectComponent* object = tmpScene.objects.GetComponent(objEnt);
				layer							 = tmpScene.layers.GetComponent(objEnt);
				layer->layerMask				 = LAYER_SCHEMATIC;
				wiScene::TransformComponent* tf	 = tmpScene.transforms.GetComponent(objEnt);
				object->meshID					 = meshID;
				object->SetUserStencilRef(chunkLoader::STENCIL_HIGHLIGHT_OBJ);
				object->emissiveColor = XMFLOAT4(color.x, color.y, color.z, 1.0f);
				*tf					  = transform;
				tf->UpdateTransform();
				object->parentObject = mainEntity;
				tmpScene.Component_Attach(objEnt, mainEntity);
			}
		}
		catch (...) {
		}
	}
}

void cySchematic::attachGizmos(wiScene::Scene& tmpScene) {
	float arrowsize				   = (size.x + size.y) / 100.0f;
	wiECS::Entity objEnt		   = tmpScene.Entity_CreateObject("cwArrow");
	ObjectComponent* object		   = tmpScene.objects.GetComponent(objEnt);
	TransformComponent* tf		   = tmpScene.transforms.GetComponent(objEnt);
	wiScene::LayerComponent* layer = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask			   = LAYER_SCHEMATIC;
	object->meshID				   = cyBlocks::m_toolMeshes[0];
	object->parentObject		   = mainEntity;
	object->color				   = XMFLOAT4(0.5, 0.5, 0.5, 1.0);
	tf->Translate(XMFLOAT3(5 + size.x / 2, 3 + size.z, size.y / 2));
	tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
	tf->RotateRollPitchYaw(XMFLOAT3(0, 0, 0));
	tf->UpdateTransform();
	hoverEntities[HOVER_ROTCW].entity	  = objEnt;
	hoverEntities[HOVER_ROTCW].hovercolor = XMFLOAT4(0.8f, 0.1f, 0.0f, 1.f);
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt				 = tmpScene.Entity_CreateObject("ccArrow");
	object				 = tmpScene.objects.GetComponent(objEnt);
	layer				 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask	 = LAYER_SCHEMATIC;
	tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_ROT];
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.5, 0.5, 0.5, 1.0);
	tf->Translate(XMFLOAT3(5 + size.x / 2, -3, size.y / 2));
	tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
	tf->RotateRollPitchYaw(XMFLOAT3(PI, 0, 0));
	tf->UpdateTransform();
	hoverEntities[HOVER_ROTCC].entity	  = objEnt;
	hoverEntities[HOVER_ROTCC].hovercolor = XMFLOAT4(0.8f, 0.1f, 0.0f, 1.f);
	tmpScene.Component_Attach(objEnt, mainEntity);

	arrowsize			 = (size.x + size.y + size.z) / 75.0f;
	objEnt				 = tmpScene.Entity_CreateObject("Origin");
	object				 = tmpScene.objects.GetComponent(objEnt);
	layer				 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask	 = LAYER_SCHEMATIC;
	tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_ORIGIN];
	object->parentObject = mainEntity;
	tf->Translate(XMFLOAT3(-0.75, -0.75, -0.25));
	tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
	tf->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
	tf->UpdateTransform();
	hoverEntities[HOVER_ORIGIN].entity	   = objEnt;
	hoverEntities[HOVER_ORIGIN].hovercolor = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.f);
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt				 = tmpScene.Entity_CreateObject("xAxis");
	object				 = tmpScene.objects.GetComponent(objEnt);
	layer				 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask	 = LAYER_SCHEMATIC;
	tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_XAXIS];
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.5, 0.5, 0.5, 1.0);
	tf->Translate(XMFLOAT3(-0.75, -0.75, -0.25));
	tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
	tf->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
	tf->UpdateTransform();
	hoverEntities[HOVER_X_AXIS].entity	   = objEnt;
	hoverEntities[HOVER_X_AXIS].hovercolor = XMFLOAT4(0.0f, 0.4f, 0.8f, 1.f);
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt				 = tmpScene.Entity_CreateObject("yAxis");
	object				 = tmpScene.objects.GetComponent(objEnt);
	layer				 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask	 = LAYER_SCHEMATIC;
	tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_YAXIS];
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.5, 0.5, 0.5, 1.0);
	tf->Translate(XMFLOAT3(-0.75, -0.75, -0.25));
	tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
	tf->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
	tf->UpdateTransform();
	hoverEntities[HOVER_Y_AXIS].entity	   = objEnt;
	hoverEntities[HOVER_Y_AXIS].hovercolor = XMFLOAT4(0.5f, 0.8f, 0.0f, 1.f);
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt				 = tmpScene.Entity_CreateObject("zAxis");
	object				 = tmpScene.objects.GetComponent(objEnt);
	layer				 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask	 = LAYER_SCHEMATIC;
	tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_ZAXIS];
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.5, 0.5, 0.5, 1.0);
	tf->Translate(XMFLOAT3(-0.75, -0.75, -0.25));
	tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
	tf->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
	tf->UpdateTransform();
	hoverEntities[HOVER_Z_AXIS].entity	   = objEnt;
	hoverEntities[HOVER_Z_AXIS].hovercolor = XMFLOAT4(0.8f, 0.1f, 0.0f, 1.f);
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt				 = tmpScene.Entity_CreateObject("planeXY");
	object				 = tmpScene.objects.GetComponent(objEnt);
	layer				 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask	 = LAYER_SCHEMATIC;
	tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_PLANE];
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.5, 0.5, 0.5, 1.0);
	tf->Translate(XMFLOAT3(-0.25, -0.25, 0.3));
	tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
	tf->RotateRollPitchYaw(XMFLOAT3(0, 0, 0));
	tf->UpdateTransform();
	hoverEntities[HOVER_XZ_PLANE].entity	 = objEnt;
	hoverEntities[HOVER_XZ_PLANE].hovercolor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.f);
	tmpScene.Component_Attach(objEnt, mainEntity);
}
