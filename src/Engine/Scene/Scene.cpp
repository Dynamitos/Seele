#include "Scene.h"
#include "Components/PrimitiveComponent.h"
#include "Components/PrimitiveUniformBufferLayout.h"
#include "Material/MaterialInstance.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"

using namespace Seele;

Scene::Scene(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    lightEnv.directionalLights[0].color = Vector4(1, 0, 0, 1);
    lightEnv.directionalLights[0].direction = Vector4(0, 0, 0, 1);
    lightEnv.directionalLights[0].intensity = Vector4(1, 1, 1, 1);
    lightEnv.numDirectionalLights = 1;
    lightEnv.numPointLights = 0;
    srand((unsigned int)time(NULL));
}

Scene::~Scene()
{
}

void Scene::tick(double deltaTime)
{
    lightEnv.directionalLights[0].direction.x += ((rand() / (double)RAND_MAX) - 0.5f) * 100.f * deltaTime;
    lightEnv.directionalLights[0].direction.y += ((rand() / (double)RAND_MAX) - 0.5f) * 100.f * deltaTime;
    lightEnv.directionalLights[0].direction.z += ((rand() / (double)RAND_MAX) - 0.5f) * 100.f * deltaTime;
    std::cout << lightEnv.directionalLights[0].direction << std::endl;
    for (auto actor : rootActors)
    {
        actor->tick(static_cast<float>(deltaTime));
    }
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
