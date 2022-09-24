#include "Scene.h"
#include "Components/PrimitiveComponent.h"
#include "Material/MaterialAsset.h"
#include "Graphics/Graphics.h"

using namespace Seele;

inline float frand()
{
    return (float)rand()/RAND_MAX;
}

Scene::Scene(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    lightEnv.directionalLights[0].color = Vector4(1, 1, 1, 1);
    lightEnv.directionalLights[0].direction = Vector4(1, 1, 0, 1);
    lightEnv.directionalLights[0].intensity = Vector4(1, 1, 1, 1);
    lightEnv.numDirectionalLights = 1;
    srand((unsigned int)time(NULL));
    for(uint32 i = 0; i < 16; ++i)
    {
        lightEnv.pointLights[i].colorRange = Vector4(frand(), frand(), frand(), frand() * 300);
        lightEnv.pointLights[i].positionWS = Vector4(frand() * 100-50, frand(), frand() * 100-50, 1);
    }
    lightEnv.numPointLights = 16;
    lightEnv.pointLights[0].colorRange = Vector4(1, 0, 1, 1000);
    lightEnv.pointLights[0].positionWS = Vector4(0, 10, 0, 1);
}

Scene::~Scene()
{
}

void Scene::start() 
{
    for(auto actor : rootActors)
    {
        actor->launchStart();
    }
}

static int64 lastUpdate;
static uint64 numUpdates;

void Scene::beginUpdate(double deltaTime)
{
    //std::cout << "Scene::beginUpdate" << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();
    for(auto actor : rootActors)
    {
        actor->launchTick(static_cast<float>(deltaTime));
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    int64 delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    lastUpdate += delta;
    numUpdates++;
    if(lastUpdate > 1000000)
    {
        lastUpdate -= 1000000;
        std::cout << numUpdates << " updates per second" << std::endl;
        numUpdates = 0;
    }
}

void Scene::commitUpdate() 
{
    //std::cout << "Scene::commitUpdate" << std::endl;
    for(auto actor : rootActors)
    {
        actor->launchUpdate();
    }
    //std::cout << "Scene::commitUpdate finished waiting" << std::endl;
}

void Scene::addActor(PActor actor)
{
    rootActors.push_back(actor);
    actor->notifySceneAttach(this);
}

void Scene::addPrimitiveComponent(PPrimitiveComponent comp)
{
    primitives.push_back(comp);
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
        staticMeshes.push_back(batch);
    }
}
