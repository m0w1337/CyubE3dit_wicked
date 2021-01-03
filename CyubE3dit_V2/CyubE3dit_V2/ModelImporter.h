#pragma once
#include <string>

namespace wiScene
{
	struct Scene;
}

wiECS::Entity ImportModel_OBJ(const std::string& fileName, wiScene::Scene& scene, uint8_t windMode);
void ImportModel_GLTF(const std::string& fileName, wiScene::Scene& scene);

