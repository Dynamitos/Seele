#include "UIPass.h"
#include "RenderGraph.h"
#include "Graphics/Graphics.h"

using namespace Seele;

UIPass::UIPass(Gfx::PGraphics graphics, Gfx::PViewport viewport, Gfx::PRenderTargetAttachment attachment) 
    : RenderPass(graphics, viewport)
    , renderTarget(attachment)
{
}

UIPass::~UIPass()
{
    
}

void UIPass::beginFrame() 
{
    VertexBufferCreateInfo info = {
        .resourceData = {
            .size = (uint32)(sizeof(UI::RenderElementStyle) * passData.renderElements.size()),
            .data = (uint8*)passData.renderElements.data()
        },
        .vertexSize = sizeof(UI::RenderElementStyle),
        .numVertices = (uint32)passData.renderElements.size(),
    };
    elementBuffer = graphics->createVertexBuffer(info);
    uint32 numTextures = static_cast<uint32>(passData.usedTextures.size());
    numTexturesBuffer->updateContents({
        .size = sizeof(uint32),
        .data = (uint8*)&numTextures,
    });
    descriptorSet->updateBuffer(2, numTexturesBuffer);
    descriptorSet->updateTextureArray(3, passData.usedTextures);
    descriptorSet->writeChanges();
    //co_return;
}

void UIPass::render() 
{
    graphics->beginRenderPass(renderPass);
    Gfx::PRenderCommand command = graphics->createRenderCommand("UIPassCommand");
    command->setViewport(viewport);
    command->bindPipeline(pipeline);
    command->bindVertexBuffer({VertexInputStream(0, 0, elementBuffer)});
    command->bindDescriptor(descriptorSet);
    command->draw(4, static_cast<uint32>(passData.renderElements.size()), 0, 0);
    graphics->executeCommands(Array<Gfx::PRenderCommand>({command}));
    graphics->endRenderPass();
    //co_return;
}

void UIPass::endFrame() 
{
    //co_return;
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
    resources->registerRenderPassOutput("UIPASS_DEPTH", depthAttachment);
}

void UIPass::createRenderPass() 
{
    ShaderCreateInfo createInfo;
    createInfo.mainModule = "UIPass";
    createInfo.name = "UIVertex";
    createInfo.entryPoint = "vertexMain";
    vertexShader = graphics->createVertexShader(createInfo);

    createInfo.name = "UIFragment";
    createInfo.entryPoint =  "fragmentMain";
    fragmentShader = graphics->createFragmentShader(createInfo);
    Array<Gfx::VertexElement> decl;
    decl.add({
        .streamIndex = 0,
        .offset = offsetof(UI::RenderElementStyle, position),
        .vertexFormat = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
        .attributeIndex = 0,
        .stride = sizeof(UI::RenderElementStyle),
        .bInstanced = 1
    });
    decl.add({
        .streamIndex = 0,
        .offset = offsetof(UI::RenderElementStyle, backgroundImageIndex),
        .vertexFormat = Gfx::SE_FORMAT_R32_UINT,
        .attributeIndex = 1,
        .stride = sizeof(UI::RenderElementStyle),
        .bInstanced = 1
    });
    decl.add({
        .streamIndex = 0,
        .offset = offsetof(UI::RenderElementStyle, backgroundColor),
        .vertexFormat = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
        .attributeIndex = 2,
        .stride = sizeof(UI::RenderElementStyle),
        .bInstanced = 1
    });
    decl.add({
        .streamIndex = 0,
        .offset = offsetof(UI::RenderElementStyle, opacity),
        .vertexFormat = Gfx::SE_FORMAT_R32_SFLOAT,
        .attributeIndex = 3,
        .stride = sizeof(UI::RenderElementStyle),
        .bInstanced = 1
    });
    decl.add({
        .streamIndex = 0,
        .offset = offsetof(UI::RenderElementStyle, dimensions),
        .vertexFormat = Gfx::SE_FORMAT_R32G32_SFLOAT,
        .attributeIndex = 4,
        .stride = sizeof(UI::RenderElementStyle),
        .bInstanced = 1
    });
    declaration = graphics->createVertexDeclaration(decl);

    descriptorLayout = graphics->createDescriptorLayout("UIDescriptorLayout");
    descriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_SAMPLER);
    descriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptorLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 256, Gfx::SE_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | Gfx::SE_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
    descriptorLayout->create();

    Math::Matrix4 projectionMatrix = glm::ortho(0, 1, 1, 0);
    UniformBufferCreateInfo info = {
        .resourceData = {
            .size = sizeof(Math::Matrix4),
            .data = (uint8*)&projectionMatrix,
        },
        .bDynamic = false,
    };
    Gfx::PUniformBuffer uniformBuffer = graphics->createUniformBuffer(info);
    Gfx::PSamplerState backgroundSampler = graphics->createSamplerState({});

    info = {
        .resourceData = {
            .size = sizeof(uint32),
            .data = nullptr
        },
        .bDynamic = true,
    };

    numTexturesBuffer = graphics->createUniformBuffer(info);

    descriptorSet = descriptorLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(0, uniformBuffer);
    descriptorSet->updateSampler(1, backgroundSampler);
    descriptorSet->writeChanges();

    pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->addDescriptorLayout(0, descriptorLayout);
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
