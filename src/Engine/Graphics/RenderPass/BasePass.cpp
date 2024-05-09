#include "BasePass.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/Shader.h"
#include "Window/Window.h"
#include "Component/Camera.h"
#include "Component/Mesh.h"
#include "Actor/CameraActor.h"
#include "Math/Vector.h"
#include "RenderGraph.h"
#include "Material/MaterialInstance.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Command.h"
#include "Graphics/StaticMeshVertexData.h"

using namespace Seele;

BasePass::BasePass(Gfx::PGraphics graphics, PScene scene) 
    : RenderPass(graphics, scene)
{
    basePassLayout = graphics->createPipelineLayout("BasePassLayout");

    basePassLayout->addDescriptorLayout(viewParamsLayout);
    basePassLayout->addDescriptorLayout(scene->getLightEnvironment()->getDescriptorLayout());

    lightCullingLayout = graphics->createDescriptorLayout("pLightCullingData");
    // oLightIndexList
    lightCullingLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 0, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, });
    // oLightGrid
    lightCullingLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 1, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE, });
    lightCullingLayout->create();

    basePassLayout->addDescriptorLayout(lightCullingLayout);

    if (graphics->supportMeshShading())
    {
        graphics->getShaderCompiler()->registerRenderPass(basePassLayout, "BasePass", "MeshletPass", true, true, "BasePass", true, true, "MeshletPass");
    }
    else
    {
        graphics->getShaderCompiler()->registerRenderPass(basePassLayout, "BasePass", "LegacyPass", true, true, "BasePass");
    }
}

BasePass::~BasePass()
{
}

void BasePass::beginFrame(const Component::Camera& cam) 
{
    RenderPass::beginFrame(cam);

    lightCullingLayout->reset();
    opaqueCulling = lightCullingLayout->allocateDescriptorSet();
    transparentCulling = lightCullingLayout->allocateDescriptorSet();
}

void BasePass::render() 
{
    oLightIndexList->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    tLightIndexList->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    oLightGrid->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    tLightGrid->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    opaqueCulling->updateBuffer(0, oLightIndexList);
    opaqueCulling->updateTexture(1, oLightGrid);
    transparentCulling->updateBuffer(0, tLightIndexList);
    transparentCulling->updateTexture(1, tLightGrid);
    opaqueCulling->writeChanges();
    transparentCulling->writeChanges();

    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;

        Gfx::ShaderPermutation permutation;
        if (graphics->supportMeshShading())
        {
            permutation.setTaskFile("MeshletPass");
            permutation.setMeshFile("MeshletPass");
        }
        else
        {
            permutation.setVertexFile("LegacyPass");
        }
        permutation.setFragmentFile("BasePass");
        for (VertexData* vertexData : VertexData::getList())
        {
            permutation.setVertexData(vertexData->getTypeName());
            const auto& materials = vertexData->getMaterialData();
            for (const auto& materialData : materials)
            {
                // Create Pipeline(Material, VertexData)
                // Descriptors:
                // ViewData => global, static
                // VertexData => per meshtype
                // SceneData => per material instance
                // LightEnv => provided by scene
                // Material => per material
                // LightCulling => calculated by pass
                permutation.setMaterial(materialData.material->getName());
                Gfx::PermutationId id(permutation);

                Gfx::ORenderCommand command = graphics->createRenderCommand("BaseRender");
                command->setViewport(viewport);

                const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(id);
                assert(collection != nullptr);
                if (graphics->supportMeshShading())
                {
                    Gfx::MeshPipelineCreateInfo pipelineInfo;
                    pipelineInfo.taskShader = collection->taskShader;
                    pipelineInfo.meshShader = collection->meshShader;
                    pipelineInfo.fragmentShader = collection->fragmentShader;
                    pipelineInfo.pipelineLayout = collection->pipelineLayout;
                    pipelineInfo.renderPass = renderPass;
                    pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_GREATER_OR_EQUAL;
                    pipelineInfo.multisampleState.samples = viewport->getSamples();
                    pipelineInfo.colorBlend.attachmentCount = 1;
                    Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                    command->bindPipeline(pipeline);
                }
                else
                {
                    Gfx::LegacyPipelineCreateInfo pipelineInfo;
                    pipelineInfo.vertexShader = collection->vertexShader;
                    pipelineInfo.fragmentShader = collection->fragmentShader;
                    pipelineInfo.pipelineLayout = collection->pipelineLayout;
                    pipelineInfo.renderPass = renderPass;
                    pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_GREATER_OR_EQUAL;
                    pipelineInfo.multisampleState.samples = viewport->getSamples();
                    pipelineInfo.colorBlend.attachmentCount = 1;
                    Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                    command->bindPipeline(pipeline);
                }
                command->bindDescriptor(vertexData->getVertexDataSet());
                command->bindDescriptor(viewParamsSet);
                command->bindDescriptor(scene->getLightEnvironment()->getDescriptorSet());
                command->bindDescriptor(opaqueCulling);
                for (const auto& instance : materialData.instances)
                {
                    command->bindDescriptor(instance.materialInstance->getDescriptorSet());
                    command->bindDescriptor(vertexData->getInstanceDataSet(), {instance.descriptorOffset, instance.descriptorOffset});
                    if (graphics->supportMeshShading())
                    {
                        command->drawMesh(vertexData->getMeshData(instance.meshId).size(), 1, 1);
                    }
                    else
                    {
                        //command->bindIndexBuffer(vertexData->getIndexBuffer());
                        //uint32 instanceOffset = 0;
                        //for (const auto& meshData : vertexData->getMeshData(instance.meshId))
                        //{
                        //    if (meshData.numIndices > 0)
                        //    {
                        //        command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex, meshData.indicesOffset, instanceOffset);
                        //    }
                        //    instanceOffset++;
                        //}
                    }
                }
                commands.add(std::move(command));
            }
        }
    
    
    graphics->executeCommands(std::move(commands));
    graphics->endRenderPass();
}

void BasePass::endFrame() 
{
}

void BasePass::publishOutputs() 
{
    colorAttachment = Gfx::RenderTargetAttachment(viewport,
        Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        Gfx::SE_ATTACHMENT_LOAD_OP_LOAD, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    resources->registerRenderPassOutput("BASEPASS_COLOR", colorAttachment);
}

void BasePass::createRenderPass() 
{
    depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH");
    meshletIdAttachment = resources->requestRenderTarget("BASEPASS_MESHLETID");
    depthAttachment.setLoadOp(Gfx::SE_ATTACHMENT_LOAD_OP_LOAD);
    depthAttachment.setInitialLayout(Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    depthAttachment.setFinalLayout(Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{
        .colorAttachments = { colorAttachment, meshletIdAttachment }, 
        .depthAttachment = depthAttachment,
    };
    Array<Gfx::SubPassDependency> dependency = {
        {
            .srcSubpass = ~0U,
            .dstSubpass = 0,
            .srcStage = Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
        {
            .srcSubpass = ~0U,
            .dstSubpass = 0,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        },
        {
            .srcSubpass = 0,
            .dstSubpass = ~0U,
            .srcStage = Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
        {
            .srcSubpass = 0,
            .dstSubpass = ~0U,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_NONE,
        },
    };
    renderPass = graphics->createRenderPass(std::move(layout), std::move(dependency), viewport);
    oLightIndexList = resources->requestBuffer("LIGHTCULLING_OLIGHTLIST");
    tLightIndexList = resources->requestBuffer("LIGHTCULLING_TLIGHTLIST");
    oLightGrid = resources->requestTexture("LIGHTCULLING_OLIGHTGRID");
    tLightGrid = resources->requestTexture("LIGHTCULLING_TLIGHTGRID");
}
