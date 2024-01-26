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
    Gfx::PRenderCommand renderCommand = graphics->createRenderCommand("DebugRender");
    renderCommand->setViewport(viewport);
    renderCommand->bindPipeline(pipeline);
    renderCommand->bindDescriptor(viewParamsSet);
    renderCommand->bindVertexBuffer({ debugVertices });
    renderCommand->draw((uint32)gDebugVertices.size(), 1, 0, 0);
    graphics->executeCommands(Array{renderCommand});
    graphics->endRenderPass();
}

void DebugPass::endFrame()
{
    
}

void DebugPass::publishOutputs()
{
}

void DebugPass::createRenderPass()
{
    Gfx::PRenderTargetAttachment baseColorAttachment = resources->requestRenderTarget("BASEPASS_COLOR");
    baseColorAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    Gfx::PRenderTargetAttachment depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH");
    depthAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    Gfx::ORenderTargetLayout layout = new Gfx::RenderTargetLayout{
        .colorAttachments = {baseColorAttachment}, 
        .depthAttachment = depthAttachment,
    };
    renderPass = graphics->createRenderPass(std::move(layout), viewport);
    
    Gfx::OPipelineLayout pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->addDescriptorLayout(0, viewParamsLayout);
    pipelineLayout->create();

    ShaderCreateInfo createInfo = {
        .mainModule = "Debug",
        .additionalModules = { "Debug" },
        .name = "DebugVertex",
        .entryPoint = "vertexMain",
    };
    vertexShader = graphics->createVertexShader(createInfo);

    createInfo.name = "DebugFragment";
    createInfo.entryPoint = "fragmentMain";
    fragmentShader = graphics->createFragmentShader(createInfo);

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
    pipeline = graphics->createGraphicsPipeline(std::move(gfxInfo));
}
