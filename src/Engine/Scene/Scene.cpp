#include "Scene.h"
#include "Material/MaterialAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Mesh.h"
#include "Component/StaticMesh.h"
#include "Component/Transform.h"

using namespace Seele;

inline float frand()
{
    return (float)rand()/RAND_MAX;
}

Scene::Scene(Gfx::PGraphics graphics)
    : graphics(graphics)
{
}

Scene::~Scene()
{
}

void Scene::start() 
{
    //for(auto actor : rootActors)
    //{
    //    actor->launchStart();
    //}
}

static int64 lastUpdate;
static uint64 numUpdates;

void Scene::beginUpdate(double)
{
    //std::cout << "Scene::beginUpdate" << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();
    // TODO
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
    //for(auto actor : rootActors)
    //{
    //    actor->launchUpdate();
    //}
    //std::cout << "Scene::commitUpdate finished waiting" << std::endl;
}

Array<StaticMeshBatch> Scene::getStaticMeshes()
{
    Array<StaticMeshBatch> result;
    auto view = registry.view<Component::StaticMesh, Component::Transform>();
    struct PrimitiveSceneData
    {
        Math::Matrix4 localToWorld;
        Math::Matrix4 worldToLocal;
        Math::Vector4 actorLocation;
    };
    for(auto&& [entity, mesh, transform] : view.each())
    {
        PrimitiveSceneData sceneData = {
            .localToWorld = transform.toMatrix(),
            .worldToLocal = glm::inverse(transform.toMatrix()),
            .actorLocation = Math::Vector4(transform.getPosition(), 1.0f)
        };
        UniformBufferCreateInfo info = {
            .resourceData = {
                .size = sizeof(PrimitiveSceneData),
                .data = (uint8_t*)&sceneData,
            }
        };
        Gfx::PUniformBuffer transformBuf = graphics->createUniformBuffer(info);
        // TODO
        for(auto& m : mesh.mesh->meshes)
        {
            auto& batch = result.add();
            batch.material = m->referencedMaterial;
            batch.isBackfaceCullingDisabled = false;
            batch.isCastingShadow = true;
            batch.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            batch.useReverseCulling = false;
            batch.useWireframe = false;
            batch.vertexInput = m->vertexInput;
            MeshBatchElement batchElement;
            batchElement.baseVertexIndex = 0;
            batchElement.firstIndex = 0;
            batchElement.indexBuffer = m->indexBuffer;
            batchElement.indirectArgsBuffer = nullptr;
            batchElement.instanceRuns = nullptr;
            batchElement.isInstanced = false;
            batchElement.numInstances = 1;
            batchElement.uniformBuffer = transformBuf;
            batchElement.numPrimitives = static_cast<uint32>(m->indexBuffer->getNumIndices() / 3); //TODO: hardcoded
            batch.elements.add(batchElement);
        }
    }
    return result;
}

LightEnv Scene::getLightBuffer() const 
{
    LightEnv result;
    result.directionalLights[0].color = Math::Vector4(0.4, 0.3, 0.5, 1.0);
    result.directionalLights[0].direction = Math::Vector4(0.5, 0.5, 0, 0);
    result.directionalLights[0].intensity = Math::Vector4(1.0, 0.9, 0.7, 0.5);\
    result.numDirectionalLights = 1;
    result.numPointLights = 0;
    return result;
}