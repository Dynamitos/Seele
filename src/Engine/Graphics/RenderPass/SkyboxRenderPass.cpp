#include "SkyboxRenderPass.h"
#include "Graphics/Graphics.h"
#include "Asset/AssetRegistry.h"

using namespace Seele;

SkyboxRenderPass::SkyboxRenderPass(Gfx::PGraphics graphics, PScene scene)
    : RenderPass(graphics, scene)
{
    skybox = Seele::Component::Skybox{
        .day = AssetRegistry::findTexture("FS000_Day_01")->getTexture().cast<Gfx::TextureCube>(),
        .night = AssetRegistry::findTexture("FS000_Night_01")->getTexture().cast<Gfx::TextureCube>(),
        .fogColor = Vector(0.2, 0.1, 0.6),
        .blendFactor = 0,
    };
    Array<Vector> vertices = {
        // Back
        Vector(-512, -512,  512),
        Vector(-512,  512,  512),
        Vector( 512, -512,  512),

        Vector( 512, -512,  512),
        Vector(-512,  512,  512),
        Vector( 512,  512,  512),

        // Front
        Vector( 512, -512, -512),
        Vector( 512,  512, -512),
        Vector(-512, -512, -512),

        Vector(-512, -512, -512),
        Vector( 512,  512, -512),
        Vector(-512,  512, -512),

        // Top
        Vector(-512, -512, -512),
        Vector(-512, -512,  512),
        Vector( 512, -512, -512),

        Vector( 512, -512, -512),
        Vector(-512, -512,  512),
        Vector( 512, -512,  512),

        // Bottom
        Vector(-512,  512,  512),
        Vector(-512,  512, -512),
        Vector( 512,  512,  512),

        Vector( 512,  512,  512),
        Vector(-512,  512, -512),
        Vector( 512,  512, -512),

        // Left
        Vector(-512, -512, -512),
        Vector(-512,  512, -512),
        Vector(-512, -512,  512),

        Vector(-512, -512,  512),
        Vector(-512,  512, -512),
        Vector(-512,  512,  512),

        // Right
        Vector( 512, -512,  512),
        Vector( 512,  512,  512),
        Vector( 512, -512, -512),

        Vector( 512, -512, -512),
        Vector( 512,  512,  512),
        Vector( 512,  512, -512),
    };

    VertexBufferCreateInfo vertexBufferInfo = {
        .sourceData = {
            .size = sizeof(Vector) * vertices.size(),
            .data = (uint8*)vertices.data(),
        },
        .vertexSize = sizeof(Vector),
        .numVertices = (uint32)vertices.size(),
    };
    cubeBuffer = graphics->createVertexBuffer(vertexBufferInfo);
}
SkyboxRenderPass::~SkyboxRenderPass()
{

}

void SkyboxRenderPass::beginFrame(const Component::Camera& cam)
{
    RenderPass::beginFrame(cam);
    DataSource uniformUpdate;

    viewParams.viewMatrix = cam.getViewMatrix();
    viewParams.projectionMatrix = viewport->getProjectionMatrix();
    viewParams.cameraPosition = Vector4(cam.getCameraPosition(), 1);
    viewParams.screenDimensions = Vector2(static_cast<float>(viewport->getSizeX()), static_cast<float>(viewport->getSizeY()));
    uniformUpdate.size = sizeof(ViewParameter);
    uniformUpdate.data = (uint8*)&viewParams;
    viewParamsBuffer->updateContents(uniformUpdate);
    skyboxDataLayout->reset();
    textureLayout->reset();
    skyboxDataSet = skyboxDataLayout->allocateDescriptorSet();
    skyboxDataSet->updateBuffer(0, viewParamsBuffer);
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
    renderCommand->bindDescriptor({skyboxDataSet, textureSet});
    renderCommand->bindVertexBuffer({ cubeBuffer });
    renderCommand->draw(36, 1, 0, 0);
    graphics->executeCommands(Array{ renderCommand });
    graphics->endRenderPass();
}

void SkyboxRenderPass::endFrame()
{

}

void SkyboxRenderPass::publishOutputs()
{
    UniformBufferCreateInfo viewCreateInfo = {
        .sourceData = DataSource {
            .size = sizeof(ViewParameter),
            .data = nullptr,
        },
        .dynamic = true
    };
    viewParamsBuffer = graphics->createUniformBuffer(viewCreateInfo);

    skyboxDataLayout = graphics->createDescriptorLayout("SkyboxDescLayout");
    skyboxDataLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    skyboxDataLayout->create();
    textureLayout = graphics->createDescriptorLayout();
    textureLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    textureLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    textureLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_SAMPLER);
    textureLayout->create();

    skyboxSampler = graphics->createSamplerState({});
}

void SkyboxRenderPass::createRenderPass()
{
    Gfx::PRenderTargetAttachment baseColorAttachment = resources->requestRenderTarget("BASEPASS_COLOR");
    baseColorAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    Gfx::PRenderTargetAttachment depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH");
    depthAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    Gfx::ORenderTargetLayout layout = new Gfx::RenderTargetLayout(baseColorAttachment, depthAttachment);
    renderPass = graphics->createRenderPass(std::move(layout), viewport);

    ShaderCreateInfo createInfo;
    createInfo.name = "SkyboxVertex";
    createInfo.mainModule = "Skybox";
    createInfo.entryPoint = "vertexMain";
    vertexShader = graphics->createVertexShader(createInfo);

    createInfo.name = "SkyboxFragment";
    createInfo.entryPoint = "fragmentMain";
    fragmentShader = graphics->createFragmentShader(createInfo);

    declaration = graphics->createVertexDeclaration({
        Gfx::VertexElement {
            .binding = 0,
            .offset = 0,
            .vertexFormat = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
            .attributeIndex = 0,
            .stride = sizeof(Vector),
        }
    });

    Gfx::OPipelineLayout pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->addDescriptorLayout(0, skyboxDataLayout);
    pipelineLayout->addDescriptorLayout(0, textureLayout);
    pipelineLayout->create();

    Gfx::LegacyPipelineCreateInfo gfxInfo;
    gfxInfo.vertexDeclaration = declaration;
    gfxInfo.vertexShader = vertexShader;
    gfxInfo.fragmentShader = fragmentShader;
    gfxInfo.rasterizationState.polygonMode = Gfx::SE_POLYGON_MODE_FILL;
    gfxInfo.rasterizationState.lineWidth = 5.f;
    gfxInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    gfxInfo.pipelineLayout = std::move(pipelineLayout);
    gfxInfo.renderPass = renderPass;
    pipeline = graphics->createGraphicsPipeline(std::move(gfxInfo));
}
