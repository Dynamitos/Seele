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

using namespace Seele;

BasePass::BasePass(Gfx::PGraphics graphics, PScene scene) 
    : RenderPass(graphics, scene)
    , descriptorSets(6)
{
    basePassLayout = graphics->createPipelineLayout();

    basePassLayout->addDescriptorLayout(INDEX_VIEW_PARAMS, viewParamsLayout);
    basePassLayout->addDescriptorLayout(INDEX_LIGHT_ENV, scene->getLightEnvironment()->getDescriptorLayout());

    lightCullingLayout = graphics->createDescriptorLayout("BasePassLightCulling");
    // oLightIndexList
    lightCullingLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    // tLightIndexList
    lightCullingLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    // oLightGrid
    lightCullingLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    // tLightGrid
    lightCullingLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    lightCullingLayout->create();

    basePassLayout->addDescriptorLayout(INDEX_LIGHT_CULLING, lightCullingLayout);
    if (graphics->supportMeshShading())
    {
        graphics->getShaderCompiler()->registerRenderPass("BasePass", "MeshletBasePass", true, true, "BasePass", true, true, "MeshletBasePass");
    }
    else
    {
        graphics->getShaderCompiler()->registerRenderPass("BasePass", "LegacyBasePass", true, true, "BasePass");
    }
}

BasePass::~BasePass()
{
}

void BasePass::beginFrame(const Component::Camera& cam) 
{
    RenderPass::beginFrame(cam);

    lightCullingLayout->reset();
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
    depthAttachment.getTexture()->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

    descriptorSets[INDEX_VIEW_PARAMS] = viewParamsSet;
    descriptorSets[INDEX_LIGHT_ENV] = scene->getLightEnvironment()->getDescriptorSet();
    descriptorSets[INDEX_LIGHT_CULLING] = lightCullingLayout->allocateDescriptorSet();

    descriptorSets[INDEX_LIGHT_CULLING]->updateBuffer(0, oLightIndexList);
    descriptorSets[INDEX_LIGHT_CULLING]->updateBuffer(1, tLightIndexList);
    descriptorSets[INDEX_LIGHT_CULLING]->updateTexture(2, oLightGrid);
    descriptorSets[INDEX_LIGHT_CULLING]->updateTexture(3, tLightGrid);
    descriptorSets[INDEX_LIGHT_CULLING]->writeChanges();

    Gfx::ShaderPermutation permutation;
    if (graphics->supportMeshShading())
    {
        permutation.setTaskFile("MeshletBasePass");
        permutation.setMeshFile("MeshletBasePass");
    }
    else
    {
        permutation.setVertexFile("LegacyBasePass");
    }
    permutation.setFragmentFile("BasePass");
    graphics->beginRenderPass(renderPass);
    Array<Gfx::PRenderCommand> commands;
    for (VertexData* vertexData : VertexData::getList())
    {
        permutation.setVertexData(vertexData->getTypeName());
        const auto& materials = vertexData->getMaterialData();
        for (const auto& [_, materialData] : materials)
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

            Gfx::PRenderCommand command = graphics->createRenderCommand("DepthRender");
            command->setViewport(viewport);
            Gfx::OPipelineLayout layout = graphics->createPipelineLayout(basePassLayout);
            layout->addDescriptorLayout(INDEX_MATERIAL, materialData.material->getDescriptorLayout());
            layout->addDescriptorLayout(INDEX_VERTEX_DATA, vertexData->getVertexDataLayout());
            layout->addDescriptorLayout(INDEX_SCENE_DATA, vertexData->getInstanceDataLayout());
            layout->create();

            const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(id);
            assert(collection != nullptr);
            if (graphics->supportMeshShading())
            {
                Gfx::MeshPipelineCreateInfo pipelineInfo;
                pipelineInfo.taskShader = collection->taskShader;
                pipelineInfo.meshShader = collection->meshShader;
                pipelineInfo.fragmentShader = collection->fragmentShader;
                pipelineInfo.pipelineLayout = std::move(layout);
                pipelineInfo.renderPass = renderPass;
                pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_LESS_OR_EQUAL;
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
                pipelineInfo.pipelineLayout = std::move(layout);
                pipelineInfo.renderPass = renderPass;
                pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_LESS_OR_EQUAL;
                pipelineInfo.multisampleState.samples = viewport->getSamples();
                pipelineInfo.colorBlend.attachmentCount = 1;
                Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                command->bindPipeline(pipeline);
            }

            descriptorSets[INDEX_VERTEX_DATA] = vertexData->getVertexDataSet();
            for (const auto& [_, instance] : materialData.instances)
            {
                descriptorSets[INDEX_MATERIAL] = instance.materialInstance->getDescriptorSet();
                descriptorSets[INDEX_SCENE_DATA] = instance.descriptorSet;
                command->bindDescriptor(descriptorSets);
                if (graphics->supportMeshShading())
                {
                    command->dispatch(instance.meshes.size(), 1, 1);
                }
                else
                {
                    command->bindIndexBuffer(vertexData->getIndexBuffer());
                    uint32 instanceOffset = 0;
                    for (const auto& [instance, meshData] : instance.meshes)
                    {
                        if (meshData.numIndices > 0)
                        {
                            command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex, meshData.indicesOffset, instanceOffset);
                        }
                        instanceOffset++;
                    }
                }
            }
            commands.add(command);
        }
    }
    graphics->executeCommands(commands);
    graphics->endRenderPass();
}

void BasePass::endFrame() 
{
}

void BasePass::publishOutputs() 
{
    colorAttachment = Gfx::RenderTargetAttachment(viewport,
        Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    resources->registerRenderPassOutput("BASEPASS_COLOR", colorAttachment);
}

void BasePass::createRenderPass() 
{
    depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH");
    depthAttachment.setLoadOp(Gfx::SE_ATTACHMENT_LOAD_OP_LOAD);
    depthAttachment.setInitialLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL);
    depthAttachment.setFinalLayout(Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{
        .colorAttachments = { colorAttachment }, 
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
            .srcAccess = Gfx::SE_ACCESS_NONE,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        }
    };
    renderPass = graphics->createRenderPass(std::move(layout), {dependency}, viewport);
    oLightIndexList = resources->requestBuffer("LIGHTCULLING_OLIGHTLIST");
    tLightIndexList = resources->requestBuffer("LIGHTCULLING_TLIGHTLIST");
    oLightGrid = resources->requestTexture("LIGHTCULLING_OLIGHTGRID");
    tLightGrid = resources->requestTexture("LIGHTCULLING_TLIGHTGRID");
}

void BasePass::modifyRenderPassMacros(Map<const char*, const char*>&)
{
}
