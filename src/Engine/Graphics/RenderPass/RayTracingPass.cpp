#include "RayTracingPass.h"
#include "Asset/AssetRegistry.h"
#include "Asset/MeshAsset.h"
#include "Graphics/Mesh.h"
#include "Graphics/RayTracing.h"
#include "Graphics/Shader.h"
#include "Graphics/StaticMeshVertexData.h"
#include "RenderPass.h"

using namespace Seele;

RayTracingPass::RayTracingPass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics, scene) {
    paramsLayout = graphics->createDescriptorLayout("pRayTracingParams");
    paramsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
    });
    paramsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    });
    paramsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 2,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    paramsLayout->create();
    pipelineLayout = graphics->createPipelineLayout("RayTracing");
    pipelineLayout->addDescriptorLayout(viewParamsLayout);
    pipelineLayout->addDescriptorLayout(Material::getDescriptorLayout());
    pipelineLayout->addDescriptorLayout(paramsLayout);
    pipelineLayout->addDescriptorLayout(scene->getLightEnvironment()->getDescriptorLayout());
    pipelineLayout->addDescriptorLayout(StaticMeshVertexData::getInstance()->getVertexDataLayout());
    pipelineLayout->addDescriptorLayout(StaticMeshVertexData::getInstance()->getInstanceDataLayout());
    graphics->getShaderCompiler()->registerRenderPass("RayTracing", Gfx::PassConfig{
                                                                        .baseLayout = pipelineLayout,
                                                                        .mainFile = "ClosestHit",
                                                                        .useMaterial = true,
                                                                        .rayTracing = true,
                                                                    });
}

void RayTracingPass::beginFrame(const Component::Camera& cam) { RenderPass::beginFrame(cam); }

void RayTracingPass::render() {
    Array<Gfx::RayTracingHitGroup> callableGroups;
    Array<Gfx::PBottomLevelAS> accelerationStructures;
    Array<InstanceData> instanceData;

    for (VertexData* vertexData : VertexData::getList()) {
        auto& materialData = vertexData->getMaterialData();

        for (auto& matData : materialData) {
            if (matData.instances.size() == 0)
                continue;
            PMaterial mat = matData.material;

            Gfx::ShaderPermutation permutation = graphics->getShaderCompiler()->getTemplate("RayTracing");
            permutation.setMaterial(mat->getName());
            permutation.setVertexData(vertexData->getTypeName());

            const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(Gfx::PermutationId(permutation));
            assert(collection != nullptr);

            for (auto& inst : matData.instances) {
                for (uint32 i = 0; i < inst.instanceData.size(); ++i) {
                    Gfx::RayTracingHitGroup callableGroup = {
                        .closestHitShader = collection->callableShader,
                    };
                    callableGroup.parameters.resize(sizeof(VertexData::DrawCallOffsets));
                    std::memcpy(callableGroup.parameters.data(), &inst.offsets, sizeof(VertexData::DrawCallOffsets));
                    callableGroups.add(callableGroup);

                    instanceData.add(inst.instanceData[i]);
                    accelerationStructures.add(inst.rayTracingData[i]);
                }
            }
        }
        for (const auto& transparentData : vertexData->getTransparentData()) {
            PMaterial mat = transparentData.matInst->getBaseMaterial();

            Gfx::ShaderPermutation permutation = graphics->getShaderCompiler()->getTemplate("RayTracing");
            permutation.setMaterial(mat->getName());
            permutation.setVertexData(vertexData->getTypeName());
            const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(Gfx::PermutationId(permutation));
            assert(collection != nullptr);
            Gfx::RayTracingHitGroup callableGroup = {
                .closestHitShader = collection->callableShader,
            };
            callableGroup.parameters.resize(sizeof(VertexData::DrawCallOffsets));
            std::memcpy(callableGroup.parameters.data(), &transparentData.offsets, sizeof(VertexData::DrawCallOffsets));
            callableGroups.add(callableGroup);

            instanceData.add(transparentData.instanceData);
            accelerationStructures.add(transparentData.rayTracingScene);
        }
    }
    pipeline = graphics->createRayTracingPipeline(Gfx::RayTracingPipelineCreateInfo{
        .pipelineLayout = pipelineLayout, .rayGenGroup = {.shader = rayGen}, .hitGroups = callableGroups, .missGroups = {{.shader = miss}},
        //.callableGroups = callableGroups,
    });
    tlas = graphics->createTopLevelAccelerationStructure(Gfx::TopLevelASCreateInfo{
        .instances = instanceData,
        .bottomLevelStructures = accelerationStructures,
    });
    Gfx::PDescriptorSet desc = paramsLayout->allocateDescriptorSet();
    desc->updateAccelerationStructure(0, tlas);
    desc->updateTexture(1, Gfx::PTexture2D(texture));
    desc->updateBuffer(2, StaticMeshVertexData::getInstance()->getIndexBuffer());
    desc->writeChanges();

    Gfx::ORenderCommand command = graphics->createRenderCommand("RayTracing");
    command->bindPipeline(pipeline);
    StaticMeshVertexData::getInstance()->getInstanceDataSet()->writeChanges();
    StaticMeshVertexData::getInstance()->getVertexDataSet()->writeChanges();
    command->bindDescriptor({viewParamsSet, StaticMeshVertexData::getInstance()->getInstanceDataSet(),
                             StaticMeshVertexData::getInstance()->getVertexDataSet(), Material::getDescriptorSet(),
                             scene->getLightEnvironment()->getDescriptorSet(), desc});
    command->traceRays(texture->getWidth(), texture->getHeight(), 1);
    texture->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
                             Gfx::SE_ACCESS_TRANSFER_READ_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    Array<Gfx::ORenderCommand> commands;
    commands.add(std::move(command));
    graphics->executeCommands(std::move(commands));
    viewport->getOwner()->getBackBuffer()->changeLayout(Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, Gfx::SE_ACCESS_NONE,
                                                        Gfx::SE_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                                        Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    graphics->copyTexture(Gfx::PTexture2D(texture), viewport->getOwner()->getBackBuffer());
}

void RayTracingPass::endFrame() {}

void RayTracingPass::publishOutputs() {
    texture = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT,
    });
    texture->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_NONE, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                          Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                          Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
    ShaderCompilationInfo compileInfo = {
        .name = "RT",
        .modules = {"RayGen", "Miss"},
        .entryPoints = {{"raygen", "RayGen"}, {"miss", "Miss"}},
        .defines = {{"RAY_TRACING", "1"}},
        .rootSignature = pipelineLayout,
    };
    graphics->beginShaderCompilation(compileInfo);
    rayGen = graphics->createRayGenShader({0});
    miss = graphics->createMissShader({1});
    pipelineLayout->create();
}

void RayTracingPass::createRenderPass() {}
