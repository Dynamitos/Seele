#include "StaticDepthPrepass.h"
#include "Graphics/StaticMeshVertexData.h"
#include "Graphics/Shader.h"

using namespace Seele;

StaticDepthPrepass::StaticDepthPrepass(Gfx::PGraphics graphics, PScene scene)
    : RenderPass(graphics, scene)
{
    meshletCullingLayout = graphics->createDescriptorLayout("pCullingList");
    meshletCullingLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 0, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, });
    meshletCullingLayout->create();

    depthPrepassLayout = graphics->createPipelineLayout("DepthPrepassLayout");
    depthPrepassLayout->addDescriptorLayout(viewParamsLayout);
    depthPrepassLayout->addDescriptorLayout(meshletCullingLayout);
    if (graphics->supportMeshShading())
    {
        graphics->getShaderCompiler()->registerRenderPass(depthPrepassLayout, "DepthPass", "MeshletPass", false, false, "", true);
    }
    else
    {
        graphics->getShaderCompiler()->registerRenderPass(depthPrepassLayout, "DepthPass", "LegacyPass");
    }
}

StaticDepthPrepass::~StaticDepthPrepass()
{
}

void StaticDepthPrepass::beginFrame(const Component::Camera& cam)
{
    RenderPass::beginFrame(cam);
}

void StaticDepthPrepass::render()
{
    Array<Gfx::ORenderCommand> commands;
    graphics->beginRenderPass(renderPass);

    Gfx::ShaderPermutation permutation;
    if (graphics->supportMeshShading())
    {
        permutation.setTaskFile("StaticMeshletPass");
        permutation.setMeshFile("StaticMeshletPass");
    }
    else
    {
        permutation.setVertexFile("LegacyPass");
    }
    StaticMeshVertexData* vd = StaticMeshVertexData::getInstance();
    permutation.setVertexData(vd->getTypeName());
    meshletCullingLayout->reset();
    for (size_t i = 0; i < vd->getStaticMeshes().size(); ++i)
    {
        const auto& mesh = vd->getStaticMeshes()[i];
        Gfx::PermutationId id(permutation);

        Gfx::ORenderCommand command = graphics->createRenderCommand("DepthRender");
        command->setViewport(viewport);

        const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(id);
        assert(collection != nullptr);
        if (graphics->supportMeshShading())
        {
            Gfx::MeshPipelineCreateInfo pipelineInfo = {
                .taskShader = collection->taskShader,
                .meshShader = collection->meshShader,
                .fragmentShader = collection->fragmentShader,
                .renderPass = renderPass,
                .pipelineLayout = collection->pipelineLayout,
                .multisampleState = {
                    .samples = viewport->getSamples(),},
                .depthStencilState = {
                    .depthCompareOp = Gfx::SE_COMPARE_OP_GREATER,
                },
                .colorBlend = {
                    .attachmentCount = 1,
                },
            };
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
            pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_GREATER;
            pipelineInfo.multisampleState.samples = viewport->getSamples();
            pipelineInfo.colorBlend.attachmentCount = 1;
            Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
            command->bindPipeline(pipeline);
        }
        command->bindDescriptor(viewParamsSet);
        command->bindDescriptor(vd->getVertexDataSet());
        command->bindDescriptor(vd->getInstanceDataSet(), { 0, 0 });
        for (const auto& instance : mesh.staticInstance)
        {
            Gfx::PDescriptorSet cullingSet = meshletCullingLayout->allocateDescriptorSet();
            cullingSet->updateBuffer(0, instance.culledMeshletBuffer);
            cullingSet->writeChanges();
            command->bindDescriptor(cullingSet);
            if (graphics->supportMeshShading())
            {
                command->drawMesh(instance.meshletIds.size(), 1, 1);
            }
            else
            {
                //command->bindIndexBuffer(vd->getIndexBuffer());
                //uint32 instanceOffset = 0;
                //for (const auto& meshData : vd->getMeshData(mapping.mapped))
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
    graphics->endRenderPass();
}

void StaticDepthPrepass::endFrame()
{
}

void StaticDepthPrepass::publishOutputs()
{
    
}

void StaticDepthPrepass::createRenderPass()
{
    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{
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
            .srcSubpass = 0,
            .dstSubpass = ~0U,
            .srcStage = Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        }
    };
    renderPass = graphics->createRenderPass(std::move(layout), dependency, viewport);
}
