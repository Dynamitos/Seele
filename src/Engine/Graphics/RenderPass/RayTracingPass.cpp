#include "RayTracingPass.h"
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
    graphics->getShaderCompiler()->registerRenderPass("RayTracing", Gfx::PassConfig{
                                                                        .baseLayout = pipelineLayout,
                                                                        .mainFile = "ClosestHit",
                                                                        .useMaterial = true,
                                                                        .rayTracing = true,
                                                                    });
}

void RayTracingPass::beginFrame(const Component::Camera& cam) { RenderPass::beginFrame(cam); }

void RayTracingPass::render() {
    Gfx::ORenderCommand command = graphics->createRenderCommand("RayTracing");
    Array<Gfx::RayTracingHitGroup> hitgroups;
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
                    Gfx::RayTracingHitGroup hitgroup = {
                        .closestHitShader = collection->closestHitShader,
                        .anyHitShader = nullptr,
                        .intersectionShader = nullptr,
                    };
                    hitgroup.parameters.resize(sizeof(VertexData::DrawCallOffsets));
                    std::memcpy(hitgroup.parameters.data(), &inst.offsets, sizeof(VertexData::DrawCallOffsets));
                    hitgroups.add(hitgroup);

                    instanceData.add(inst.instanceData[i]);
                    accelerationStructures.add(inst.rayTracingData[i]);
                }
            }
        }
    }
    Gfx::OTopLevelAS tlas = graphics->createTopLevelAccelerationStructure(Gfx::TopLevelASCreateInfo{
        .instances = instanceData,
        .bottomLevelStructures = accelerationStructures,
    });
    Gfx::PDescriptorSet desc = paramsLayout->allocateDescriptorSet();
    desc->updateAccelerationStructure(0, tlas);
    desc->updateTexture(1, Gfx::PTexture2D(texture));
    desc->updateBuffer(2, StaticMeshVertexData::getInstance()->getIndexBuffer());
    desc->writeChanges();

    Gfx::PRayTracingPipeline pipeline = graphics->createRayTracingPipeline(Gfx::RayTracingPipelineCreateInfo{
        .pipelineLayout = pipelineLayout,
        .rayGenShader = raygen,
        .hitgroups = hitgroups,
        .missShaders = {miss},
    });

    command->bindPipeline(pipeline);
    command->bindDescriptor({viewParamsSet, StaticMeshVertexData::getInstance()->getInstanceDataSet(),
                             StaticMeshVertexData::getInstance()->getVertexDataSet(), Material::getDescriptorSet(),
                             scene->getLightEnvironment()->getDescriptorSet()});
    command->traceRays(texture->getWidth(), texture->getHeight(), 1);
    Array<Gfx::ORenderCommand> commands;
    commands.add(std::move(command));
    graphics->executeCommands(std::move(commands));
}

void RayTracingPass::endFrame() {}

void RayTracingPass::publishOutputs() {
    ShaderCompilationInfo createInfo{
        .name = "RayGen",
        .modules = {"RayGen", "Miss"},
        .entryPoints = {{"main", "RayGen"}, {"main", "Miss"}},
        .rootSignature = pipelineLayout,
    };
    graphics->beginShaderCompilation(createInfo);
    raygen = graphics->createRayGenShader({0});
    miss = graphics->createMissShader({1});

    texture = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT,
    });
}

void RayTracingPass::createRenderPass() {}
