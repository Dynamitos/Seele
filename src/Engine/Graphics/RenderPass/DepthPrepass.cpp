#include "DepthPrepass.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include "Window/Window.h"
#include "Component/Camera.h"
#include "Component/Mesh.h"
#include "Actor/CameraActor.h"
#include "Math/Vector.h"
#include "RenderGraph.h"
#include "Graphics/Command.h"

using namespace Seele;

DepthPrepass::DepthPrepass(Gfx::PGraphics graphics, PScene scene) 
    : RenderPass(graphics, scene)
    , descriptorSets(3)
{
    depthPrepassLayout = graphics->createPipelineLayout();
    depthPrepassLayout->addDescriptorLayout(INDEX_VIEW_PARAMS, viewParamsLayout);
    if (graphics->supportMeshShading())
    {
        graphics->getShaderCompiler()->registerRenderPass("DepthPass", "MeshletBasePass", false, false, "", true, true, "MeshletBasePass");
    }
    else
    {
        graphics->getShaderCompiler()->registerRenderPass("DepthPass", "LegacyBasePass");
    }
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
    Gfx::ShaderPermutation permutation;
    if(graphics->supportMeshShading())
    {
        permutation.setTaskFile("MeshletBasePass");
        permutation.setMeshFile("MeshletBasePass");
    }
    else
    {
        permutation.setVertexFile("LegacyBasePass");
    }
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
            // Material => per material
            // VertexData => per meshtype
            // SceneData => per material instance
            Gfx::PermutationId id(permutation);
            
            Gfx::PRenderCommand command = graphics->createRenderCommand("DepthRender");
            command->setViewport(viewport);
            Gfx::OPipelineLayout layout = graphics->createPipelineLayout(depthPrepassLayout);
            //layout->addDescriptorLayout(INDEX_MATERIAL, materialData.material->getDescriptorLayout());
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
                pipelineInfo.pipelineLayout = std::move(layout);
                pipelineInfo.renderPass = renderPass;
                pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_LESS_OR_EQUAL;
                pipelineInfo.multisampleState.samples = viewport->getSamples();
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
                Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                command->bindPipeline(pipeline);
            }

            descriptorSets[INDEX_VERTEX_DATA] = vertexData->getVertexDataSet();
            for (const auto& [_, instance] : materialData.instances)
            {
                //descriptorSets[INDEX_MATERIAL] = instance.materialInstance->getDescriptorSet();
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
                    for (const auto& [_, meshData] : instance.meshes)
                    {
                        if (meshData.numIndices > 0)
                        {
                            command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex, meshData.indicesOffset, instanceOffset++);
                        }
                    }
                }
            }
            commands.add(command);
        }
    }
    graphics->executeCommands(commands);
    graphics->endRenderPass();
}

void DepthPrepass::endFrame() 
{
}

void DepthPrepass::publishOutputs() 
{
    // If we render to a part of an image, the depth buffer itself must
    // still match the size of the whole image or their coordinate systems go out of sync
    TextureCreateInfo depthBufferInfo = {
        .format = Gfx::SE_FORMAT_D32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };
    depthBuffer = graphics->createTexture2D(depthBufferInfo);
    depthAttachment = 
        new Gfx::RenderTargetAttachment(depthBuffer, Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    depthAttachment->clear.depthStencil.depth = 1.0f;
    resources->registerRenderPassOutput("DEPTHPREPASS_DEPTH", depthAttachment);
}

void DepthPrepass::createRenderPass() 
{
    Gfx::ORenderTargetLayout layout = new Gfx::RenderTargetLayout{
        .depthAttachment = depthAttachment,
    };
    renderPass = graphics->createRenderPass(std::move(layout), viewport);
}

void DepthPrepass::modifyRenderPassMacros(Map<const char*, const char*>&) 
{
}
