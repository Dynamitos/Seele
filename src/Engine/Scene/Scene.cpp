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
}

Scene::~Scene()
{
}

void Scene::tick(float deltaTime)
{
    for (auto actor : rootActors)
    {
        actor->tick(deltaTime);
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
    for(auto batch : comp->staticMeshes)
    {
        PrimitiveUniformBuffer data;
        data.actorWorldPosition = Vector4(comp->getTransform().getPosition(), 1);
        data.localToWorld = comp->getRenderMatrix();
        data.worldToLocal = glm::inverse(data.localToWorld);
        BulkResourceData createInfo;
        createInfo.data = reinterpret_cast<uint8*>(&data);
        createInfo.owner = Gfx::QueueType::GRAPHICS;
        createInfo.size = sizeof(data);
        Gfx::PUniformBuffer uniformBuffer = graphics->createUniformBuffer(createInfo);
        for(auto& element : batch.elements)
        {
            element.uniformBuffer = uniformBuffer;
        }
        staticMeshes.add(batch);
    }
}
