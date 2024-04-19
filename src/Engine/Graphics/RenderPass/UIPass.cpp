#include "UIPass.h"
#include "Graphics/Enums.h"
#include "RenderGraph.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/Command.h"

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
    Gfx::ORenderCommand command = graphics->createRenderCommand("UIPassCommand");
    command->setViewport(viewport);
    command->bindPipeline(pipeline);
    command->bindVertexBuffer({elementBuffer});
    command->bindDescriptor(descriptorSet);
    command->draw(4, static_cast<uint32>(renderElements.size()), 0, 0);
    Array<Gfx::ORenderCommand> commands;
    commands.add(std::move(command));
    graphics->executeCommands(std::move(commands));
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
        .format = Gfx::SE_FORMAT_D32_SFLOAT,
        .width = viewport->getWidth(),
        .height = viewport->getHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };

    depthBuffer = graphics->createTexture2D(depthBufferInfo);
    depthAttachment = 
        Gfx::RenderTargetAttachment(depthBuffer, 
            Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    depthAttachment.clear.depthStencil.depth = 1.0f;
    resources->registerRenderPassOutput("UIPASS_DEPTH", depthAttachment);

    TextureCreateInfo colorBufferInfo = {
        .format = Gfx::SE_FORMAT_R16G16B16A16_SFLOAT,
        .width = viewport->getWidth(),
        .height = viewport->getHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    };
    colorBuffer = graphics->createTexture2D(colorBufferInfo);
    renderTarget =
        Gfx::RenderTargetAttachment(colorBuffer, 
            Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    renderTarget.clear.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
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

    descriptorLayout = graphics->createDescriptorLayout("pParams");
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 0, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 1, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 2, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 3, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .textureType = Gfx::SE_IMAGE_VIEW_TYPE_2D_ARRAY, .descriptorCount = 256, .bindingFlags = Gfx::SE_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | Gfx::SE_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,});
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
    Gfx::OSampler backgroundSampler = graphics->createSampler({});

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
    pipelineLayout->addDescriptorLayout(descriptorLayout);
    pipelineLayout->create();

    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{
        .colorAttachments = { renderTarget }, 
        .depthAttachment = depthAttachment
    };
    renderPass = graphics->createRenderPass(std::move(layout), {}, viewport);
    
    Gfx::LegacyPipelineCreateInfo pipelineInfo;
    pipelineInfo.vertexShader = vertexShader;
    pipelineInfo.fragmentShader = fragmentShader;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.pipelineLayout = std::move(pipelineLayout);
    pipelineInfo.rasterizationState.cullMode = Gfx::SE_CULL_MODE_NONE;
    pipelineInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
}
