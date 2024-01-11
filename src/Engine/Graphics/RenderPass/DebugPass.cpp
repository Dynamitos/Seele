#include "DebugPass.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderTarget.h"

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
    renderCommand->bindDescriptor(descriptorSet);
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
    UniformBufferCreateInfo viewCreateInfo = {
        .sourceData = DataSource {
            .size = sizeof(ViewParameter),
            .data = nullptr,
        },
        .dynamic = true
    };
    viewParamsBuffer = graphics->createUniformBuffer(viewCreateInfo);

    descriptorLayout = graphics->createDescriptorLayout("DebugDescLayout");
    descriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptorLayout->create();

    pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->addDescriptorLayout(0, descriptorLayout);
    pipelineLayout->create();
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
    
    ShaderCreateInfo createInfo;
    createInfo.name = "DebugVertex";
    createInfo.mainModule = "Debug";
    createInfo.entryPoint = "vertexMain";
    createInfo.defines["INDEX_VIEW_PARAMS"] = "0";
    Gfx::PVertexShader vertexShader = graphics->createVertexShader(createInfo);

    createInfo.name = "DebugFragment";

    createInfo.entryPoint = "fragmentMain";
    Gfx::PFragmentShader fragmentShader = graphics->createFragmentShader(createInfo);

    Gfx::OVertexDeclaration vertexDecl = graphics->createVertexDeclaration({
        Gfx::VertexElement {
            .streamIndex = 0,
            .offset = offsetof(DebugVertex, position),
            .vertexFormat = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
            .attributeIndex = 0,
            .stride = sizeof(DebugVertex),
        },
        Gfx::VertexElement {
            .streamIndex = 0,
            .offset = offsetof(DebugVertex, color),
            .vertexFormat = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
            .attributeIndex = 1,
            .stride = sizeof(DebugVertex),
        }
    });

    GraphicsPipelineCreateInfo gfxInfo;
    gfxInfo.vertexDeclaration = vertexDecl;
    gfxInfo.vertexShader = vertexShader;
    gfxInfo.fragmentShader = fragmentShader;
    gfxInfo.rasterizationState.polygonMode = Gfx::SE_POLYGON_MODE_LINE;
    gfxInfo.rasterizationState.lineWidth = 5.f;
    gfxInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_LIST;
    gfxInfo.pipelineLayout = pipelineLayout;
    gfxInfo.renderPass = renderPass;
    pipeline = graphics->createGraphicsPipeline(gfxInfo);
}
