#include "SkyboxRenderPass.h"
#include "Graphics/Graphics.h"
#include "Asset/AssetRegistry.h"
#include "Graphics/Command.h"

using namespace Seele;

SkyboxRenderPass::SkyboxRenderPass(Gfx::PGraphics graphics, PScene scene)
    : RenderPass(graphics, scene)
{
    skybox = Seele::Component::Skybox{
        .day = AssetRegistry::findTexture("FS000_Day_01")->getTexture().cast<Gfx::TextureCube>(),
        .night = AssetRegistry::findTexture("FS000_Night_01")->getTexture().cast<Gfx::TextureCube>(),
        .fogColor = Vector(0.1, 0.1, 0.8),
        .blendFactor = 0,
    };
}
SkyboxRenderPass::~SkyboxRenderPass()
{

}

void SkyboxRenderPass::beginFrame(const Component::Camera& cam)
{
    RenderPass::beginFrame(cam);

    skyboxDataLayout->reset();
    textureLayout->reset();
    skyboxData.transformMatrix = glm::rotate(skyboxData.transformMatrix, (float)(Gfx::getCurrentFrameDelta()), Vector(0, 1, 0));
    skyboxBuffer->updateContents(DataSource{
        .size = sizeof(SkyboxData),
        .data = (uint8*)&skyboxData,
        });
    skyboxDataSet = skyboxDataLayout->allocateDescriptorSet();
    skyboxDataSet->updateBuffer(0, skyboxBuffer);
    skyboxDataSet->writeChanges();
    textureSet = textureLayout->allocateDescriptorSet();
    textureSet->updateTexture(0, skybox.day);
    textureSet->updateTexture(1, skybox.night);
    textureSet->updateSampler(2, skyboxSampler);
    textureSet->writeChanges();
}

void SkyboxRenderPass::render()
{
    graphics->beginRenderPass(renderPass);
    Gfx::PRenderCommand renderCommand = graphics->createRenderCommand("SkyboxRender");
    renderCommand->setViewport(viewport);
    renderCommand->bindPipeline(pipeline);
    renderCommand->bindDescriptor({viewParamsSet, skyboxDataSet, textureSet});
    renderCommand->draw(36, 1, 0, 0);
    graphics->executeCommands(Array{ renderCommand });
    graphics->endRenderPass();
    baseColorAttachment->getTexture()->changeLayout(Gfx::SE_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    graphics->resolveTexture(baseColorAttachment->getTexture(), viewport->getOwner()->getBackBuffer());
}

void SkyboxRenderPass::endFrame()
{

}

void SkyboxRenderPass::publishOutputs()
{
    skyboxDataLayout = graphics->createDescriptorLayout("SkyboxDescLayout");
    skyboxDataLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    skyboxDataLayout->create();
    textureLayout = graphics->createDescriptorLayout();
    textureLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    textureLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    textureLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_SAMPLER);
    textureLayout->create();

    skyboxSampler = graphics->createSampler({});
}

void SkyboxRenderPass::createRenderPass()
{
    baseColorAttachment = resources->requestRenderTarget("BASEPASS_COLOR");
    baseColorAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    Gfx::PRenderTargetAttachment depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH");
    depthAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    Gfx::ORenderTargetLayout layout = new Gfx::RenderTargetLayout{
        .colorAttachments = { baseColorAttachment },
        .depthAttachment = depthAttachment
    };
    renderPass = graphics->createRenderPass(std::move(layout), viewport);

    skyboxData.transformMatrix = Matrix4(1);
    skyboxData.fogColor = skybox.fogColor;
    skyboxData.blendFactor = skybox.blendFactor;

    skyboxBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData = {
            .size = sizeof(SkyboxData),
            .data = (uint8*)&skyboxData,
        },
        .dynamic = true,
        });

    ShaderCreateInfo createInfo = {
        .mainModule = "Skybox",
        .additionalModules = {"Skybox"},
        .name = "SkyboxVertex",
        .entryPoint = "vertexMain",
    };
    vertexShader = graphics->createVertexShader(createInfo);

    createInfo.name = "SkyboxFragment";
    createInfo.entryPoint = "fragmentMain";
    fragmentShader = graphics->createFragmentShader(createInfo);

    Gfx::OPipelineLayout pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->addDescriptorLayout(0, viewParamsLayout);
    pipelineLayout->addDescriptorLayout(1, skyboxDataLayout);
    pipelineLayout->addDescriptorLayout(2, textureLayout);
    pipelineLayout->create();

    Gfx::LegacyPipelineCreateInfo gfxInfo;
    gfxInfo.vertexShader = vertexShader;
    gfxInfo.fragmentShader = fragmentShader;
    gfxInfo.rasterizationState.polygonMode = Gfx::SE_POLYGON_MODE_FILL;
    gfxInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    gfxInfo.pipelineLayout = std::move(pipelineLayout);
    gfxInfo.renderPass = renderPass;
    gfxInfo.multisampleState.samples = viewport->getSamples();
    pipeline = graphics->createGraphicsPipeline(std::move(gfxInfo));
}
