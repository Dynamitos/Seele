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
    lightCullingLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    // tLightGrid
    lightCullingLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    lightCullingLayout->create();

    basePassLayout->addDescriptorLayout(INDEX_LIGHT_CULLING, lightCullingLayout);
}

BasePass::~BasePass()
{
}

void BasePass::beginFrame(const Component::Camera& cam) 
{
    RenderPass::beginFrame(cam);

    lightCullingLayout->reset();
    descriptorSets[INDEX_VIEW_PARAMS] = viewParamsSet;
    descriptorSets[INDEX_LIGHT_ENV] = scene->getLightEnvironment()->getDescriptorSet();
    descriptorSets[INDEX_LIGHT_CULLING] = lightCullingLayout->allocateDescriptorSet();
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

    descriptorSets[INDEX_LIGHT_CULLING]->updateBuffer(0, oLightIndexList);
    descriptorSets[INDEX_LIGHT_CULLING]->updateBuffer(1, tLightIndexList);
    descriptorSets[INDEX_LIGHT_CULLING]->updateTexture(2, oLightGrid);
    descriptorSets[INDEX_LIGHT_CULLING]->updateTexture(3, tLightGrid);
    descriptorSets[INDEX_LIGHT_CULLING]->writeChanges();

    graphics->beginRenderPass(renderPass);
    Gfx::ShaderPermutation permutation;
    permutation.hasFragment = true;
    if(graphics->supportMeshShading())
    {
        permutation.useMeshShading = true;
        permutation.hasTaskShader = true;
        std::memcpy(permutation.taskFile, "MeshletBasePass", sizeof("MeshletBasePass"));
        std::memcpy(permutation.vertexMeshFile, "MeshletBasePass", sizeof("MeshletBasePass"));
    }
    else
    {
        permutation.useMeshShading = false;
        std::memcpy(permutation.vertexMeshFile, "LegacyBasePass", sizeof("LegacyBasePass"));
    }
    std::memcpy(permutation.fragmentFile, "BasePass", std::strlen("BasePass"));
    for (VertexData* vertexData : VertexData::getList())
    {
        std::memcpy(permutation.vertexDataName, vertexData->getTypeName().c_str(), std::strlen(vertexData->getTypeName().c_str()));
        const auto& materials = vertexData->getMaterialData();
        for (const auto& [_, materialData] : materials)
        {
            // Create Pipeline(Material, VertexData)
            // Descriptors:
            // ViewData => global, static
            // LightEnv => global, static
            // LightCulling => global, static
            // Material => per material
            // VertexData => per meshtype
            // SceneData => per material instance

            std::memcpy(permutation.materialName, materialData.material->getName().c_str(), std::strlen(materialData.material->getName().c_str()));
            Gfx::PermutationId id(permutation);

            Gfx::PRenderCommand command = graphics->createRenderCommand("DepthRender");
            Gfx::OPipelineLayout layout = graphics->createPipelineLayout(basePassLayout);
            layout->addDescriptorLayout(INDEX_MATERIAL, materialData.material->getDescriptorLayout());
            layout->addDescriptorLayout(INDEX_VERTEX_DATA, vertexData->getVertexDataLayout());
            layout->addDescriptorLayout(INDEX_SCENE_DATA, vertexData->getInstanceDataLayout());
            layout->create();

            const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(id);
            assert(collection != nullptr);
            if(graphics->supportMeshShading())
            {
                Gfx::MeshPipelineCreateInfo pipelineInfo;
                pipelineInfo.taskShader = collection->taskShader;
                pipelineInfo.meshShader = collection->meshShader;
                pipelineInfo.fragmentShader = collection->fragmentShader;
                pipelineInfo.pipelineLayout = layout;
                pipelineInfo.renderPass = renderPass;
                pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_LESS_OR_EQUAL;
                Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(pipelineInfo);
                command->bindPipeline(pipeline);
            }
            else
            {
                Gfx::LegacyPipelineCreateInfo pipelineInfo;
                pipelineInfo.vertexDeclaration = collection->vertexDeclaration;
                pipelineInfo.vertexShader = collection->vertexShader;
                pipelineInfo.fragmentShader = collection->fragmentShader;
                pipelineInfo.pipelineLayout = layout;
                pipelineInfo.renderPass = renderPass;
                pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_LESS_OR_EQUAL;
                Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(pipelineInfo);
                command->bindPipeline(pipeline);
            }

            descriptorSets[INDEX_VERTEX_DATA] = vertexData->getVertexDataSet();
            for (const auto& [_, instance] : materialData.instances)
            {
                descriptorSets[INDEX_MATERIAL] = instance.materialInstance->getDescriptorSet();
                descriptorSets[INDEX_SCENE_DATA] = instance.descriptorSet;
                command->bindDescriptor(descriptorSets);
                if(Gfx::useMeshShading)
                {
                    command->dispatch(instance.numMeshes, 1, 1);
                }
                else
                {
                    uint32 instanceOffset = 0;
                    for(const auto& mesh : instance.meshes)
                    {
                        uint32 vertexOffset = vertexData->getMeshOffset(mesh.id);
                        command->bindIndexBuffer(mesh.indexBuffer);
                        command->drawIndexed(mesh.indexBuffer->getNumIndices(), 1, 0, vertexOffset, instanceOffset);
                    }
                }
            }
        }
    }
    graphics->endRenderPass();
}

void BasePass::endFrame() 
{
}

void BasePass::publishOutputs() 
{
	colorAttachment = new Gfx::SwapchainAttachment(viewport->getOwner());
    resources->registerRenderPassOutput("BASEPASS_COLOR", colorAttachment);
}

void BasePass::createRenderPass() 
{
    Gfx::PRenderTargetAttachment depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH");
    depthAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment->storeOp = Gfx::SE_ATTACHMENT_STORE_OP_STORE;
    Gfx::ORenderTargetLayout layout = new Gfx::RenderTargetLayout(colorAttachment, depthAttachment);
    renderPass = graphics->createRenderPass(std::move(layout), viewport);
    oLightIndexList = resources->requestBuffer("LIGHTCULLING_OLIGHTLIST");
    tLightIndexList = resources->requestBuffer("LIGHTCULLING_TLIGHTLIST");
    oLightGrid = resources->requestTexture("LIGHTCULLING_OLIGHTGRID");
    tLightGrid = resources->requestTexture("LIGHTCULLING_TLIGHTGRID");
}

void BasePass::modifyRenderPassMacros(Map<const char*, const char*>&)
{
}
