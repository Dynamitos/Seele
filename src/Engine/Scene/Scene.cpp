#include "Scene.h"
#include "Actor/PointLightActor.h"
#include "Asset/AssetRegistry.h"
#include "Asset/MaterialAsset.h"
#include "Asset/TextureAsset.h"
#include "Component/DirectionalLight.h"
#include "Component/Mesh.h"
#include "Component/PointLight.h"
#include "Component/Transform.h"
#include "Graphics/Graphics.h"
#include "Graphics/Mesh.h"
#include "Graphics/StaticMeshVertexData.h"

using namespace Seele;

Scene::Scene(Gfx::PGraphics graphics) : graphics(graphics), lightEnv(new LightEnvironment(graphics)), physics(registry) {}

Scene::~Scene() {}

void Scene::bakeLighting() { 
	const auto& matData = StaticMeshVertexData::getInstance()->getMaterialData(); 
	for (const auto& mat : matData)
	{
		
	}
}

void Scene::update(float deltaTime) { physics.update(deltaTime); }
