#include "UIPass.h"
#include "Graphics/Command.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderTarget.h"
#include "RenderGraph.h"

using namespace Seele;

UIPass::UIPass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics, scene) {}

UIPass::~UIPass() {}

void UIPass::beginFrame(const Component::Camera& cam) {
    RenderPass::beginFrame(cam);
    VertexBufferCreateInfo info = {
        .sourceData =
            {
                .size = (uint32)(sizeof(UI::RenderElementStyle) * renderElements.size()),
                .data = (uint8*)renderElements.data(),
            },
        .vertexSize = sizeof(UI::RenderElementStyle),
        .numVertices = (uint32)renderElements.size(),
    };
    elementBuffer = graphics->createVertexBuffer(info);
    uint32 numTextures = static_cast<uint32>(usedTextures.size());
    numTexturesBuffer->updateContents(0, sizeof(uint32), &numTextures);
    descriptorSet->updateBuffer(2, 0, numTexturesBuffer);
    for (uint32 i = 0; i < usedTextures.size(); ++i) {
        descriptorSet->updateTexture(3, i, usedTextures[i]);
    }
    descriptorSet->writeChanges();
    // co_return;
}

void UIPass::render() {
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

    // co_return;
}

void UIPass::endFrame() {
    // co_return;
}

void UIPass::publishOutputs() {
    TextureCreateInfo depthBufferInfo = {
        .format = Gfx::SE_FORMAT_D32_SFLOAT,
        .width = viewport->getWidth(),
        .height = viewport->getHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };

    depthBuffer = graphics->createTexture2D(depthBufferInfo);
    depthAttachment =
        Gfx::RenderTargetAttachment(depthBuffer, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    resources->registerRenderPassOutput("UIPASS_DEPTH", depthAttachment);

    TextureCreateInfo colorBufferInfo = {
        .format = Gfx::SE_FORMAT_R16G16B16A16_SFLOAT,
        .width = viewport->getWidth(),
        .height = viewport->getHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    };
    colorBuffer = graphics->createTexture2D(colorBufferInfo);
    renderTarget = Gfx::RenderTargetAttachment(colorBuffer, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                               Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR,
                                               Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    renderTarget.clear.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    resources->registerRenderPassOutput("UIPASS_COLOR", renderTarget);
}

void UIPass::createRenderPass() {
    ShaderCompilationInfo createInfo = {
        .name = "UIVertex",
        .modules = {"UIPass"},
        .entryPoints =
            {
                {"vertexMain", "UIPass"},
                {"fragmentMain", "UIFragment"},
            },
    };
    graphics->beginShaderCompilation(createInfo);
    vertexShader = graphics->createVertexShader({0});
    fragmentShader = graphics->createFragmentShader({1});

    descriptorLayout = graphics->createDescriptorLayout("pParams");
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .uniformLength = sizeof(Matrix4),
    });
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 2,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .uniformLength = sizeof(uint32)
    });
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 3,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .textureType = Gfx::SE_IMAGE_VIEW_TYPE_2D_ARRAY,
        .descriptorCount = 256,
        .bindingFlags = Gfx::SE_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | Gfx::SE_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
    });
    descriptorLayout->create();

    Matrix4 projectionMatrix = glm::ortho(0, 1, 1, 0);

    Gfx::OUniformBuffer uniformBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(Matrix4),
                .data = (uint8*)&projectionMatrix,
            },
    });
    Gfx::OSampler backgroundSampler = graphics->createSampler({});

    numTexturesBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32),
                .data = nullptr,
            },
    });

    descriptorSet = descriptorLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(0, 0, uniformBuffer);
    descriptorSet->updateSampler(1, 0, backgroundSampler);
    descriptorSet->writeChanges();

    pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->addDescriptorLayout(descriptorLayout);
    pipelineLayout->create();

    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{.colorAttachments = {renderTarget}, .depthAttachment = depthAttachment};
    renderPass = graphics->createRenderPass(std::move(layout), {}, viewport);

    Gfx::LegacyPipelineCreateInfo pipelineInfo;
    pipelineInfo.vertexShader = vertexShader;
    pipelineInfo.fragmentShader = fragmentShader;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.pipelineLayout = pipelineLayout;
    pipelineInfo.rasterizationState.cullMode = Gfx::SE_CULL_MODE_NONE;
    pipelineInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
}
