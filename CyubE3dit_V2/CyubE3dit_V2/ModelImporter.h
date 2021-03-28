#pragma once
#include <string>

namespace wiScene
{
	struct Scene;
}

wiECS::Entity ImportModel_OBJ(const std::string& fileName, wiScene::Scene& scene, uint8_t windMode, float emissiveStrength = 0.0);
void ImportModel_GLTF(const std::string& fileName, wiScene::Scene& scene);

