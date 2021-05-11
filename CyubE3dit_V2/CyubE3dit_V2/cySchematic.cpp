#include "stdafx.h"
#include "cySchematic.h"
#include <fstream>

using namespace std;
using namespace wiScene;
extern mutex m;
bool cySchematic::updating = false;
std::vector<cySchematic*> cySchematic::m_schematics;

cySchematic::cySchematic(string filename) :
  m_dirty(DIRTY_NOTRENDERED) {
	wiScene::Scene tmpScene;
	ifstream file;
	if (filename.size()) {
		file.open(filename, fstream::in | ios::binary);
	}
	m_chunkdata = nullptr;
	if (file.is_open()) {
		blockpos_t bsize;
		file.read(reinterpret_cast<char*>(&bsize.x), sizeof(bsize.x));
		file.read(reinterpret_cast<char*>(&bsize.y), sizeof(bsize.y));
		file.read(reinterpret_cast<char*>(&bsize.z), sizeof(bsize.z));
		size.x = bsize.x / 2.f;
		size.y = bsize.y / 2.f;
		size.z = bsize.z / 2.f;
		size.w = (size.x + size.y + size.z) * 0.334f;

		uint32_t uncompressedSize = 0;
		file.seekg(0, ios_base::end);
		int flength = file.tellg();
		file.seekg(flength - 4, ios_base::beg);
		file.read(reinterpret_cast<char*>(&uncompressedSize), sizeof(uncompressedSize));
		m_chunkdata		 = (uint8_t*)malloc(uncompressedSize);
		char* compressed = (char*)malloc(flength - HEADER_SIZE);
		file.seekg(12, ios_base::beg);
		uint32_t magic;
		file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
		if (magic == 0x13371337) {
			file.read(compressed, flength - HEADER_SIZE);
			try {
				LZ4_decompress_fast(compressed, reinterpret_cast<char*>(m_chunkdata), uncompressedSize);
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
				m_torches[bpos] = rot;
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

void cySchematic::addBoxSelector(void) {
	cySchematic* schem = new cySchematic("");
	wiScene::Scene tmpScene;
	schem->size.x = 10;
	schem->size.y = 10;
	schem->size.z = 10;
	schem->size.w = 10;
	schem->positionBeforeCam();

	wiScene::MeshComponent* mesh;

	mesh				  = meshGen::AddMesh(tmpScene, "Schematic", cyBlocks::toolblock_material, &schem->mainEntity);
	LayerComponent* layer = tmpScene.layers.GetComponent(schem->mainEntity);
	layer->layerMask	  = LAYER_SCHEMATIC | LAYER_GIZMO;
	mesh->SetDoubleSided(true);
	float boxOffset = 4.75f;
	schem->attachGizmos(tmpScene, true);
	wiScene::TransformComponent* tf = tmpScene.transforms.GetComponent(schem->mainEntity);
	tf->Translate(XMFLOAT3(schem->pos.x, schem->pos.z, schem->pos.y));
	tf->UpdateTransform();
	wiScene::GetScene().Merge(tmpScene);
	schem->resizeGizmos();
	schem->type	   = TYPE_SELECTION;
	schem->m_dirty = DIRTY_RESIZE;	//Make sure the underlying chunks are updated accordingly
	m_schematics.push_back(schem);
}

void cySchematic::clearAllSchematics(void) {
	for (size_t i = 0; i < m_schematics.size(); i++) {
		wiScene::Scene& scene		  = wiScene::GetScene();
		wiScene::ObjectComponent* obj = scene.objects.GetComponent(m_schematics[i]->mainEntity);
		wiECS::Entity meshEnt		  = obj->meshID;
		m_schematics[i]->m_dirty	  = DIRTY_NOTRENDERED;
		scene.Component_RemoveChildren(m_schematics[i]->mainEntity);
		scene.Entity_Remove(m_schematics[i]->mainEntity);
		scene.Entity_Remove(meshEnt);
		delete m_schematics[i];
		chunkLoader::clearMaskedChunk();
		for (size_t ii = 0; ii < m_schematics[i]->m_chunkPreviews.size(); ii++) {
			wiScene::ObjectComponent* obj = scene.objects.GetComponent(m_schematics[i]->m_chunkPreviews[ii].chunkObj);
			if (obj != nullptr) {
				wiECS::Entity meshEnt = obj->meshID;
				scene.Component_RemoveChildren(m_schematics[i]->m_chunkPreviews[ii].chunkObj);
				scene.Entity_Remove(m_schematics[i]->m_chunkPreviews[ii].chunkObj);
				scene.Entity_Remove(meshEnt);
			}
		}
	}
	m_schematics.clear();
}

void cySchematic::clearSchematic(void) {
	wiScene::Scene& scene		  = wiScene::GetScene();
	wiScene::ObjectComponent* obj = scene.objects.GetComponent(mainEntity);
	wiECS::Entity meshEnt		  = obj->meshID;
	m_dirty						  = DIRTY_NOTRENDERED;
	scene.Component_RemoveChildren(mainEntity);
	scene.Entity_Remove(mainEntity);
	scene.Entity_Remove(meshEnt);
	for (size_t ii = 0; ii < m_chunkPreviews.size(); ii++) {
		wiScene::ObjectComponent* obj = scene.objects.GetComponent(m_chunkPreviews[ii].chunkObj);
		if (obj != nullptr) {
			wiECS::Entity meshEnt = obj->meshID;
			scene.Component_RemoveChildren(m_chunkPreviews[ii].chunkObj);
			scene.Entity_Remove(m_chunkPreviews[ii].chunkObj);
			scene.Entity_Remove(meshEnt);
		}
	}
}

void cySchematic::unRenderSchematic(void) {
	wiScene::Scene& scene		  = wiScene::GetScene();
	wiScene::ObjectComponent* obj = scene.objects.GetComponent(mainEntity);
	wiECS::Entity meshEnt		  = obj->meshID;
	m_dirty						  = DIRTY_NOTRENDERED;
	scene.Component_RemoveChildren(mainEntity);
	scene.Entity_Remove(mainEntity);
	scene.Entity_Remove(meshEnt);

	chunkLoader::clearMaskedChunk();
	for (size_t ii = 0; ii < m_chunkPreviews.size(); ii++) {
		wiScene::ObjectComponent* obj = scene.objects.GetComponent(m_chunkPreviews[ii].chunkObj);
		if (obj != nullptr) {
			wiECS::Entity meshEnt = obj->meshID;
			//m.lock();
			scene.Component_RemoveChildren(m_chunkPreviews[ii].chunkObj);
			scene.Entity_Remove(m_chunkPreviews[ii].chunkObj);
			scene.Entity_Remove(meshEnt);
			//m.unlock();
		}
	}
}

void cySchematic::positionBeforeCam(void) {
	CameraComponent cam = wiScene::GetCamera();
	XMFLOAT3 schempos	= cam.Eye;
	XMFLOAT3 camlook	= cam.At;

	pos.x = ceilf(schempos.x + camlook.x * size.x * 1.3f);
	pos.y = ceilf(schempos.z + camlook.z * size.y * 1.3f);
	pos.z = ceilf(schempos.y + camlook.y * size.z * 1.3f);
	pos.x -= ceilf(size.x) * 0.5f;
	pos.z -= ceilf(size.z) * 0.5f;
	pos.y -= ceilf(size.y) * 0.5f;
}

void cySchematic::addSchematic(std::string filename) {
	cySchematic* schem = new cySchematic(filename);
	std::vector<wiECS::Entity> affected;
	schem->positionBeforeCam();

	if (schem->pos.z < 0.5f) {
		schem->pos.z = 0.5f;
	} else if (schem->pos.z + schem->size.z > 399.5f) {
		schem->pos.z = 399.5f - schem->size.z;
	}

	schem->RenderSchematic();
	//schem->getAffectedChunks(affected);
	schem->m_dirty = DIRTY_DRAG;  //Make sure the underlying chunks are updated accordingly
	schem->type	   = TYPE_SCHEM_V1;
	m_schematics.push_back(schem);
}

void cySchematic::updateDirtyPreviews(void) {
	for (size_t i = 0; i < m_schematics.size(); i++) {
		if (m_schematics[i]->m_dirty > DIRTY_NOTRENDERED) {
			switch (m_schematics[i]->m_dirty) {
				case DIRTY_DRAG:
					if (m_schematics[i]->type != TYPE_SELECTION && m_schematics[i]->size.x * m_schematics[i]->size.y < (5 * 5 * 16 * 16)) {	 //only perform the live preview for reasonable sized schematics (less that 25square chunks

						m_schematics[i]->showGizmos(false);
						m_schematics[i]->generateChunkPreview();
						m_schematics[i]->showGizmos(true);

					} else if (m_schematics[i]->type == TYPE_SELECTION) {
						m_schematics[i]->resizeGizmos();
					}
					m_schematics[i]->m_dirty = NOT_DIRTY;
					break;
				case DIRTY_ROTCC:
					m_schematics[i]->rotate(true);
					m_schematics[i]->m_dirty = NOT_DIRTY;
					break;
				case DIRTY_ROTCW:
					m_schematics[i]->rotate(false);
					m_schematics[i]->m_dirty = NOT_DIRTY;
					break;
				case DIRTY_SAVE:
					if (m_schematics[i]->type != TYPE_SELECTION) {
						m_schematics[i]->saveToWorld();
					} else {
						m_schematics[i]->m_dirty = DIRTY_NOTRENDERED;
						m_schematics[i]->showGizmos(false);
						m_schematics[i]->saveToMem();
						m_schematics[i]->showGizmos(true);
					}
					break;
				case DIRTY_RESIZE:
					m_schematics[i]->m_dirty = DIRTY_NOTRENDERED;
					m.lock();
					meshGen::ResizeToolBlock(m_schematics[i]->size.x, m_schematics[i]->size.z, m_schematics[i]->size.y);
					m.unlock();
					m_schematics[i]->m_dirty = NOT_DIRTY;
					break;
				case DIRTY_REMOVE:
					chunkLoader::clearMaskedChunk();
					m.lock();
					m_schematics[i]->clearSchematic();
					delete m_schematics[i];
					m_schematics.erase(m_schematics.begin() + i);
					m.unlock();
					break;
				default:
					break;
			}
		}
	}
	updating = false;
}

cySchematic::hovertype_t cySchematic::hoverGizmo(const wiECS::Entity entity) {
	hovertype_t ret = HOVER_NONE;
	m_activeGizmo	= HOVER_NONE;
	if (m_dirty == DIRTY_NOTRENDERED)
		return ret;

	for (size_t i = 0; i < HOVER_NUMELEMENTS; i++) {
		if (hoverEntities[i].entity != wiECS::INVALID_ENTITY) {
			if (hoverEntities[i].entity == entity) {
				wiScene::GetScene().objects.GetComponent(entity)->color			= hoverEntities[i].hovercolor;
				wiScene::GetScene().objects.GetComponent(entity)->emissiveColor = hoverEntities[i].hovercolor;
				m_activeGizmo													= i;
				ret																= (hovertype_t)i;
			} else if (wiScene::GetScene().objects.GetComponent(hoverEntities[i].entity) != nullptr) {
				wiScene::GetScene().objects.GetComponent(hoverEntities[i].entity)->color		 = hoverEntities[i].nohovercolor;
				wiScene::GetScene().objects.GetComponent(hoverEntities[i].entity)->emissiveColor = XMFLOAT4(0, 0, 0, 0);
			}
		}
	}
	return ret;
}

void cySchematic::RenderSchematic(void) {
	wiScene::Scene tmpScene;
	prepareSchematic(tmpScene);
	m.lock();
	wiScene::GetScene().Merge(tmpScene);
	resizeGizmos();
	m_dirty = NOT_DIRTY;
	m.unlock();
};

void cySchematic::prepareSchematic(wiScene::Scene& tmpScene) {
	chunkLoader::face_t tmpface;
	vector<chunkLoader::face_t> faces;
	vector<torch_t> torches;

	blockpos_t bsize(size.x * 2, size.y * 2, size.z * 2);
	if (m_chunkdata == nullptr) {
		mainEntity = wiECS::INVALID_ENTITY;
		return;
	}

	for (int_fast16_t z = 0; z < bsize.z; z++) {
		for (uint_fast16_t x = 0; x < bsize.x; x++) {
			for (uint_fast16_t y = 0; y < bsize.y; y++) {
				uint8_t blocktype = (uint8_t) * (m_chunkdata + x + bsize.x * y + bsize.x * bsize.y * z);
				if (cyBlocks::m_regBlockTypes[blocktype] <= cyBlocks::BLOCKTYPE_ALPHA) {
					uint8_t neighbour[6];
					if (z < bsize.z - 1)
						neighbour[0] = cyBlocks::m_regBlockTypes[*(m_chunkdata + x + bsize.x * y + bsize.x * bsize.y * (z + 1))];
					else
						neighbour[0] = cyBlocks::BLOCKTYPE_VOID;

					if (z > 1)
						neighbour[1] = cyBlocks::m_regBlockTypes[*(m_chunkdata + x + bsize.x * y + bsize.x * bsize.y * (z - 1))];
					else
						neighbour[1] = cyBlocks::BLOCKTYPE_VOID;

					if (y < bsize.y - 1)
						neighbour[4] = cyBlocks::m_regBlockTypes[*(m_chunkdata + x + bsize.x * (y + 1) + bsize.x * bsize.y * z)];
					else
						neighbour[4] = cyBlocks::BLOCKTYPE_VOID;

					if (y >= 1)
						neighbour[5] = cyBlocks::m_regBlockTypes[*(m_chunkdata + x + bsize.x * (y - 1) + bsize.x * bsize.y * z)];
					else
						neighbour[5] = cyBlocks::BLOCKTYPE_VOID;

					if (x < bsize.x - 1)
						neighbour[3] = cyBlocks::m_regBlockTypes[*(m_chunkdata + 1 + x + bsize.x * y + bsize.x * bsize.y * z)];
					else
						neighbour[3] = cyBlocks::BLOCKTYPE_VOID;

					if (x >= 1)
						neighbour[2] = cyBlocks::m_regBlockTypes[*(m_chunkdata - 1 + x + bsize.x * y + bsize.x * bsize.y * z)];
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
					auto it = m_torches.find(bpos);
					if (it != m_torches.end()) {
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
		//tmpScene.objects.GetComponent(mainEntity)->SetUserStencilRef(chunkLoader::STENCIL_HIGHLIGHT_OBJ);
		LayerComponent* layer = tmpScene.layers.GetComponent(mainEntity);
		layer->layerMask	  = LAYER_SCHEMATIC | LAYER_GIZMO;
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
					meshGen::AddFaceTop(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
					break;
				case cyBlocks::FACE_BOTTOM:
					meshGen::AddFaceBottom(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
					break;
				case cyBlocks::FACE_LEFT:
					meshGen::AddFaceLeft(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
					break;
				case cyBlocks::FACE_RIGHT:
					meshGen::AddFaceRight(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
					break;
				case cyBlocks::FACE_BACK:
					meshGen::AddFaceBack(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
					break;
				case cyBlocks::FACE_FRONT:
					meshGen::AddFaceFront(mesh, faces[i].x, faces[i].y, faces[i].z, faces[i].antitile);
					break;
				case cyBlocks::FACE_BILLBOARD:
					meshGen::AddBillboard(mesh, faces[i].x, faces[i].y, faces[i].z);
					break;
			}
		}
		mesh->subsets.back().indexCount = (uint32_t)mesh->indices.size() - mesh->subsets.back().indexOffset;

		mesh->SetDynamic(false);
		mesh->CreateRenderData();
		wiScene::TransformComponent* tf;
		wiScene::ObjectComponent* object;
		wiECS::Entity objEnt;
		for (size_t i = 0; i < trees.size(); i++) {
			if (trees[i].scale.x > 2 || trees[i].scale.y > 2 || trees[i].scale.z > 2) {
				wiHelper::messageBox("Tree scaling out of bounds!", "Error!");
				wiBackLog::post("Weird tree data: in Schematic.");
			} else {
				objEnt			 = tmpScene.Entity_CreateObject("tree");
				object			 = tmpScene.objects.GetComponent(objEnt);
				layer			 = tmpScene.layers.GetComponent(objEnt);
				layer->layerMask = LAYER_SCHEMATIC | LAYER_GIZMO;
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
				tf->Translate(XMFLOAT3(pos.x + trees[i].pos.x / 2.0f, pos.z + trees[i].pos.z / 2.0f - 0.25, pos.y + size.y - trees[i].pos.y / 2.0f));
				tf->Scale(XMFLOAT3(trees[i].scale.x, trees[i].scale.z, trees[i].scale.y));
				tf->RotateRollPitchYaw(XMFLOAT3(0, trees[i].yaw, 0));
				tf->UpdateTransform();
				object->parentObject = mainEntity;
				tmpScene.Component_Attach(objEnt, mainEntity);
			}
		}
		placeTorches(torches, tmpScene);
		attachGizmos(tmpScene);
		tf = tmpScene.transforms.GetComponent(mainEntity);
		tf->Translate(XMFLOAT3(pos.x, pos.z, pos.y));
		tf->UpdateTransform();

	}  //else	wiBackLog::post("Chunk has no blocks");
}

void cySchematic::reposition(void) {
	for (size_t i = 0; i < m_schematics.size(); i++) {
		m_schematics[i]->positionBeforeCam();
		wiScene::TransformComponent* transform = wiScene::GetScene().transforms.GetComponent(cySchematic::m_schematics[i]->mainEntity);
		transform->ClearTransform();
		transform->Translate(XMFLOAT3(m_schematics[i]->pos.x, m_schematics[i]->pos.z, m_schematics[i]->pos.y));
		m_schematics[i]->m_dirty = DIRTY_DRAG;
	}
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
					transform.Translate(XMFLOAT3(torch.x / 2.0f + 0.248, torch.z / 2.0f - 0.14f, size.y - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(0, PI *0.5f, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f + 0.1f, torch.z / 2.0f + 0.02f, size.y - torch.y / 2.0f), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 1:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(torch.x / 2.0f - 0.248, torch.z / 2.0f - 0.14f, size.y - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(0, -PI * 0.5f, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f - 0.1, torch.z / 2.0f + 0.02f, size.y - torch.y / 2.0f), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 2:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f - 0.14f, size.y - torch.y / 2.0f + 0.248f));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f + 0.02f, size.y - torch.y / 2.0f + 0.1), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 3:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[0];
					transform.Translate(XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f - 0.14f, size.y - torch.y / 2.0f - 0.248f));
					transform.RotateRollPitchYaw(XMFLOAT3(0, PI, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f + 0.02f, size.y - torch.y / 2.0f - 0.1), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				case 4:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[1];
					transform.Translate(XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f - 0.135f, size.y - torch.y / 2.0f));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f + 0.08f, size.y - torch.y / 2.0f), color, 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
				default:
					meshID = cyBlocks::m_regMeshes.at(cyBlocks::m_torchID).mesh[1];
					transform.Translate(XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f + 0.135f, size.y - torch.y / 2.0f));
					transform.RotateRollPitchYaw(XMFLOAT3(PI, 0, 0));
					lightEnt = tmpScene.Entity_CreateLight("TL", XMFLOAT3(torch.x / 2.0f, torch.z / 2.0f - 0.08f, size.y - torch.y / 2.0f), XMFLOAT3(1.0f, 0.3f, 0.0f), 5, 4);
					light	 = tmpScene.lights.GetComponent(lightEnt);
					break;
			}
			if (light != nullptr) {
				wiScene::LayerComponent* layer = tmpScene.layers.GetComponent(lightEnt);
				layer->layerMask			   = LAYER_SCHEMATIC | LAYER_GIZMO;
				light->SetCastShadow(true);
				if (settings::torchlights == false) {
					light->SetStatic(true);
				}
				light->parentObject = mainEntity;
				tmpScene.Component_Attach(lightEnt, mainEntity);
				wiECS::Entity objEnt			 = tmpScene.Entity_CreateObject("torch" + to_string(lightEnt));
				wiScene::ObjectComponent* object = tmpScene.objects.GetComponent(objEnt);
				layer							 = tmpScene.layers.GetComponent(objEnt);
				layer->layerMask				 = LAYER_SCHEMATIC | LAYER_GIZMO;
				wiScene::TransformComponent* tf	 = tmpScene.transforms.GetComponent(objEnt);
				object->meshID					 = meshID;
				object->emissiveColor			 = XMFLOAT4(color.x, color.y, color.z, 1.0f);
				*tf								 = transform;
				tf->UpdateTransform();
				object->parentObject = mainEntity;
				tmpScene.Component_Attach(objEnt, mainEntity);
			}
		}
		catch (...) {
		}
	}
}

void cySchematic::attachGizmos(wiScene::Scene& tmpScene, const bool _toolblock) {
	float arrowsize = (size.x + size.y) / 75.0f;
	wiECS::Entity objEnt;
	ObjectComponent* object;
	TransformComponent* tf;
	wiScene::LayerComponent* layer;
	if (_toolblock) {
		objEnt = tmpScene.Entity_CreateObject("SizX");
		object = tmpScene.objects.GetComponent(objEnt);
		//tf					 = tmpScene.transforms.GetComponent(objEnt);
		layer				 = tmpScene.layers.GetComponent(objEnt);
		layer->layerMask	 = LAYER_GIZMO;
		object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_ORIGIN];
		object->parentObject = mainEntity;
		object->color		 = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		//tf->Translate(XMFLOAT3(-0.25f + arrowsize * 18, -0.25f - arrowsize, 0.25f - arrowsize));
		//tf->Scale(XMFLOAT3(arrowsize * 1.5, arrowsize * 1.5, arrowsize * 1.5));
		//tf->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
		//tf->UpdateTransform();
		hoverEntities[HOVER_SIZEX].entity		= objEnt;
		hoverEntities[HOVER_SIZEX].hovercolor	= XMFLOAT4(0.0f, 0.4f, 0.8f, 1.f);
		hoverEntities[HOVER_SIZEX].nohovercolor = object->color;
		tmpScene.Component_Attach(objEnt, mainEntity);

		objEnt			 = tmpScene.Entity_CreateObject("SizY");
		object			 = tmpScene.objects.GetComponent(objEnt);
		layer			 = tmpScene.layers.GetComponent(objEnt);
		layer->layerMask = LAYER_GIZMO;
		//tf					 = tmpScene.transforms.GetComponent(objEnt);
		object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_ORIGIN];
		object->parentObject = mainEntity;
		object->color		 = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		//tf->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f - arrowsize, 0.25f + arrowsize * 18));
		//tf->Scale(XMFLOAT3(arrowsize * 1.5, arrowsize * 1.5, arrowsize * 1.5));
		//tf->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
		//tf->UpdateTransform();
		hoverEntities[HOVER_SIZEY].entity		= objEnt;
		hoverEntities[HOVER_SIZEY].hovercolor	= XMFLOAT4(0.5f, 0.8f, 0.0f, 1.f);
		hoverEntities[HOVER_SIZEY].nohovercolor = object->color;
		tmpScene.Component_Attach(objEnt, mainEntity);
		objEnt			 = tmpScene.Entity_CreateObject("SizZ");
		object			 = tmpScene.objects.GetComponent(objEnt);
		layer			 = tmpScene.layers.GetComponent(objEnt);
		layer->layerMask = LAYER_GIZMO;
		//tf					 = tmpScene.transforms.GetComponent(objEnt);
		object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_ORIGIN];
		object->parentObject = mainEntity;
		object->color		 = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		//tf->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f + arrowsize * 18, 0.25f - arrowsize));
		//tf->Scale(XMFLOAT3(arrowsize * 1.5, arrowsize * 1.5, arrowsize * 1.5));
		//tf->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
		//tf->UpdateTransform();
		hoverEntities[HOVER_SIZEZ].entity		= objEnt;
		hoverEntities[HOVER_SIZEZ].hovercolor	= XMFLOAT4(0.8f, 0.1f, 0.0f, 1.f);
		hoverEntities[HOVER_SIZEZ].nohovercolor = object->color;
		tmpScene.Component_Attach(objEnt, mainEntity);
	} else {
		arrowsize = (size.x + size.y) / 100.0f;
		objEnt	  = tmpScene.Entity_CreateObject("cwArrow");
		object	  = tmpScene.objects.GetComponent(objEnt);
		//tf					 = tmpScene.transforms.GetComponent(objEnt);
		layer				 = tmpScene.layers.GetComponent(objEnt);
		layer->layerMask	 = LAYER_GIZMO;
		object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_ROT];
		object->parentObject = mainEntity;
		object->color		 = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		//tf->Translate(XMFLOAT3(arrowsize * 4 + size.x * 0.5f, arrowsize * 2.2 + size.z, size.y * 0.5f));
		//tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
		//tf->RotateRollPitchYaw(XMFLOAT3(0, 0, 0));
		//tf->UpdateTransform();
		hoverEntities[HOVER_ROTCW].entity		= objEnt;
		hoverEntities[HOVER_ROTCW].hovercolor	= XMFLOAT4(0.8f, 0.1f, 0.0f, 1.f);
		hoverEntities[HOVER_ROTCW].nohovercolor = object->color;
		tmpScene.Component_Attach(objEnt, mainEntity);

		objEnt			 = tmpScene.Entity_CreateObject("ccArrow");
		object			 = tmpScene.objects.GetComponent(objEnt);
		layer			 = tmpScene.layers.GetComponent(objEnt);
		layer->layerMask = LAYER_GIZMO;
		//tf					 = tmpScene.transforms.GetComponent(objEnt);
		object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_ROT];
		object->parentObject = mainEntity;
		object->color		 = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		//tf->Translate(XMFLOAT3(arrowsize * 4 + size.x * 0.5f, -arrowsize * 2.2, size.y * 0.5f));
		//tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
		//tf->RotateRollPitchYaw(XMFLOAT3(PI, 0, 0));
		//tf->UpdateTransform();
		hoverEntities[HOVER_ROTCC].entity		= objEnt;
		hoverEntities[HOVER_ROTCC].hovercolor	= XMFLOAT4(0.8f, 0.1f, 0.0f, 1.f);
		hoverEntities[HOVER_ROTCC].nohovercolor = object->color;
		tmpScene.Component_Attach(objEnt, mainEntity);
		arrowsize = (size.x + size.y) / 75.0f;
	}
	objEnt			 = tmpScene.Entity_CreateObject("Origin");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	//tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_ORIGIN];
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	//tf->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f - arrowsize, 0.25f - arrowsize));
	//tf->Scale(XMFLOAT3(arrowsize * 1.5, arrowsize * 1.5, arrowsize * 1.5));
	//tf->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
	//tf->UpdateTransform();
	hoverEntities[HOVER_ORIGIN].entity		 = objEnt;
	hoverEntities[HOVER_ORIGIN].hovercolor	 = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	hoverEntities[HOVER_ORIGIN].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt			 = tmpScene.Entity_CreateObject("xAxis");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	//tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_XAXIS];
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	//tf->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f - arrowsize, 0.25f - arrowsize));
	//tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
	//tf->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
	//tf->UpdateTransform();
	hoverEntities[HOVER_X_AXIS].entity		 = objEnt;
	hoverEntities[HOVER_X_AXIS].hovercolor	 = XMFLOAT4(0.0f, 0.4f, 0.8f, 1.f);
	hoverEntities[HOVER_X_AXIS].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt			 = tmpScene.Entity_CreateObject("yAxis");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	//tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_YAXIS];
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	//tf->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f - arrowsize, 0.25f - arrowsize));
	//tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
	//tf->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
	//tf->UpdateTransform();
	hoverEntities[HOVER_Y_AXIS].entity		 = objEnt;
	hoverEntities[HOVER_Y_AXIS].hovercolor	 = XMFLOAT4(0.5f, 0.8f, 0.0f, 1.f);
	hoverEntities[HOVER_Y_AXIS].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt			 = tmpScene.Entity_CreateObject("zAxis");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	//tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_ZAXIS];
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	//tf->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f - arrowsize, 0.25f - arrowsize));
	//tf->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
	//tf->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
	//tf->UpdateTransform();
	hoverEntities[HOVER_Z_AXIS].entity		 = objEnt;
	hoverEntities[HOVER_Z_AXIS].hovercolor	 = XMFLOAT4(0.8f, 0.1f, 0.0f, 1.f);
	hoverEntities[HOVER_Z_AXIS].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt			 = tmpScene.Entity_CreateObject("planeXZ");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	tf				 = tmpScene.transforms.GetComponent(objEnt);
	if (_toolblock) {
		object->meshID = meshGen::toolBlockFaces[cyBlocks::FACE_FRONT];
		tf->Translate(XMFLOAT3(-0.005f, -0.0005f, -0.0005f));
	} else {
		object->meshID = cyBlocks::m_toolMeshes[cyBlocks::TOOL_PLANE];
		tf->Scale(XMFLOAT3(size.x, size.z, 1));
	}
	tf->Translate(XMFLOAT3(-0.25f, -0.25f, 0.25f));
	tf->UpdateTransform();
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);

	hoverEntities[HOVER_XZ_PLANE].entity	   = objEnt;
	hoverEntities[HOVER_XZ_PLANE].hovercolor   = XMFLOAT4(0.5f, 0.8f, 0.0f, 1.f);
	hoverEntities[HOVER_XZ_PLANE].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt			 = tmpScene.Entity_CreateObject("planeXZ2");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	tf				 = tmpScene.transforms.GetComponent(objEnt);
	if (_toolblock) {
		object->meshID = meshGen::toolBlockFaces[cyBlocks::FACE_BACK];
		tf->Translate(XMFLOAT3(-0.0005f, -0.0005f, -0.0005f));
	} else {
		object->meshID = cyBlocks::m_toolMeshes[cyBlocks::TOOL_PLANE];
		tf->Translate(XMFLOAT3(0, 0, size.y));
		tf->Scale(XMFLOAT3(size.x, size.z, 1));
	}
	tf->Translate(XMFLOAT3(-0.25f, -0.25f, 0.25f));
	tf->UpdateTransform();
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);

	hoverEntities[HOVER_XZ2_PLANE].entity		= objEnt;
	hoverEntities[HOVER_XZ2_PLANE].hovercolor	= XMFLOAT4(0.5f, 0.8f, 0.0f, 1.f);
	hoverEntities[HOVER_XZ2_PLANE].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt			 = tmpScene.Entity_CreateObject("planeYZ");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	tf				 = tmpScene.transforms.GetComponent(objEnt);
	if (_toolblock) {
		object->meshID = meshGen::toolBlockFaces[cyBlocks::FACE_RIGHT];
		tf->Translate(XMFLOAT3(-0.0005f, -0.0005f, -0.0005f));
	} else {
		object->meshID = cyBlocks::m_toolMeshes[cyBlocks::TOOL_PLANE];
		tf->RotateRollPitchYaw(XMFLOAT3(0, -PI / 2, 0));
		tf->Scale(XMFLOAT3(size.y, size.z, 1));
	}
	tf->Translate(XMFLOAT3(-0.25f, -0.25f, 0.25f));
	tf->UpdateTransform();
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);

	hoverEntities[HOVER_YZ_PLANE].entity	   = objEnt;
	hoverEntities[HOVER_YZ_PLANE].hovercolor   = XMFLOAT4(0.0f, 0.4f, 0.8f, 1.f);
	hoverEntities[HOVER_YZ_PLANE].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt			 = tmpScene.Entity_CreateObject("planeYZ2");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	tf				 = tmpScene.transforms.GetComponent(objEnt);
	if (_toolblock) {
		object->meshID = meshGen::toolBlockFaces[cyBlocks::FACE_LEFT];
		tf->Translate(XMFLOAT3(-0.0005f, -0.0005f, -0.0005f));
	} else {
		object->meshID = cyBlocks::m_toolMeshes[cyBlocks::TOOL_PLANE];
		tf->RotateRollPitchYaw(XMFLOAT3(0, -PI / 2, 0));
		tf->Translate(XMFLOAT3(size.x, 0, 0));
		tf->Scale(XMFLOAT3(size.y, size.z, 1));
	}
	tf->Translate(XMFLOAT3(-0.25f, -0.25f, 0.25f));
	tf->UpdateTransform();
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);

	hoverEntities[HOVER_YZ2_PLANE].entity		= objEnt;
	hoverEntities[HOVER_YZ2_PLANE].hovercolor	= XMFLOAT4(0.0f, 0.4f, 0.8f, 1.f);
	hoverEntities[HOVER_YZ2_PLANE].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt			 = tmpScene.Entity_CreateObject("planeXY");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	tf				 = tmpScene.transforms.GetComponent(objEnt);
	if (_toolblock) {
		object->meshID = meshGen::toolBlockFaces[cyBlocks::FACE_BOTTOM];
		tf->Translate(XMFLOAT3(-0.0005f, -0.0005f, -0.0005f));
	} else {
		object->meshID = cyBlocks::m_toolMeshes[cyBlocks::TOOL_PLANE];
		tf->RotateRollPitchYaw(XMFLOAT3(-PI / 2, -PI / 2, 0));
		tf->Scale(XMFLOAT3(size.y, size.x, 1));
	}
	tf->Translate(XMFLOAT3(-0.25f, -0.25f, 0.25f));
	tf->UpdateTransform();
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);

	hoverEntities[HOVER_XY_PLANE].entity	   = objEnt;
	hoverEntities[HOVER_XY_PLANE].hovercolor   = XMFLOAT4(0.8f, 0.1f, 0.0f, 1.f);
	hoverEntities[HOVER_XY_PLANE].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt			 = tmpScene.Entity_CreateObject("planeXY2");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	tf				 = tmpScene.transforms.GetComponent(objEnt);
	if (_toolblock) {
		object->meshID = meshGen::toolBlockFaces[cyBlocks::FACE_TOP];
		tf->Translate(XMFLOAT3(-0.0005f, -0.0005f, -0.0005f));
	} else {
		object->meshID = cyBlocks::m_toolMeshes[cyBlocks::TOOL_PLANE];
		tf->RotateRollPitchYaw(XMFLOAT3(-PI / 2, -PI / 2, 0));
		tf->Translate(XMFLOAT3(0, size.z, 0));
		tf->Scale(XMFLOAT3(size.y, size.x, 1));
	}
	tf->Translate(XMFLOAT3(-0.25f, -0.25f, 0.25f));
	tf->UpdateTransform();
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);

	hoverEntities[HOVER_XY2_PLANE].entity		= objEnt;
	hoverEntities[HOVER_XY2_PLANE].hovercolor	= XMFLOAT4(0.8f, 0.1f, 0.0f, 1.f);
	hoverEntities[HOVER_XY2_PLANE].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt			 = tmpScene.Entity_CreateObject("check");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	//tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_CHECK];
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.2f, 0.6f, 0.2f, 1.0f);
	//tf->RotateRollPitchYaw(XMFLOAT3(0, 0, PI));
	//tf->Translate(XMFLOAT3(size.x - arrowsize / 9, size.z + arrowsize * 2, size.y / 2));
	//tf->Scale(XMFLOAT3(arrowsize / 10, arrowsize / 10, arrowsize / 10));
	//tf->UpdateTransform();
	hoverEntities[HOVER_CHECK].entity		= objEnt;
	hoverEntities[HOVER_CHECK].hovercolor	= XMFLOAT4(0.02f, .6f, 0.0f, 1.f);
	hoverEntities[HOVER_CHECK].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);

	objEnt			 = tmpScene.Entity_CreateObject("cross");
	object			 = tmpScene.objects.GetComponent(objEnt);
	layer			 = tmpScene.layers.GetComponent(objEnt);
	layer->layerMask = LAYER_GIZMO;
	//tf					 = tmpScene.transforms.GetComponent(objEnt);
	object->meshID		 = cyBlocks::m_toolMeshes[cyBlocks::TOOL_CROSS];
	object->parentObject = mainEntity;
	object->color		 = XMFLOAT4(0.6f, 0.2f, 0.2f, 1.0f);
	//tf->RotateRollPitchYaw(XMFLOAT3(0, 0, PI));
	//tf->Translate(XMFLOAT3(arrowsize / 9, size.z + arrowsize * 2, size.y / 2));
	//tf->Scale(XMFLOAT3(arrowsize / 10, arrowsize / 10, arrowsize / 10));
	//tf->RotateRollPitchYaw(XMFLOAT3(0, 0, 0));
	//tf->UpdateTransform();
	hoverEntities[HOVER_CROSS].entity		= objEnt;
	hoverEntities[HOVER_CROSS].hovercolor	= XMFLOAT4(0.6f, 0.0f, 0.02f, 1.f);
	hoverEntities[HOVER_CROSS].nohovercolor = object->color;
	tmpScene.Component_Attach(objEnt, mainEntity);
}

void cySchematic::resizeGizmos(void) {
	float arrowsize = (size.x + size.y) / 75.0f;
	if (arrowsize < 0.1)
		arrowsize = 0.1;

	wiECS::Entity objEnt;
	ObjectComponent* object;
	TransformComponent* tfnew;
	wiScene::LayerComponent* layer;
	for (size_t i = 0; i < HOVER_NUMELEMENTS; i++) {
		if (hoverEntities[i].entity) {
			tfnew = wiScene::GetScene().transforms.GetComponent(hoverEntities[i].entity);
			switch (i) {
				case HOVER_SIZEX:
					tfnew->ClearTransform();
					tfnew->Translate(XMFLOAT3(-0.25f + arrowsize * 18, -0.25f - arrowsize, 0.25f - arrowsize));
					tfnew->Scale(XMFLOAT3(arrowsize * 1.5, arrowsize * 1.5, arrowsize * 1.5));
					break;
				case HOVER_SIZEY:
					tfnew->ClearTransform();
					tfnew->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f - arrowsize, 0.25f + arrowsize * 18));
					tfnew->Scale(XMFLOAT3(arrowsize * 1.5, arrowsize * 1.5, arrowsize * 1.5));
					break;
				case HOVER_SIZEZ:
					tfnew->ClearTransform();
					tfnew->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f + arrowsize * 18, 0.25f - arrowsize));
					tfnew->Scale(XMFLOAT3(arrowsize * 1.5, arrowsize * 1.5, arrowsize * 1.5));
					break;
				case HOVER_ROTCC:
					tfnew->ClearTransform();
					tfnew->Translate(XMFLOAT3(arrowsize * 4 + size.x * 0.5f, -arrowsize * 2.2, size.y * 0.5f));
					tfnew->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
					tfnew->RotateRollPitchYaw(XMFLOAT3(PI, 0, 0));
					break;
				case HOVER_ROTCW:
					tfnew->ClearTransform();
					tfnew->Translate(XMFLOAT3(arrowsize * 4 + size.x * 0.5f, arrowsize * 2.2 + size.z, size.y * 0.5f));
					tfnew->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
					tfnew->RotateRollPitchYaw(XMFLOAT3(0, 0, 0));
					break;
				case HOVER_ORIGIN:
					tfnew->ClearTransform();
					tfnew->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f - arrowsize, 0.25f - arrowsize));
					tfnew->Scale(XMFLOAT3(arrowsize * 1.5, arrowsize * 1.5, arrowsize * 1.5));
					tfnew->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
					break;
				case HOVER_X_AXIS:
					tfnew->ClearTransform();
					tfnew->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f - arrowsize, 0.25f - arrowsize));
					tfnew->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
					tfnew->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
					break;
				case HOVER_Y_AXIS:
					tfnew->ClearTransform();
					tfnew->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f - arrowsize, 0.25f - arrowsize));
					tfnew->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
					tfnew->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
					break;
				case HOVER_Z_AXIS:
					tfnew->ClearTransform();
					tfnew->Translate(XMFLOAT3(-0.25f - arrowsize, -0.25f - arrowsize, 0.25f - arrowsize));
					tfnew->Scale(XMFLOAT3(arrowsize, arrowsize, arrowsize));
					tfnew->RotateRollPitchYaw(XMFLOAT3(PI / 2, 0, 0));
					break;
				case HOVER_CHECK:
					tfnew->ClearTransform();
					tfnew->RotateRollPitchYaw(XMFLOAT3(0, 0, PI));
					tfnew->Translate(XMFLOAT3(size.x - arrowsize / 9, size.z + arrowsize * 2, size.y / 2));
					tfnew->Scale(XMFLOAT3(arrowsize / 10, arrowsize / 10, arrowsize / 10));
					break;
				case HOVER_CROSS:
					tfnew->ClearTransform();
					tfnew->RotateRollPitchYaw(XMFLOAT3(-0, 0, PI));
					tfnew->Translate(XMFLOAT3(arrowsize / 9, size.z + arrowsize * 2, size.y / 2));
					tfnew->Scale(XMFLOAT3(arrowsize / 10, arrowsize / 10, arrowsize / 10));
					break;
				default:
					break;
			}
			tfnew->UpdateTransform();
		}
	}
}

cySchematic::chunkstate_t cySchematic::checkAffectedChunks(void) {
	chunkLoader::clearMaskedChunk();
	settings::worldOffset_t offset;
	cyImportant* world = settings::getWorld();
	offset.x		   = settings::getWorld()->m_playerpos.x / 100;
	offset.y		   = settings::getWorld()->m_playerpos.y / 100;
	uint32_t chunkID   = 0;
	if (world->isValid()) {
		for (float x = floor((pos.x - 0.5f) / 16); x <= ceil((pos.x + size.x + 0.5f) / 16); x += 1) {
			for (float y = floor((pos.y - 0.5f) / 16); y <= ceil((pos.y + size.y + 0.5f) / 16); y += 1) {
				if (settings::getWorld()->getChunkID(offset.x + x * 16, offset.y - y * 16, &chunkID)) {
					cyChunk chunk;
					chunk.loadChunk(world->db[cyImportant::DBHANDLE_MAIN], chunkID);
					if (!chunk.m_saveable) {
						if (chunk.m_version < 29)
							return STATE_OLD;
						else
							return STATE_BROKEN;
					}

				} else {
					return STATE_NOTPRESENT;
				}
			}
		}
	} else
		return STATE_NOTPRESENT;

	return STATE_SAVEABLE;
}

void cySchematic::generateChunkPreview(void) {
	cyImportant::chunkpos_t zero, chunkPos;
	uint32_t chunkID;

	cyImportant* world = settings::getWorld();

	zero.x = world->m_playerpos.x / 100;
	zero.y = world->m_playerpos.y / 100;

	if (world->isValid()) {
		settings::pauseChunkloader = true;
		wiScene::Scene& scene	   = wiScene::GetScene();
		chunkLoader::clearMaskedChunk();
		std::vector<chunkLoader::chunkobjects_t> OLDchunkPreviews = m_chunkPreviews;
		m_chunkPreviews.clear();

		for (float x = floor((pos.x - 0.5f) / 16); x <= ceil((pos.x + size.x + 0.5f) / 16); x += 1) {
			for (float y = floor((pos.y - 0.5f) / 16); y <= ceil((pos.y + size.y + 0.5f) / 16); y += 1) {
				chunkPos = world->getChunkPos(x * 16, -y * 16);
				cyChunk chunk;
				cyChunk chunkL;
				cyChunk chunkR;
				cyChunk chunkU;
				cyChunk chunkD;
				if (world->getChunkID(chunkPos.x + zero.x, chunkPos.y + zero.y, &chunkID)) {
					chunk.loadChunk(world->db[cyImportant::DBHANDLE_MAIN], chunkID);
					for (float schX = max(pos.x, (float)chunkPos.x); schX < min(pos.x + size.x, (float)chunkPos.x + 16); schX += 0.5f) {
						for (float schY = max(pos.y, -(float)chunkPos.y); schY < min(pos.y + size.y, -(float)chunkPos.y + 16); schY += 0.5f) {
							for (float schZ = pos.z; schZ < pos.z + size.z; schZ += 0.5f) {
								chunk.replaceWithAir((uint8_t)((schX - chunkPos.x) * 2), (uint8_t)((15.5 - schY - chunkPos.y) * 2), (uint16_t)((schZ)*2));
								//chunk.m_chunkdata[4 + (uint8_t)((schX - chunkPos.x) * 2) + 32 * (uint8_t)((15.5 - schY - chunkPos.y) * 2) + 32 * 32 * (uint16_t)((schZ)*2)] = cyBlocks::m_voidID;
							}
						}
					}
					chunk.m_lowestZ	 = min(chunk.m_lowestZ, (uint16_t)(pos.z * 2));
					chunk.m_highestZ = max(chunk.m_highestZ, (uint16_t)((pos.z + size.z) * 2));
					if (world->getChunkID(chunkPos.x + 16 + zero.x, chunkPos.y + zero.y, &chunkID)) {
						chunkL.loadChunk(world->db[cyImportant::DBHANDLE_MAIN], chunkID, true);
						for (float schX = max(pos.x, (float)chunkPos.x + 16); schX < min(pos.x + size.x, (float)chunkPos.x + 32); schX += 0.5f) {
							for (float schY = max(pos.y, -(float)chunkPos.y); schY < min(pos.y + size.y, -(float)chunkPos.y + 16); schY += 0.5f) {
								for (float schZ = pos.z; schZ < pos.z + size.z; schZ += 0.5f) {
									chunkL.replaceWithAir((uint8_t)((schX - chunkPos.x - 16) * 2), (uint8_t)((15.5 - schY - chunkPos.y) * 2), (uint16_t)((schZ)*2));
									//chunk.m_chunkdata[4 + (uint8_t)((schX - chunkPos.x - 16) * 2) + 32 * (uint8_t)((15.5 - schY - chunkPos.y) * 2) + 32 * 32 * (uint16_t)((schZ)*2)] = cyBlocks::m_voidID;
								}
							}
						}
					} else
						chunkL.airChunk();
					if (world->getChunkID(chunkPos.x - 16 + zero.x, chunkPos.y + zero.y, &chunkID)) {
						chunkR.loadChunk(world->db[cyImportant::DBHANDLE_MAIN], chunkID, true);
						for (float schX = max(pos.x, (float)chunkPos.x - 16); schX < min(pos.x + size.x, (float)chunkPos.x); schX += 0.5f) {
							for (float schY = max(pos.y, -(float)chunkPos.y); schY < min(pos.y + size.y, -(float)chunkPos.y + 16); schY += 0.5f) {
								for (float schZ = pos.z; schZ < pos.z + size.z; schZ += 0.5f) {
									chunkR.replaceWithAir((uint8_t)((schX - chunkPos.x + 16) * 2), (uint8_t)((15.5 - schY - chunkPos.y) * 2), (uint16_t)((schZ)*2));
									//chunk.m_chunkdata[4 + (uint8_t)((schX - chunkPos.x + 16) * 2) + 32 * (uint8_t)((15.5 - schY - chunkPos.y) * 2) + 32 * 32 * (uint16_t)((schZ)*2)] = cyBlocks::m_voidID;
								}
							}
						}
					}

					else
						chunkR.airChunk();
					if (world->getChunkID(chunkPos.x + zero.x, chunkPos.y + zero.y + 16, &chunkID)) {
						chunkU.loadChunk(world->db[cyImportant::DBHANDLE_MAIN], chunkID, true);
						for (float schX = max(pos.x, (float)chunkPos.x); schX < min(pos.x + size.x, (float)chunkPos.x + 16); schX += 0.5f) {
							for (float schY = max(pos.y, -(float)chunkPos.y - 16); schY < min(pos.y + size.y, -(float)chunkPos.y); schY += 0.5f) {
								for (float schZ = pos.z; schZ < pos.z + size.z; schZ += 0.5f) {
									chunkU.replaceWithAir((uint8_t)((schX - chunkPos.x) * 2), (uint8_t)((15.5 - schY - chunkPos.y - 16) * 2), (uint16_t)((schZ)*2));
									//chunk.m_chunkdata[4 + (uint8_t)((schX - chunkPos.x) * 2) + 32 * (uint8_t)((15.5 - schY - chunkPos.y - 16) * 2) + 32 * 32 * (uint16_t)((schZ)*2)] = cyBlocks::m_voidID;
								}
							}
						}
					}

					else
						chunkU.airChunk();
					if (world->getChunkID(chunkPos.x + zero.x, chunkPos.y + zero.y - 16, &chunkID)) {
						chunkD.loadChunk(world->db[cyImportant::DBHANDLE_MAIN], chunkID, true);
						for (float schX = max(pos.x, (float)chunkPos.x); schX < min(pos.x + size.x, (float)chunkPos.x + 16); schX += 0.5) {
							for (float schY = max(pos.y, -(float)chunkPos.y + 16); schY < min(pos.y + size.y, -(float)chunkPos.y + 32); schY += 0.5) {
								for (float schZ = pos.z; schZ < pos.z + size.z; schZ += 0.5) {
									chunkD.replaceWithAir((uint8_t)((schX - chunkPos.x) * 2), (uint8_t)((15.5 - schY - chunkPos.y + 16) * 2), (uint16_t)((schZ)*2));
								}
							}
						}
					}

					else
						chunkD.airChunk();
					m_chunkPreviews.push_back(chunkLoader::RenderChunk(chunk, chunkU, chunkL, chunkD, chunkR, chunkPos.x, -chunkPos.y, false));
					wiScene::ObjectComponent* mesh = nullptr;
					for (size_t iii = 0; iii < m_chunkPreviews.back().meshes.size(); iii++) {
						m.lock();
						mesh = wiScene::GetScene().objects.GetComponent(m_chunkPreviews.back().meshes[iii]);
						if (mesh != nullptr) {
							mesh->SetCastShadow(true);
							mesh->SetRenderable(true);
						}
						m.unlock();
					}
					chunkLoader::addMaskedChunk(chunkPos);
				}
			}
		}
		for (size_t i = 0; i < OLDchunkPreviews.size(); i++) {
			wiScene::ObjectComponent* obj = scene.objects.GetComponent(OLDchunkPreviews[i].chunkObj);
			if (obj != nullptr) {
				wiECS::Entity meshEnt = obj->meshID;
				m.lock();
				scene.Component_RemoveChildren(OLDchunkPreviews[i].chunkObj);
				scene.Entity_Remove(OLDchunkPreviews[i].chunkObj);
				scene.Entity_Remove(meshEnt);
				m.unlock();
			}
		}
		settings::pauseChunkloader = false;
	}
}

void cySchematic::drawGridLines(const bool x, const bool y, const bool z) {
	/*line.start.x = cySchematic::m_schematics[dragID]->pos.x - 0.25;
						line.start.y = cySchematic::m_schematics[dragID]->pos.z - 0.25;
						line.start.z = cySchematic::m_schematics[dragID]->pos.y + 0.25;
						line.end	 = line.start;*/
	float gridlen	  = 30;
	XMFLOAT3 gridstep = XMFLOAT3(0.5f, 0.5f, 0.5f);
	XMFLOAT3 linestart;
	linestart.x		 = pos.x - 0.25;
	linestart.y		 = pos.z - 0.25;
	linestart.z		 = pos.y + 0.25;
	XMFLOAT3 lineend = linestart;

	if (wiInput::Down(wiInput::KEYBOARD_BUTTON_LSHIFT)) {
		gridstep.x = size.x;
		gridstep.y = size.z;
		gridstep.z = size.y;
		gridlen	   = 10;
	}
	if (x) {
		for (float i = 0; i < gridlen; i++) {
			wiRenderer::DrawLine(wiRenderer::RenderableLine{XMFLOAT3(linestart.x - gridstep.x * i, linestart.y, linestart.z),
															XMFLOAT3(linestart.x - gridstep.x / 2 - gridstep.x * i, linestart.y, linestart.z),
															XMFLOAT4(1, 1, 1, (gridlen - i) / gridlen),
															XMFLOAT4(0.25, 0.25, 0.25, (gridlen - i) / gridlen)});
			wiRenderer::DrawLine(wiRenderer::RenderableLine{XMFLOAT3(linestart.x + gridstep.x * i + size.x, linestart.y, linestart.z),
															XMFLOAT3(linestart.x + gridstep.x / 2 + gridstep.x * i + size.x, linestart.y, linestart.z),
															XMFLOAT4(1, 1, 1, (gridlen - i) / gridlen),
															XMFLOAT4(0.25, 0.25, 0.25, (gridlen - i) / gridlen)});
		}
	}
	if (y) {
		for (float i = 0; i < gridlen; i++) {
			wiRenderer::DrawLine(wiRenderer::RenderableLine{XMFLOAT3(linestart.x, linestart.y, linestart.z - gridstep.z * i),
															XMFLOAT3(linestart.x, linestart.y, linestart.z - gridstep.z / 2 - gridstep.z * i),
															XMFLOAT4(1, 1, 1, (gridlen - i) / gridlen),
															XMFLOAT4(0.25, 0.25, 0.25, (gridlen - i) / gridlen)});
			wiRenderer::DrawLine(wiRenderer::RenderableLine{XMFLOAT3(linestart.x, linestart.y, linestart.z + gridstep.z * i + size.y),
															XMFLOAT3(linestart.x, linestart.y, linestart.z + gridstep.z / 2 + gridstep.z * i + size.y),
															XMFLOAT4(1, 1, 1, (gridlen - i) / gridlen),
															XMFLOAT4(0.25, 0.25, 0.25, (gridlen - i) / gridlen)});
		}
	}
	if (z) {
		for (float i = 0; i < gridlen; i++) {
			wiRenderer::DrawLine(wiRenderer::RenderableLine{XMFLOAT3(linestart.x, linestart.y - gridstep.y * i, linestart.z),
															XMFLOAT3(linestart.x, linestart.y - gridstep.y / 2 - gridstep.y * i, linestart.z),
															XMFLOAT4(1, 1, 1, (gridlen - i) / gridlen),
															XMFLOAT4(0.25, 0.25, 0.25, (gridlen - i) / gridlen)});
			wiRenderer::DrawLine(wiRenderer::RenderableLine{XMFLOAT3(linestart.x, linestart.y + gridstep.y * i + size.z, linestart.z),
															XMFLOAT3(linestart.x, linestart.y + gridstep.y / 2 + gridstep.y * i + size.z, linestart.z),
															XMFLOAT4(1, 1, 1, (gridlen - i) / gridlen),
															XMFLOAT4(0.25, 0.25, 0.25, (gridlen - i) / gridlen)});
		}
	}
}

void cySchematic::showGizmos(bool _show) {
	for (uint8_t i = 0; i < HOVER_NUMELEMENTS; i++) {
		if (hoverEntities[i].entity != wiECS::INVALID_ENTITY)
			wiScene::GetScene().objects.GetComponent(hoverEntities[i].entity)->SetRenderable(_show);
	}
}

void cySchematic::rotate(bool _cclock) {
	wiScene::Scene& scene				= wiScene::GetScene();
	float angle							= 0.01f;
	float step							= 0.0f;
	wiScene::TransformComponent* tf		= wiScene::GetScene().transforms.GetComponent(mainEntity);
	wiScene::TransformComponent starttf = *tf;
	wiScene::TransformComponent endtf	= *tf;
	wiScene::Scene tmpScene;
	wiJobSystem::context ctx;
	showGizmos(false);
	m_dirty						  = DIRTY_NOTRENDERED;						 //Make sure no more gizmo interaction is tracked, because gizmo references are updated within thread by prepareSchematic at any time
	wiScene::ObjectComponent* obj = scene.objects.GetComponent(mainEntity);	 //get the ID of the currently rendered objects NOW to delete them after the prepare thread has already overwritten them with the new ones.
	wiECS::Entity meshEnt		  = obj->meshID;
	if (_cclock) {
		endtf.Translate(XMFLOAT3(size.y, 0, 0));  //<<<<--------TODO: find the correct translation to emulate rotation around center
		endtf.RotateRollPitchYaw(XMFLOAT3(0, -PI / 2, 0));
		if (--m_rotation > 3) {
			m_rotation = 3;
		}
		wiJobSystem::Execute(ctx, [this, &tmpScene](wiJobArgs args) { rotateMemory(true); });
	} else {
		endtf.Translate(XMFLOAT3(0, 0, size.x));
		endtf.RotateRollPitchYaw(XMFLOAT3(0, PI / 2, 0));
		if (++m_rotation > 3) {
			m_rotation = 0;
		}
		wiJobSystem::Execute(ctx, [this, &tmpScene](wiJobArgs args) { rotateMemory(false); });
	}

	endtf.UpdateTransform();
	while (angle < 1) {
		angle += sinf(angle * PI) * 0.2;

		m.lock();
		tf->Lerp(starttf, endtf, angle);

		//tf->Translate(XMFLOAT3(step * ((float)floorf((size.x)) / (PI)), 0, 0));
		//tf->UpdateTransform();
		m.unlock();
		Sleep(20);
	}

	float tmp = size.x;
	//pos.x += floor((size.x - size.y)) / 2;	//adjust the origin position
	//pos.y += floor((size.y - size.x)) / 2;
	size.x = size.y;  //swap width/height
	size.y = tmp;

	//delete mesh->
	m.lock();
	scene.Component_RemoveChildren(mainEntity);
	scene.Entity_Remove(mainEntity);
	scene.Entity_Remove(meshEnt);
	m.unlock();
	prepareSchematic(tmpScene);
	
	while (wiJobSystem::IsBusy(ctx))
		;
	m.lock();
	scene.Merge(tmpScene);
	showGizmos(false);
	m.unlock();
	resizeGizmos();
	generateChunkPreview();
	showGizmos(true);
}

void cySchematic::rotateMemory(const bool _cc) {
	uint8_t torchrot_change[6] = {3, 2, 0, 1, 4, 5};
	if (_cc) {
		torchrot_change[0] = 2;
		torchrot_change[1] = 3;
		torchrot_change[2] = 1;
		torchrot_change[3] = 0;
	}
	uint8_t* new_blocks;
	unordered_map<blockpos_t, uint32_t, blockpos_t> old_cBlocks;
	unordered_map<blockpos_t, uint8_t, blockpos_t> old_torches;
	const blockpos_t bsize(size.x * 2, size.y * 2, size.z * 2);
	blockpos_t bpos;
	uint8_t tmp = 0;

	try {
		old_cBlocks = m_cBlocks;
		old_torches = m_torches;

		new_blocks = (uint8_t*)malloc((size_t)(size.x * 2 * size.y * 2 * size.z * 2));
	}
	catch (...) {
		wiHelper::messageBox("Insufficient Memory, reduce view distance or schematic size.", "Error!");
		return;
	}
	m_torches.clear();
	m_cBlocks.clear();
	if (!_cc) {
		for (uint32_t z = 0; z < bsize.z; z++) {
			uint32_t dest_col = bsize.y - 1;
			for (uint32_t y = 0; y < bsize.y; y++) {
				for (uint32_t x = 0; x < bsize.x; x++) {
					new_blocks[x * bsize.y + dest_col] = m_chunkdata[z * bsize.x * bsize.y + y * bsize.x + x];
					if (cyBlocks::m_regBlockTypes[new_blocks[x * bsize.y + dest_col]] == cyBlocks::BLOCKTYPE_MOD) {
						bpos	= {x, y, z};
						auto it = old_cBlocks.find(bpos);
						if (it != old_cBlocks.end()) {
							bpos.x			= dest_col;
							bpos.y			= x;
							m_cBlocks[bpos] = it->second;
						}
					} else if (cyBlocks::m_regBlockTypes[new_blocks[x * bsize.y + dest_col]] == cyBlocks::BLOCKTYPE_TORCH) {
						bpos	= {x, y, z};
						auto it = old_torches.find(bpos);
						if (it != old_torches.end()) {
							bpos.x			= dest_col;
							bpos.y			= x;
							m_torches[bpos] = torchrot_change[it->second];
						}
					}
				}
				dest_col--;
			}
			memcpy(&(m_chunkdata[z * bsize.x * bsize.y]), new_blocks, bsize.x * bsize.y);
		}
	} else {
		for (uint32_t z = 0; z < bsize.z; z++) {
			for (uint32_t y = 0; y < bsize.y; y++) {
				uint32_t dest_col = bsize.x - 1;
				for (uint32_t x = 0; x < bsize.x; x++) {
					new_blocks[dest_col * bsize.y + y] = m_chunkdata[z * bsize.x * bsize.y + y * bsize.x + x];
					if (cyBlocks::m_regBlockTypes[new_blocks[dest_col * bsize.y + y]] == cyBlocks::BLOCKTYPE_MOD) {
						bpos	= {x, y, z};
						auto it = old_cBlocks.find(bpos);
						if (it != old_cBlocks.end()) {
							bpos.y			= dest_col;
							bpos.x			= y;
							m_cBlocks[bpos] = it->second;
						}
					} else if (cyBlocks::m_regBlockTypes[new_blocks[dest_col * bsize.y + y]] == cyBlocks::BLOCKTYPE_TORCH) {
						bpos	= {x, y, z};
						auto it = old_torches.find(bpos);
						if (it != old_torches.end()) {
							bpos.y			= dest_col;
							bpos.x			= y;
							m_torches[bpos] = torchrot_change[it->second];
						}
					}
					dest_col--;
				}
			}
			memcpy(&(m_chunkdata[z * bsize.x * bsize.y]), new_blocks, bsize.x * bsize.y);
		}
	}
	free(new_blocks);
}
/*
	Procedure RotateSchematic(Map customBlocks.xy(), Map torches.xy(), *destbuff, rotation)
  Dim dest(toolBox\sx * toolBox\sz)
  Dim rotRemap(6)
  rotRemap(0) = 3
  rotRemap(1) = 2
  rotRemap(2) = 0
  rotRemap(3) = 1
  rotRemap(4) = 4
  rotRemap(5) = 5
  NewMap customBlocksRot.xy()
  NewMap torchesRot.xy()
  While rotation
    sx = toolBox\sx
    sy = toolBox\sz
    sz = toolBox\sy
    For z = 0 To sz-1
      dest_col = sy - 1 
      For h = 0 To sy - 1
        For w = 0 To sx - 1
          tmp = PeekB(*destbuff+z*sx*sy + h*sx + w)
          dest(w * sy + dest_col) = tmp
          If tmp = 66
            If FindMapElement(customBlocks(),Str(w)+","+Str(h)+","+Str(z))
              customBlocksRot(Str(dest_col)+","+Str(w)+","+Str(z))\vis = customBlocks()\vis
            EndIf
          ElseIf SBlocks(tmp)\type = #BLOCKTYPE_Torch
            If FindMapElement(torches(),Str(w)+","+Str(h)+","+Str(z))
              torchesRot(Str(dest_col)+","+Str(w)+","+Str(z))\vis = rotRemap(torches()\vis) ;Rotate torches
            EndIf
          EndIf
         Next
         dest_col-1
       Next
       For h = 0 To sy - 1
        For w = 0 To sx - 1
            PokeB(*destbuff+z*sx*sy + h*sx + w, dest(h*sx + w))
         Next
       Next
     Next z
     
     
     ClearMap(customBlocks())
     ClearMap(torches())
     
     ResetMap(customBlocksRot())
     While NextMapElement(customBlocksRot())
       customBlocks(MapKey(customBlocksRot()))\vis = customBlocksRot()\vis
     Wend

     ResetMap(torchesRot())
     While NextMapElement(torchesRot())
       torches(MapKey(torchesRot()))\vis = torchesRot()\vis
     Wend
     rotation-1
     toolBox\sx = sy
     toolBox\sz = sx
     toolBox\sy = sz
     ClearMap(torchesRot())
     ClearMap(customBlocksRot())
  Wend
  FreeMap(customBlocksRot())
  FreeMap(torchesRot())
EndProcedure
*/
void cySchematic::saveToWorld(void) {
	cyImportant::chunkpos_t zero, chunkPos;
	uint32_t chunkID;
	bool success		  = false;
	cyImportant* world	  = settings::getWorld();
	wiScene::Scene& scene = wiScene::GetScene();
	const blockpos_t bsize((uint32_t)(size.x * 2.f), (uint32_t)(size.y * 2.f), (uint32_t)(size.z * 2.f));
	zero.x				   = (int32_t)(world->m_playerpos.x / 100.);
	zero.y				   = (int32_t)(world->m_playerpos.y / 100.);
	chunkstate_t areastate = checkAffectedChunks();
	switch (areastate) {
		case STATE_SAVEABLE:
			break;
		case STATE_BROKEN:
			wiHelper::messageBox("There is at least one chunk, that was either not yet generated, or with data format not completely readable by CyubE3dit.");
			return;
		case STATE_NOTPRESENT:
			wiHelper::messageBox("There is at least one chunk, that was either not yet generated. Please visit this spot in VR to generate the world here before inserting schematics.");
			return;
		case STATE_OLD:
			wiHelper::messageBox("There is at least one chunk, that was saved with a very old version of the game, please visit this spot in VR to let the game update the world data to the newest format.");
			return;
		default:
			wiHelper::messageBox("There was a problem checking the affected chunks before the update, aborted!");
			return;
	}

	if (world->isValid()) {
		wiScene::Scene& scene = wiScene::GetScene();
		m_dirty				  = DIRTY_NOTRENDERED;	//Make sure no more gizmo interaction is tracked, because gizmo references are updated within thread by prepareSchematic at any time
		showGizmos(false);
		uint8_t blockID;
		blockpos_t bpos;
		uint32_t blockData = 0;
		for (float x = floor((pos.x - 0.5f) / 16.f); x <= ceil((pos.x + size.x + 0.5f) / 16.f); x += 1.f) {
			for (float y = floor((pos.y - 0.5f) / 16.f); y <= ceil((pos.y + size.y + 0.5f) / 16.f); y += 1.f) {
				chunkPos = world->getChunkPos(x * 16, -y * 16);
				cyChunk chunk;

				if (world->getChunkID(chunkPos.x + zero.x, chunkPos.y + zero.y, &chunkID)) {
					chunk.loadChunk(world->db[cyImportant::DBHANDLE_MAIN], chunkID);
					for (float schX = max(pos.x, (float)chunkPos.x); schX < min(pos.x + size.x, (float)chunkPos.x + 16); schX += 0.5f) {
						for (float schY = max(pos.y, -(float)chunkPos.y); schY < min(pos.y + size.y, -(float)chunkPos.y + 16); schY += 0.5f) {
							for (float schZ = pos.z; schZ < pos.z + size.z; schZ += 0.5f) {
								bpos.x	= (uint32_t)((schX - pos.x) * 2.f);
								bpos.y	= (uint32_t)((size.y - 0.5f - (schY - pos.y)) * 2.f);
								bpos.z	= (uint32_t)((schZ - pos.z) * 2.f);
								blockID = *(m_chunkdata + bpos.x + bsize.x * bpos.y + bsize.x * bsize.y * bpos.z);
								if (cyBlocks::m_regBlockTypes[blockID] == cyBlocks::BLOCKTYPE_MOD) {
									auto it = m_cBlocks.find(bpos);
									if (it != m_cBlocks.end()) {
										blockData = it->second;
									}
								} else if (cyBlocks::m_regBlockTypes[blockID] == cyBlocks::BLOCKTYPE_TORCH) {
									auto it = m_torches.find(bpos);
									if (it != m_torches.end()) {
										blockData = it->second;
									}
								}
								chunk.replaceBlock((uint8_t)((schX - chunkPos.x) * 2.f), (uint8_t)((15.5f - schY - chunkPos.y) * 2.f), (uint16_t)(schZ * 2.f), blockID, blockData);
							}
						}
					}
					chunk.saveZlimits(min(chunk.m_lowestZ, (uint16_t)(pos.z * 2.f)), max(chunk.m_highestZ, (uint16_t)((pos.z + size.z) * 2)));
					success = chunk.saveChunk();
				}
			}
		}
		if (success) {
			m_dirty = DIRTY_REMOVE;
		} else {
			showGizmos(true);
			m_dirty = NOT_DIRTY;
		}
	}
}

void cySchematic::saveToMem(void) {
	cyImportant::chunkpos_t zero, chunkPos;
	uint32_t chunkID;
	bool success		  = false;
	cyImportant* world	  = settings::getWorld();
	wiScene::Scene& scene = wiScene::GetScene();
	const blockpos_t bsize((uint32_t)(size.x * 2.f), (uint32_t)(size.y * 2.f), (uint32_t)(size.z * 2.f));
	zero.x				   = (int32_t)(world->m_playerpos.x / 100.);
	zero.y				   = (int32_t)(world->m_playerpos.y / 100.);
	chunkstate_t areastate = checkAffectedChunks();
	switch (areastate) {
		case STATE_SAVEABLE:
			break;
		case STATE_BROKEN:
			wiHelper::messageBox("There is at least one chunk, that was either not yet generated, or with data format not completely readable by CyubE3dit.");
			return;
		case STATE_NOTPRESENT:
			wiHelper::messageBox("There is at least one chunk, that was either not yet generated. Please visit this spot in VR to generate the world here before inserting schematics.");
			return;
		case STATE_OLD:
			wiHelper::messageBox("There is at least one chunk, that was saved with a very old version of the game, please visit this spot in VR to let the game update the world data to the newest format.");
			return;
		default:
			wiHelper::messageBox("There was a problem checking the affected chunks before the update, aborted!");
			return;
	}

	if (world->isValid()) {
		wiScene::Scene& scene = wiScene::GetScene();
		m_dirty				  = DIRTY_NOTRENDERED;	//Make sure no more gizmo interaction is tracked, because gizmo references are updated within thread by prepareSchematic at any time
		showGizmos(false);
		uint8_t blockID;
		blockpos_t bpos;
		uint32_t blockData = 0;
		m_chunkdata		   = reinterpret_cast<uint8_t*>(realloc(m_chunkdata, size.x * 2 * size.y * 2 * size.z * 2));
		if (m_chunkdata != nullptr) {
			success = true;
			for (float x = floor((pos.x - 0.5f) / 16.f); x <= ceil((pos.x + size.x + 0.5f) / 16.f); x += 1.f) {
				for (float y = floor((pos.y - 0.5f) / 16.f); y <= ceil((pos.y + size.y + 0.5f) / 16.f); y += 1.f) {
					chunkPos = world->getChunkPos(x * 16, -y * 16);
					cyChunk chunk;

					if (world->getChunkID(chunkPos.x + zero.x, chunkPos.y + zero.y, &chunkID)) {
						chunk.loadChunk(world->db[cyImportant::DBHANDLE_MAIN], chunkID);
						for (float schX = max(pos.x, (float)chunkPos.x); schX < min(pos.x + size.x, (float)chunkPos.x + 16); schX += 0.5f) {
							for (float schY = max(pos.y, -(float)chunkPos.y); schY < min(pos.y + size.y, -(float)chunkPos.y + 16); schY += 0.5f) {
								for (float schZ = pos.z; schZ < pos.z + size.z; schZ += 0.5f) {
									bpos.x = (uint32_t)((schX - pos.x) * 2.f);
									bpos.y = (uint32_t)((size.y - 0.5f - (schY - pos.y)) * 2.f);
									bpos.z = (uint32_t)((schZ - pos.z) * 2.f);
									chunk.getBlock((uint8_t)((schX - chunkPos.x) * 2.f), (uint8_t)((15.5f - schY - chunkPos.y) * 2.f), (uint16_t)(schZ * 2.f), &blockID, &blockData);
									*(m_chunkdata + bpos.x + bsize.x * bpos.y + bsize.x * bsize.y * bpos.z) = blockID;
									if (cyBlocks::m_regBlockTypes[blockID] == cyBlocks::BLOCKTYPE_MOD) {
										auto it = m_cBlocks[bpos] = blockData;
									} else if (cyBlocks::m_regBlockTypes[blockID] == cyBlocks::BLOCKTYPE_TORCH) {
										auto it = m_torches[bpos] = blockData;
									}
								}
							}
						}
					}
				}
			}
		}
		if (success) {
			saveToFile();
		} else {
			showGizmos(true);
			m_dirty = NOT_DIRTY;
		}
	}
}

void cySchematic::saveToFile(void) {
	wiHelper::FileDialogParams params;
	params.type		   = wiHelper::FileDialogParams::SAVE;
	params.description = "CyubeVR schematic";
	params.extensions.push_back("cySch");
	wiHelper::FileDialog(params, [this](std::string fileName) {
		wiEvent::Subscribe_Once(SYSTEM_EVENT_THREAD_SAFE_POINT, [=](uint64_t userdata) {
			bool success		 = false;
			std::string filename = fileName;
			if (filename.length() > 6) {
				if (filename.substr(filename.length() - 6).compare(".cySch") != 0)
				{
					filename += ".cySch";
				}
			} else if (filename.length() > 2) {
				filename += ".cySch";
			}
			if (filename.length()) {
				uint32_t mapLen;
				size_t offset = 0;
				blockpos_t pos;
				size_t datalen = size.x * 2 * size.y * 2 * size.z * 2 + m_cBlocks.size() * 10 + m_torches.size() * 7 + 8;
				m_chunkdata	   = reinterpret_cast<uint8_t*>(realloc(m_chunkdata, datalen));
				if (datalen < 500)
					datalen = 500;
				char* compressed = reinterpret_cast<char*>(malloc(datalen));
				if (m_chunkdata != nullptr && compressed != nullptr) {
					offset = size.x * 2 * size.y * 2 * size.z * 2;
					mapLen = m_cBlocks.size();
					memcpy(m_chunkdata + offset, &mapLen, 4);
					offset += 4;
					for (auto it = m_cBlocks.begin(); it != m_cBlocks.end(); it++) {
						pos = it->first;
						memcpy(m_chunkdata + offset, &pos.x, 2);
						memcpy(m_chunkdata + offset + 2, &pos.y, 2);
						memcpy(m_chunkdata + offset + 4, &pos.z, 2);
						memcpy(m_chunkdata + offset + 6, &(it->second), 4);
						offset += 10;
					}
					mapLen = m_torches.size();
					memcpy(m_chunkdata + offset, &mapLen, 4);
					offset += 4;
					for (auto it = m_torches.begin(); it != m_torches.end(); it++) {
						pos = it->first;
						memcpy(m_chunkdata + offset, &pos.x, 2);
						memcpy(m_chunkdata + offset + 2, &pos.y, 2);
						memcpy(m_chunkdata + offset + 4, &pos.z, 2);
						memcpy(m_chunkdata + offset + 6, &(it->second), 1);
						offset += 7;
					}
					size_t compressedLen = LZ4_compress_default(reinterpret_cast<char*>(m_chunkdata), compressed + 16, offset, datalen);
					memcpy(compressed + compressedLen + 16, &offset, 4);
					mapLen = size.x * 2;
					memcpy(compressed, &(mapLen), 4);
					mapLen = size.y * 2;
					memcpy(compressed + 4, &(mapLen), 4);
					mapLen = size.z * 2;
					memcpy(compressed + 8, &(mapLen), 4);
					mapLen = 0x13371337;
					memcpy(compressed + 12, &(mapLen), 4);
					compressedLen += 20;
					if (wiHelper::FileWrite(filename, reinterpret_cast<uint8_t*>(compressed), compressedLen)) {
						success = true;
					}
					free(compressed);
				}
			}
			if (success) {
				m_dirty = DIRTY_REMOVE;
			} else {
				showGizmos(true);
				m_dirty = NOT_DIRTY;
			}
		});
	});
}
