#include "DepthPrepass.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include "Window/Window.h"
#include "Component/Camera.h"
#include "Component/Mesh.h"
#include "Actor/CameraActor.h"
#include "Math/Vector.h"
#include "RenderGraph.h"

using namespace Seele;

DepthPrepass::DepthPrepass(Gfx::PGraphics graphics, PScene scene) 
    : RenderPass(graphics, scene)
    , descriptorSets(4)
{
    UniformBufferCreateInfo uniformInitializer;

    depthPrepassLayout = graphics->createPipelineLayout();
    depthPrepassLayout->addDescriptorLayout(INDEX_VIEW_PARAMS, viewParamsLayout);
}

DepthPrepass::~DepthPrepass()
{   
}

void DepthPrepass::beginFrame(const Component::Camera& cam) 
{
    RenderPass::beginFrame(cam);
    descriptorSets[INDEX_VIEW_PARAMS] = viewParamsSet;
}

void DepthPrepass::render() 
{
    depthAttachment->getTexture()->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    depthAttachment->getTexture()->transferOwnership(Gfx::QueueType::GRAPHICS);
    Gfx::ShaderPermutation permutation;
    permutation.hasFragment = false;
    permutation.useMeshShading = true;
    permutation.hasTaskShader = true;
    std::memcpy(permutation.taskFile, "MeshletBasePass", sizeof("MeshletBasePass"));
    std::memcpy(permutation.vertexMeshFile, "MeshletBasePass", sizeof("MeshletBasePass"));
    graphics->beginRenderPass(renderPass);
    for (VertexData* vertexData : VertexData::getList())
    {
        std::memcpy(permutation.vertexDataName, vertexData->getTypeName().c_str(), std::strlen(vertexData->getTypeName().c_str()));
        const auto& materials = vertexData->getMaterialData();
        for (const auto& [_, materialData] : materials) 
        {
            // Create Pipeline(Material, VertexData)
            // Descriptors:
            // ViewData => global, static
            // Material => per material
            // VertexData => per meshtype
            // SceneData => per material instance
            std::memcpy(permutation.materialName, materialData.material->getName().c_str(), std::strlen(materialData.material->getName().c_str()));
            Gfx::PermutationId id(permutation);
            
            Gfx::PRenderCommand command = graphics->createRenderCommand("DepthRender");
            Gfx::OPipelineLayout layout = graphics->createPipelineLayout(depthPrepassLayout);
            layout->addDescriptorLayout(INDEX_MATERIAL, materialData.material->getDescriptorLayout());
            layout->addDescriptorLayout(INDEX_VERTEX_DATA, vertexData->getVertexDataLayout());
            layout->addDescriptorLayout(INDEX_SCENE_DATA, vertexData->getInstanceDataLayout());
            layout->create();

            Gfx::MeshPipelineCreateInfo pipelineInfo;
            Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(pipelineInfo);
            command->bindPipeline(pipeline);

            descriptorSets[INDEX_VERTEX_DATA] = vertexData->getVertexDataSet();
            for (const auto&[_, instance]: materialData.instances)
            {
                descriptorSets[INDEX_MATERIAL] = instance.materialInstance->getDescriptorSet();
                descriptorSets[INDEX_SCENE_DATA] = instance.descriptorSet;
                command->bindDescriptor(descriptorSets);
                command->dispatch(instance.numMeshes, 1, 1);
            }
        }
    }
    graphics->endRenderPass();
}

void DepthPrepass::endFrame() 
{
}

void DepthPrepass::publishOutputs() 
{
    TextureCreateInfo depthBufferInfo;
    // If we render to a part of an image, the depth buffer itself must
    // still match the size of the whole image or their coordinate systems go out of sync
    depthBufferInfo.width = viewport->getOwner()->getSizeX();
    depthBufferInfo.height = viewport->getOwner()->getSizeY();
    depthBufferInfo.format = Gfx::SE_FORMAT_D32_SFLOAT;
    depthBufferInfo.usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthBuffer = graphics->createTexture2D(depthBufferInfo);
    depthAttachment = 
        new Gfx::RenderTargetAttachment(depthBuffer, Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    depthAttachment->clear.depthStencil.depth = 1.0f;
    resources->registerRenderPassOutput("DEPTHPREPASS_DEPTH", depthAttachment);
}

void DepthPrepass::createRenderPass() 
{
    Gfx::ORenderTargetLayout layout = new Gfx::RenderTargetLayout(depthAttachment);
    renderPass = graphics->createRenderPass(std::move(layout), viewport);
}

void DepthPrepass::modifyRenderPassMacros(Map<const char*, const char*>& defines) 
{
    defines["INDEX_VIEW_PARAMS"] = "0";
    defines["INDEX_MATERIAL"] = "1";
    defines["INDEX_VERTEX_DATA"] = "2";
    defines["INDEX_SCENE_DATA"] = "3";
}
