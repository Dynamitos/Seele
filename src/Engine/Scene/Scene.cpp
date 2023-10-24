#include "Scene.h"
#include "Graphics/Graphics.h"
#include "Graphics/Mesh.h"
#include "Component/StaticMesh.h"
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
{
    ShaderBufferCreateInfo structInfo = {
        .resourceData = {
            .size = sizeof(Component::DirectionalLight) * MAX_DIRECTIONAL_LIGHTS,
            .data = nullptr,
        },
        .stride = sizeof(Component::DirectionalLight),
        .bDynamic = true,
    };
    lightEnv.directionalLights = graphics->createShaderBuffer(structInfo);
    structInfo.resourceData.size = sizeof(Component::PointLight) * MAX_POINT_LIGHTS;
    structInfo.stride = sizeof(Component::PointLight);
    lightEnv.pointLights = graphics->createShaderBuffer(structInfo);
    
    UniformBufferCreateInfo uniformInfo = {
        .resourceData = {
            .size = sizeof(uint32),
            .data = nullptr
        },
        .bDynamic = true
    };
    lightEnv.numDirectional = graphics->createUniformBuffer(uniformInfo);
    lightEnv.numPoints = graphics->createUniformBuffer(uniformInfo);
}

Scene::~Scene()
{
}

void Scene::update(float deltaTime)
{
    physics.update(deltaTime);
}

LightEnv Scene::getLightBuffer() 
{
    StaticArray<Component::DirectionalLight, MAX_DIRECTIONAL_LIGHTS> dirLights;
    uint32 numDirLights = 0;
    for(auto&& [entity, light] : registry.view<Component::DirectionalLight>().each())
    {
        dirLights[numDirLights++] = light;
    }
    lightEnv.directionalLights->updateContents({
        .size = sizeof(Component::DirectionalLight) * numDirLights,
        .data = (uint8*)dirLights.data(),
    });
    lightEnv.numDirectional->updateContents({
        .size = sizeof(uint32),
        .data = (uint8*)&numDirLights,
    });
    StaticArray<Component::PointLight, MAX_POINT_LIGHTS> pointLights;
    uint32 numPointLights = 0;
    for(auto&& [entity, light] : registry.view<Component::PointLight>().each())
    {
        pointLights[numPointLights++] = light;
    }
    lightEnv.pointLights->updateContents({
        .size = sizeof(Component::PointLight) * numPointLights,
        .data = (uint8*)pointLights.data(),
    });
    lightEnv.numPoints->updateContents({
        .size = sizeof(uint32),
        .data = (uint8*)&numPointLights,
    });
    return lightEnv;
}


Component::Skybox Scene::getSkybox()
{
    return Seele::Component::Skybox {
        .day = AssetRegistry::findTexture("FS000_Day_01")->getTexture().cast<Gfx::TextureCube>(),
        .night = AssetRegistry::findTexture("FS000_Night_01")->getTexture().cast<Gfx::TextureCube>(),
        .fogColor = Vector(0.2, 0.1, 0.6),
        .blendFactor = 0,
    };
}