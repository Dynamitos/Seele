#include "Scene.h"
#include "Graphics/Graphics.h"
#include "Graphics/Mesh.h"
#include "Component/StaticMesh.h"
#include "Component/Transform.h"
#include "Asset/AssetRegistry.h"
#include "Asset/TextureAsset.h"
#include "Asset/MaterialAsset.h"

using namespace Seele;

inline float frand()
{
    return (float)rand()/RAND_MAX;
}

Scene::Scene(Gfx::PGraphics graphics)
    : graphics(graphics)
    , physics(registry)
{
    
    StructuredBufferCreateInfo structInfo = {
        .resourceData = {
            .size = sizeof(DirectionalLight) * MAX_DIRECTIONAL_LIGHTS,
            .data = nullptr,
        },
        .stride = sizeof(DirectionalLight),
        .bDynamic = true,
    };
    lightEnv.directionalLights = graphics->createStructuredBuffer(structInfo);
    structInfo.resourceData.size = sizeof(PointLight) * MAX_POINT_LIGHTS;
    lightEnv.pointLights = graphics->createStructuredBuffer(structInfo);
    
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

Array<MeshBatch> Scene::getStaticMeshes()
{
    Array<MeshBatch> result;
    auto view = registry.view<Component::StaticMesh, Component::Transform>();
    uint32 sceneDataIndex = 0;
    sceneData.clear();
    for(auto&& [entity, mesh, transform] : view.each())
    {
        sceneData.add(PrimitiveSceneData {
            .localToWorld = transform.toMatrix(),
            .worldToLocal = glm::inverse(transform.toMatrix()),
            .actorLocation = Vector4(transform.getPosition(), 1.0f)
        });
        for(auto& m : mesh.mesh->meshes)
        {
            auto& batch = result.add();
            batch.material = m->referencedMaterial->getMaterial();
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
            batchElement.isInstanced = false;
            batchElement.numInstances = 1;
            batchElement.sceneDataIndex = sceneDataIndex;
            batch.elements.add(batchElement);
        }
        sceneDataIndex++;
    }
    if(sceneDataBuffer == nullptr || sceneDataBuffer->getNumElements() != sceneData.size())
    {
        StructuredBufferCreateInfo createInfo = StructuredBufferCreateInfo {
            .resourceData = {
                .size = sceneData.size() * sizeof(PrimitiveSceneData),
                .data = nullptr,
            },
            .stride = (uint32)sceneData.size(),
        };
        sceneDataBuffer = graphics->createStructuredBuffer(createInfo);
    }
    sceneDataBuffer->updateContents(BulkResourceData {
        .size = sceneData.size() * sizeof(PrimitiveSceneData),
        .data = (uint8*)sceneData.data(),
    });
    return result;
}

LightEnv Scene::getLightBuffer() 
{
    StaticArray<DirectionalLight, MAX_DIRECTIONAL_LIGHTS> dirLights;
    uint32 numDirLights;
    for(auto&& [entity, light] : registry.view<DirectionalLight>().each())
    {
        dirLights[numDirLights++] = light;
    }
    lightEnv.directionalLights->updateContents({
        .size = sizeof(DirectionalLight) * numDirLights,
        .data = (uint8*)dirLights.data(),
    });
    lightEnv.numDirectional->updateContents({
        .size = sizeof(uint32),
        .data = (uint8*)&numDirLights,
    });
    StaticArray<PointLight, MAX_POINT_LIGHTS> pointLights;
    uint32 numPointLights;
    for(auto&& [entity, light] : registry.view<PointLight>().each())
    {
        pointLights[numPointLights++] = light;
    }
    lightEnv.pointLights->updateContents({
        .size = sizeof(PointLight) * numPointLights,
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