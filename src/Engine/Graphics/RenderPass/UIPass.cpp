#include "UIPass.h"
#include "RenderGraph.h"
#include "Graphics/Graphics.h"

using namespace Seele;

UIPass::UIPass(PRenderGraph renderGraph, Gfx::PGraphics graphics, Gfx::PViewport viewport, Gfx::PRenderTargetAttachment attachment) 
    : RenderPass(renderGraph, graphics, viewport)
    , renderTarget(attachment)
{
}

UIPass::~UIPass()
{
    
}

void UIPass::beginFrame() 
{
    
}

void UIPass::render() 
{
    graphics->beginRenderPass(renderPass);
    Gfx::PRenderCommand command = graphics->createRenderCommand("UIPassCommand");
    command->setViewport(viewport);
    command->bindPipeline(pipeline);
    command->draw(4, 1, 0, 0);
    graphics->executeCommands(Array<Gfx::PRenderCommand>({command}));
    graphics->endRenderPass();
}

void UIPass::endFrame() 
{
    
}

void UIPass::publishOutputs() 
{
    TextureCreateInfo depthBufferInfo;
    // Even if we only render to part of an image, we need to make sure
    // that the depthbuffer is the same size or they can't be used in the same 
    // framebuffer
    depthBufferInfo.width = viewport->getOwner()->getSizeX();
    depthBufferInfo.height = viewport->getOwner()->getSizeY();
    depthBufferInfo.format = Gfx::SE_FORMAT_D32_SFLOAT;
    depthBufferInfo.usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    depthBuffer = graphics->createTexture2D(depthBufferInfo);
    depthAttachment = 
        new Gfx::RenderTargetAttachment(depthBuffer, Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
        depthAttachment->clear.depthStencil.depth = 1.0f;
    renderGraph->registerRenderPassOutput("UIPASS_DEPTH", depthAttachment);
}

void UIPass::createRenderPass() 
{
    std::ifstream codeStream("./shaders/UIPass.slang", std::ios::ate);
    auto fileSize = codeStream.tellg();
    codeStream.seekg(0);
    Array<char> buffer(static_cast<uint32>(fileSize));
    codeStream.read(buffer.data(), fileSize);

    ShaderCreateInfo createInfo;
    createInfo.shaderCode.add(std::string(buffer.data()));
    createInfo.name = "UIVertex";
    createInfo.entryPoint = "vertexMain";
    vertexShader = graphics->createVertexShader(createInfo);

    createInfo.name = "UIFragment";
    createInfo.entryPoint =  "fragmentMain";
    fragmentShader = graphics->createFragmentShader(createInfo);
    declaration = graphics->createVertexDeclaration({});
    pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->create();

    Gfx::PRenderTargetLayout layout = new Gfx::RenderTargetLayout(renderTarget, depthAttachment);
    renderPass = graphics->createRenderPass(layout, viewport);
    
    GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.vertexDeclaration = declaration;
    pipelineInfo.vertexShader = vertexShader;
    pipelineInfo.fragmentShader = fragmentShader;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.pipelineLayout = pipelineLayout;
    pipelineInfo.rasterizationState.cullMode = Gfx::SE_CULL_MODE_NONE;
    pipelineInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    pipeline = graphics->createGraphicsPipeline(pipelineInfo);
}
