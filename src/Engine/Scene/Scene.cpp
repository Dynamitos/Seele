#include "Scene.h"
#include "Graphics/Graphics.h"
#include "Graphics/Mesh.h"
#include "Component/Mesh.h"
#include "Component/Transform.h"
#include "Asset/AssetRegistry.h"
#include "Asset/TextureAsset.h"
#include "Asset/MaterialAsset.h"
#include "Component/PointLight.h"
#include "Component/DirectionalLight.h"
#include "Actor/PointLightActor.h"

using namespace Seele;

Scene::Scene(Gfx::PGraphics graphics)
    : graphics(graphics)
    , physics(registry)
    , lightEnv(new LightEnvironment(graphics))
{
}

Scene::~Scene()
{
}

void Scene::update(float deltaTime)
{
    lightEnv->reset();
    physics.update(deltaTime);
}

