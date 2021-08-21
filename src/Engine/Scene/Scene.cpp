#include "Scene.h"
#include "Components/PrimitiveComponent.h"
#include "Components/PrimitiveUniformBufferLayout.h"
#include "Material/MaterialInstance.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"

using namespace Seele;

inline float frand()
{
    return (float)rand()/RAND_MAX;
}

Scene::Scene(Gfx::PGraphics graphics)
    : graphics(graphics)
    , updater(new SceneUpdater())
{
    lightEnv.directionalLights[0].color = Vector4(1, 1, 1, 1);
    lightEnv.directionalLights[0].direction = Vector4(1, 1, 0, 1);
    lightEnv.directionalLights[0].intensity = Vector4(1, 1, 1, 1);
    lightEnv.numDirectionalLights = 0;
    srand((unsigned int)time(NULL));
    for(uint32 i = 0; i < 16; ++i)
    {
        lightEnv.pointLights[i].colorRange = Vector4(frand(), frand(), frand(), frand() * 30);
        lightEnv.pointLights[i].positionWS = Vector4(frand() * 100-50, frand(), frand() * 100-50, 1);
    }
    lightEnv.numPointLights = 16;
    lightEnv.pointLights[0].colorRange = Vector4(1, 0, 0, 100);
    lightEnv.pointLights[0].positionWS = Vector4(0, 10, 0, 1);
}

Scene::~Scene()
{
}

void Scene::tick(double deltaTime)
{
    updater->runUpdates(static_cast<float>(deltaTime));
}

void Scene::addActor(PActor actor)
{
    rootActors.add(actor);
    actor->notifySceneAttach(this);
}

void Scene::addPrimitiveComponent(PPrimitiveComponent comp)
{
    primitives.add(comp);
    for(auto& batch : comp->getStaticMeshes())
    {
        PrimitiveUniformBuffer data;
        data.actorWorldPosition = Vector4(comp->getTransform().getPosition(), 1);
        data.localToWorld = comp->getRenderMatrix();
        data.worldToLocal = glm::inverse(data.localToWorld);
        UniformBufferCreateInfo createInfo;
        createInfo.resourceData.data = reinterpret_cast<uint8*>(&data);
        createInfo.resourceData.owner = Gfx::QueueType::GRAPHICS;
        createInfo.resourceData.size = sizeof(data);
        createInfo.bDynamic = true;
        Gfx::PUniformBuffer uniformBuffer = graphics->createUniformBuffer(createInfo);
        for(auto& element : batch.elements)
        {
            element.uniformBuffer = uniformBuffer;
        }
        staticMeshes.add(batch);
    }
}
