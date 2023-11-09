#include "UIPass.h"
#include "RenderGraph.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderTarget.h"

using namespace Seele;

UIPass::UIPass(Gfx::PGraphics graphics, PScene scene) 
    : RenderPass(graphics, scene)
{
}

UIPass::~UIPass()
{
    
}

void UIPass::beginFrame(const Component::Camera& cam) 
{
    RenderPass::beginFrame(cam);
    VertexBufferCreateInfo info = {
        .sourceData = {
            .size = (uint32)(sizeof(UI::RenderElementStyle) * renderElements.size()),
            .data = (uint8*)renderElements.data()
        },
        .vertexSize = sizeof(UI::RenderElementStyle),
        .numVertices = (uint32)renderElements.size(),
    };
    elementBuffer = graphics->createVertexBuffer(info);
    uint32 numTextures = static_cast<uint32>(usedTextures.size());
    numTexturesBuffer->updateContents({
        .size = sizeof(uint32),
        .data = (uint8*)&numTextures,
    });
    descriptorSet->updateBuffer(2, numTexturesBuffer);
    descriptorSet->updateTextureArray(3, usedTextures);
    descriptorSet->writeChanges();
    //co_return;
}

void UIPass::render() 
{
    graphics->beginRenderPass(renderPass);
    Gfx::PRenderCommand command = graphics->createRenderCommand("UIPassCommand");
    command->setViewport(viewport);
    command->bindPipeline(pipeline);
    command->bindVertexBuffer({elementBuffer});
    command->bindDescriptor(descriptorSet);
    command->draw(4, static_cast<uint32>(renderElements.size()), 0, 0);
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
    TextureCreateInfo depthBufferInfo = {
        .width = viewport->getSizeX(),
        .height = viewport->getSizeY(),
        .format = Gfx::SE_FORMAT_D32_SFLOAT,
        .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };

    depthBuffer = graphics->createTexture2D(depthBufferInfo);
    depthAttachment = 
        new Gfx::RenderTargetAttachment(depthBuffer, Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    depthAttachment->clear.depthStencil.depth = 1.0f;
    resources->registerRenderPassOutput("UIPASS_DEPTH", depthAttachment);

    TextureCreateInfo colorBufferInfo = {
        .width = viewport->getSizeX(),
        .height = viewport->getSizeY(),
        .format = Gfx::SE_FORMAT_R16G16B16A16_SFLOAT,
        .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    };
    colorBuffer = graphics->createTexture2D(colorBufferInfo);
    renderTarget =
        new Gfx::RenderTargetAttachment(colorBuffer, Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    renderTarget->clear.color = { 0.0f, 0.0f, 0.0f, 1.0f };
    resources->registerRenderPassOutput("UIPASS_COLOR", renderTarget);
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
        .binding = 0,
        .offset = offsetof(UI::RenderElementStyle, position),
        .vertexFormat = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
        .attributeIndex = 0,
        .stride = sizeof(UI::RenderElementStyle),
        .instanced = 1
    });
    decl.add({
        .binding = 0,
        .offset = offsetof(UI::RenderElementStyle, backgroundImageIndex),
        .vertexFormat = Gfx::SE_FORMAT_R32_UINT,
        .attributeIndex = 1,
        .stride = sizeof(UI::RenderElementStyle),
        .instanced = 1
    });
    decl.add({
        .binding = 0,
        .offset = offsetof(UI::RenderElementStyle, backgroundColor),
        .vertexFormat = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
        .attributeIndex = 2,
        .stride = sizeof(UI::RenderElementStyle),
        .instanced = 1
    });
    decl.add({
        .binding = 0,
        .offset = offsetof(UI::RenderElementStyle, opacity),
        .vertexFormat = Gfx::SE_FORMAT_R32_SFLOAT,
        .attributeIndex = 3,
        .stride = sizeof(UI::RenderElementStyle),
        .instanced = 1
    });
    decl.add({
        .binding = 0,
        .offset = offsetof(UI::RenderElementStyle, dimensions),
        .vertexFormat = Gfx::SE_FORMAT_R32G32_SFLOAT,
        .attributeIndex = 4,
        .stride = sizeof(UI::RenderElementStyle),
        .instanced = 1
    });
    declaration = graphics->createVertexDeclaration(decl);

    descriptorLayout = graphics->createDescriptorLayout("UIDescriptorLayout");
    descriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_SAMPLER);
    descriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptorLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 256, Gfx::SE_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | Gfx::SE_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
    descriptorLayout->create();

    Matrix4 projectionMatrix = glm::ortho(0, 1, 1, 0);
    UniformBufferCreateInfo info = {
        .sourceData = {
            .size = sizeof(Matrix4),
            .data = (uint8*)&projectionMatrix,
        },
        .dynamic = false,
    };
    Gfx::OUniformBuffer uniformBuffer = graphics->createUniformBuffer(info);
    Gfx::OSamplerState backgroundSampler = graphics->createSamplerState({});

    info = {
        .sourceData = {
            .size = sizeof(uint32),
            .data = nullptr
        },
        .dynamic = true,
    };

    numTexturesBuffer = graphics->createUniformBuffer(info);

    descriptorSet = descriptorLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(0, uniformBuffer);
    descriptorSet->updateSampler(1, backgroundSampler);
    descriptorSet->writeChanges();

    Gfx::OPipelineLayout pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->addDescriptorLayout(0, descriptorLayout);
    pipelineLayout->create();

    Gfx::ORenderTargetLayout layout = new Gfx::RenderTargetLayout(std::move(renderTarget), std::move(depthAttachment));
    renderPass = graphics->createRenderPass(std::move(layout), viewport);
    
    Gfx::LegacyPipelineCreateInfo pipelineInfo;
    pipelineInfo.vertexDeclaration = declaration;
    pipelineInfo.vertexShader = vertexShader;
    pipelineInfo.fragmentShader = fragmentShader;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.pipelineLayout = std::move(pipelineLayout);
    pipelineInfo.rasterizationState.cullMode = Gfx::SE_CULL_MODE_NONE;
    pipelineInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
}
