#include "DebugPass.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Shader.h"

using namespace Seele;

Array<DebugVertex> gDebugVertices;

void Seele::addDebugVertex(DebugVertex vert)
{
    gDebugVertices.add(vert);
}

void Seele::addDebugVertices(Array<DebugVertex> verts)
{
    gDebugVertices.addAll(verts);
}

DebugPass::DebugPass(Gfx::PGraphics graphics, PScene scene)
    : RenderPass(graphics, scene)
{
    
}

DebugPass::~DebugPass()
{
    
}

void DebugPass::beginFrame(const Component::Camera& cam)
{
    RenderPass::beginFrame(cam);
    
    VertexBufferCreateInfo vertexBufferInfo = {
        .sourceData = {
            .size = sizeof(DebugVertex) * gDebugVertices.size(),
            .data = (uint8*)gDebugVertices.data(),
        },
        .vertexSize = sizeof(DebugVertex),
        .numVertices = (uint32)gDebugVertices.size(),
    };
    debugVertices = graphics->createVertexBuffer(vertexBufferInfo);

}

void DebugPass::render()
{
    graphics->beginRenderPass(renderPass);
    Gfx::ORenderCommand renderCommand = graphics->createRenderCommand("DebugRender");
    renderCommand->setViewport(viewport);
    renderCommand->bindPipeline(pipeline);
    renderCommand->bindDescriptor(viewParamsSet);
    renderCommand->bindVertexBuffer({ debugVertices });
    renderCommand->draw((uint32)gDebugVertices.size(), 1, 0, 0);
    Array<Gfx::ORenderCommand> commands;
    commands.add(std::move(renderCommand));
    graphics->executeCommands({std::move(commands)});
    graphics->endRenderPass();
    gDebugVertices.clear();
}

void DebugPass::endFrame()
{
    
}

void DebugPass::publishOutputs()
{
}

void DebugPass::createRenderPass()
{
    Gfx::RenderTargetAttachment baseColorAttachment = resources->requestRenderTarget("BASEPASS_COLOR");
    baseColorAttachment.setLoadOp(Gfx::SE_ATTACHMENT_LOAD_OP_LOAD);
    baseColorAttachment.setInitialLayout(Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    Gfx::RenderTargetAttachment depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH");
    depthAttachment.setLoadOp(Gfx::SE_ATTACHMENT_LOAD_OP_LOAD);
    depthAttachment.setInitialLayout(Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{
        .colorAttachments = {baseColorAttachment}, 
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
    renderPass = graphics->createRenderPass(std::move(layout), dependency, viewport);
    
    pipelineLayout = graphics->createPipelineLayout("DebugPassLayout");
    pipelineLayout->addDescriptorLayout(viewParamsLayout);

    ShaderCreateInfo createInfo = {
        .name = "DebugVertex",
        .mainModule = "Debug",
        .entryPoint = "vertexMain",
        .rootSignature = pipelineLayout,
    };
    vertexShader = graphics->createVertexShader(createInfo);

    createInfo.name = "DebugFragment";
    createInfo.entryPoint = "fragmentMain";
    fragmentShader = graphics->createFragmentShader(createInfo);
    pipelineLayout->create();

    VertexInputStateCreateInfo inputCreate = {
        .bindings = {
            VertexInputBinding {
                .binding = 0,
                .stride = sizeof(DebugVertex),
                .inputRate = Gfx::SE_VERTEX_INPUT_RATE_VERTEX,
            },
        },
        .attributes = {
            VertexInputAttribute {
                .location = 0,
                .binding = 0,
                .format = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
                .offset = 0,
            },
            VertexInputAttribute {
                .location = 1,
                .binding = 0,
                .format = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
                .offset = sizeof(Vector)
            }
        },
    };
    vertexInput = graphics->createVertexInput(inputCreate);
    Gfx::LegacyPipelineCreateInfo gfxInfo;
    gfxInfo.vertexInput = vertexInput;
    gfxInfo.vertexShader = vertexShader;
    gfxInfo.fragmentShader = fragmentShader;
    gfxInfo.rasterizationState.polygonMode = Gfx::SE_POLYGON_MODE_LINE;
    gfxInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_LIST;
    gfxInfo.pipelineLayout = std::move(pipelineLayout);
    gfxInfo.renderPass = renderPass;
    gfxInfo.colorBlend.attachmentCount = 1;
    gfxInfo.rasterizationState.lineWidth = 5.f;
    pipeline = graphics->createGraphicsPipeline(std::move(gfxInfo));
}
