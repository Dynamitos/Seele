#include "SkyboxRenderPass.h"
#include "Graphics/Graphics.h"

using namespace Seele;

SkyboxRenderPass::SkyboxRenderPass(Gfx::PGraphics graphics)
    : RenderPass(graphics)
{
    Array<Vector> vertices = {
        // Back
        Vector(-1, -1, -1),
        Vector(1, -1, -1),
        Vector(-1,  1, -1),

        Vector(1, -1, -1),
        Vector(-1,  1, -1),
        Vector(1,  1, -1),

        // Front
        Vector(1, -1,  1),
        Vector(-1,  1,  1),
        Vector(-1, -1,  1),

        Vector(-1,  1,  1),
        Vector(1,  1,  1),
        Vector(1, -1,  1),

        // Top
        Vector(-1, -1, -1),
        Vector(1, -1, -1),
        Vector(1, -1,  1),

        Vector(1, -1, -1),
        Vector(1, -1,  1),
        Vector(1, -1,  1),

        // Bottom
        Vector(-1,  1, -1),
        Vector(-1,  1,  1),
        Vector(1,  1, -1),

        Vector(1,  1, -1),
        Vector(-1,  1,  1),
        Vector(1,  1,  1),

        // Left
        Vector(-1, -1, -1),
        Vector(-1, -1,  1),
        Vector(-1,  1, -1),

        Vector(-1,  1, -1),
        Vector(-1, -1,  1),
        Vector(-1,  1,  1),

        // Right
        Vector(1, -1,  1),
        Vector(1,  1, -1),
        Vector(1, -1, -1),

        Vector(1, -1,  1),
        Vector(1,  1,  1),
        Vector(1,  1, -1),
    };

    VertexBufferCreateInfo vertexBufferInfo = {
        .resourceData = {
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
    BulkResourceData uniformUpdate;

    viewParams.viewMatrix = cam.getViewMatrix();
    viewParams.projectionMatrix = viewport->getProjectionMatrix();
    viewParams.cameraPosition = Vector4(cam.getCameraPosition(), 0);
    viewParams.screenDimensions = Vector2(static_cast<float>(viewport->getSizeX()), static_cast<float>(viewport->getSizeY()));
    uniformUpdate.size = sizeof(ViewParameter);
    uniformUpdate.data = (uint8*)&viewParams;
    viewParamsBuffer->updateContents(uniformUpdate);
    descriptorLayout->reset();
    descriptorSet = descriptorLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(0, viewParamsBuffer);
    descriptorSet->updateTexture(1, passData.skybox.day);
    descriptorSet->updateTexture(2, passData.skybox.night);
    descriptorSet->updateSampler(3, passData.skybox.sampler);
    descriptorSet->writeChanges();
}

void SkyboxRenderPass::render()
{
    graphics->beginRenderPass(renderPass);
    Gfx::PRenderCommand renderCommand = graphics->createRenderCommand("SkyboxRender");
    renderCommand->setViewport(viewport);
    renderCommand->bindPipeline(pipeline);
    renderCommand->bindDescriptor(descriptorSet);
    renderCommand->bindVertexBuffer({ VertexInputStream(0, 0, cubeBuffer) });
    renderCommand->pushConstants(pipelineLayout, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, sizeof(Vector), &passData.skybox.fogColor);
    renderCommand->pushConstants(pipelineLayout, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, sizeof(Vector), sizeof(float), &passData.skybox.blendFactor);
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
        .resourceData = BulkResourceData {
            .size = sizeof(ViewParameter),
            .data = nullptr,
        },
        .bDynamic = true
    };
    viewParamsBuffer = graphics->createUniformBuffer(viewCreateInfo);

    descriptorLayout = graphics->createDescriptorLayout("SkyboxDescLayout");
    descriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptorLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_SAMPLER);
    descriptorLayout->create();

    pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->addDescriptorLayout(0, descriptorLayout);
    pipelineLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        .offset = 0,
        .size = sizeof(Vector),
        });
    pipelineLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        .offset = sizeof(Vector),
        .size = sizeof(float),
        });

    pipelineLayout->create();
}

void SkyboxRenderPass::createRenderPass()
{
    Gfx::PRenderTargetAttachment baseColorAttachment = resources->requestRenderTarget("BASEPASS_COLOR");
    baseColorAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    Gfx::PRenderTargetAttachment depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH");
    depthAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    Gfx::PRenderTargetLayout layout = new Gfx::RenderTargetLayout(baseColorAttachment, depthAttachment);
    renderPass = graphics->createRenderPass(layout, viewport);

    ShaderCreateInfo createInfo;
    createInfo.name = "SkyboxVertex";
    createInfo.mainModule = "Skybox";
    createInfo.entryPoint = "vertexMain";
    createInfo.defines["INDEX_VIEW_PARAMS"] = "0";
    Gfx::PVertexShader vertexShader = graphics->createVertexShader(createInfo);

    createInfo.name = "SkyboxFragment";
    createInfo.entryPoint = "fragmentMain";
    Gfx::PFragmentShader fragmentShader = graphics->createFragmentShader(createInfo);

    Gfx::PVertexDeclaration vertexDecl = graphics->createVertexDeclaration({
        Gfx::VertexElement {
            .streamIndex = 0,
            .offset = 0,
            .vertexFormat = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
            .attributeIndex = 0,
            .stride = sizeof(Vector),
        }
        });

    GraphicsPipelineCreateInfo gfxInfo;
    gfxInfo.vertexDeclaration = vertexDecl;
    gfxInfo.vertexShader = vertexShader;
    gfxInfo.fragmentShader = fragmentShader;
    gfxInfo.rasterizationState.polygonMode = Gfx::SE_POLYGON_MODE_FILL;
    gfxInfo.rasterizationState.lineWidth = 5.f;
    gfxInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    gfxInfo.pipelineLayout = pipelineLayout;
    gfxInfo.renderPass = renderPass;
    pipeline = graphics->createGraphicsPipeline(gfxInfo);
}
