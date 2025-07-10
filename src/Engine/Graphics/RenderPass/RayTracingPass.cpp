#include "RayTracingPass.h"
#include "Asset/AssetRegistry.h"
#include "Asset/EnvironmentMapAsset.h"
#include "Graphics/RayTracing.h"
#include "Graphics/Shader.h"
#include "Graphics/StaticMeshVertexData.h"
#include "RenderPass.h"

using namespace Seele;

struct SampleParams {
    uint32 pass;
    uint32 samplesPerPixel;
};

RayTracingPass::RayTracingPass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics), scene(scene) {
    paramsLayout = graphics->createDescriptorLayout("pRayTracingParams");
    paramsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = TLAS_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
    });
    paramsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = ACCUMULATOR_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    });
    paramsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = TEXTURE_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    });
    paramsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = INDEXBUFFER_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    paramsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = SKYBOX_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    paramsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = SKYSAMPLER_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    paramsLayout->create();
    pipelineLayout = graphics->createPipelineLayout("RayTracing");
    pipelineLayout->addDescriptorLayout(viewParamsLayout);
    pipelineLayout->addDescriptorLayout(Material::getDescriptorLayout());
    pipelineLayout->addDescriptorLayout(paramsLayout);
    pipelineLayout->addDescriptorLayout(scene->getLightEnvironment()->getDescriptorLayout());
    pipelineLayout->addDescriptorLayout(StaticMeshVertexData::getInstance()->getVertexDataLayout());
    pipelineLayout->addDescriptorLayout(StaticMeshVertexData::getInstance()->getInstanceDataLayout());
    pipelineLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_SHADER_STAGE_RAYGEN_BIT_KHR,
        .offset = 0,
        .size = sizeof(SampleParams),
    });
    graphics->getShaderCompiler()->registerRenderPass("RayTracing", Gfx::PassConfig{
                                                                        .baseLayout = pipelineLayout,
                                                                        .mainFile = "ClosestHit",
                                                                        .useMaterial = true,
                                                                        .rayTracing = true,
                                                                    });
    skyBox = AssetRegistry::findEnvironmentMap("", "newport_loft")->getSkybox();
    skyBoxSampler = graphics->createSampler({});
}

static uint32 pass = 0;
static Component::Transform lastCam;
void RayTracingPass::beginFrame(const Component::Camera& cam, const Component::Transform& transform) {
    viewParamsSet = createViewParamsSet(cam, transform);
    if (lastCam.getPosition() != transform.getPosition() || lastCam.getForward() != transform.getForward()) {
        lastCam = transform;
        pass = 0;
    }
}

void RayTracingPass::render() {
    graphics->beginDebugRegion("RayTracingPass");
    texture->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
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
            permutation.setMaterial(mat->getName(), mat->getProfile());
            permutation.setVertexData(vertexData->getTypeName());

            const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(Gfx::PermutationId(permutation));
            assert(collection != nullptr);

            for (auto& inst : matData.instances) {
                for (uint32 i = 0; i < inst.instanceData.size(); ++i) {
                    Gfx::RayTracingHitGroup callableGroup = {
                        .closestHitShader = collection->callableShader,
                        .anyHitShader = anyhit,
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
            permutation.setMaterial(mat->getName(), mat->getProfile());
            permutation.setVertexData(vertexData->getTypeName());
            const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(Gfx::PermutationId(permutation));
            assert(collection != nullptr);
            Gfx::RayTracingHitGroup callableGroup = {
                .closestHitShader = collection->callableShader,
                .anyHitShader = anyhit,
            };
            callableGroup.parameters.resize(sizeof(VertexData::DrawCallOffsets));
            std::memcpy(callableGroup.parameters.data(), &transparentData.offsets, sizeof(VertexData::DrawCallOffsets));
            callableGroups.add(callableGroup);

            instanceData.add(transparentData.instanceData);
            accelerationStructures.add(transparentData.rayTracingScene);
        }
    }
    pipeline = graphics->createRayTracingPipeline(Gfx::RayTracingPipelineCreateInfo{
        .pipelineLayout = pipelineLayout,
        .rayGenGroup =
            {
                .shader = rayGen,
            },
        .hitGroups = callableGroups,
        .missGroups =
            {
                {
                    .shader = miss,
                },
            },
        //.callableGroups = callableGroups,
    });
    tlas = graphics->createTopLevelAccelerationStructure(Gfx::TopLevelASCreateInfo{
        .instances = instanceData,
        .bottomLevelStructures = accelerationStructures,
    });
    Gfx::PDescriptorSet desc = paramsLayout->allocateDescriptorSet();
    desc->updateAccelerationStructure(TLAS_NAME, 0, tlas);
    desc->updateTexture(ACCUMULATOR_NAME, 0, radianceAccumulator->getDefaultView());
    desc->updateTexture(TEXTURE_NAME, 0, texture->getDefaultView());
    desc->updateBuffer(INDEXBUFFER_NAME, 0, StaticMeshVertexData::getInstance()->getIndexBuffer());
    desc->updateTexture(SKYBOX_NAME, 0, skyBox->getDefaultView());
    desc->updateSampler(SKYSAMPLER_NAME, 0, skyBoxSampler);
    desc->writeChanges();

    Gfx::ORenderCommand command = graphics->createRenderCommand("RayTracing");
    command->bindPipeline(pipeline);
    StaticMeshVertexData::getInstance()->getInstanceDataSet()->writeChanges();
    StaticMeshVertexData::getInstance()->getVertexDataSet()->writeChanges();
    command->bindDescriptor({viewParamsSet, StaticMeshVertexData::getInstance()->getInstanceDataSet(),
                             StaticMeshVertexData::getInstance()->getVertexDataSet(), Material::getDescriptorSet(),
                             scene->getLightEnvironment()->getDescriptorSet(), desc});
    SampleParams sampleParams = {
        .pass = 0,
        .samplesPerPixel = 10000,
    };
    // for (uint32 i = 0; i < sampleParams.samplesPerPixel; ++i)
    {
        sampleParams.pass = pass;
        command->pushConstants(Gfx::SE_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(SampleParams), &sampleParams);
        command->traceRays(texture->getWidth(), texture->getHeight(), 1);
        radianceAccumulator->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
                                             Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
        texture->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
                                 Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
    }
    pass++;
    std::cout << pass << std::endl;
    Array<Gfx::ORenderCommand> commands;
    commands.add(std::move(command));
    graphics->executeCommands(std::move(commands));
    graphics->endDebugRegion();
}

void RayTracingPass::endFrame() {}

void RayTracingPass::publishOutputs() {
    radianceAccumulator = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT,
    });
    radianceAccumulator->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_NONE, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                      Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
    texture = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT,
    });
    texture->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_NONE, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                          Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                          Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
    resources->registerRenderPassOutput("BASEPASS_COLOR", Gfx::RenderTargetAttachment(texture->getDefaultView(), Gfx::SE_IMAGE_LAYOUT_UNDEFINED,
                                                                                      Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
    ShaderCompilationInfo compileInfo = {
        .name = "RayGenMiss",
        .modules = {"RayGen", "AnyHit", "Miss"},
        .entryPoints = {{"raygen", "RayGen"}, {"anyhit", "AnyHit"}, {"miss", "Miss"}},
        .defines = {{"RAY_TRACING", "1"}},
        .rootSignature = pipelineLayout,
    };
    graphics->beginShaderCompilation(compileInfo);
    rayGen = graphics->createRayGenShader({0});
    anyhit = graphics->createAnyHitShader({1});
    miss = graphics->createMissShader({2});
    pipelineLayout->create();
}

void RayTracingPass::createRenderPass() {}
