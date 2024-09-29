#include "TerrainRenderer.h"
#include "Asset/AssetRegistry.h"
#include "Component/TerrainTile.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"

using namespace Seele;

TerrainRenderer::TerrainRenderer(Gfx::PGraphics graphics, PScene scene, Gfx::PDescriptorLayout viewParamsLayout)
    : graphics(graphics), scene(scene) {
    struct TerrainTile {
        IVector2 offset;
        float extent;
        float height;
    };
    Array<TerrainTile> payloads;
    for (int32 y = -100; y < 100; ++y) {
        for (int32 x = -100; x < 100; ++x) {
            payloads.add(TerrainTile{
                .offset = IVector2(x, y),
                .extent = Component::TerrainTile::DIMENSIONS,
                .height = 0,
            });
        }
    }
    tilesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(TerrainTile) * payloads.size(),
                .data = (uint8*)payloads.data(),
            },
        .numElements = payloads.size(),
        .dynamic = true,
        .name = "TilesBuffer",
    });
    tilesBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                 Gfx::SE_PIPELINE_STAGE_TASK_SHADER_BIT_EXT);
    colorMap = AssetRegistry::findTexture("", "azeroth")->getTexture();
    displacementMap = AssetRegistry::findTexture("", "azeroth_height")->getTexture();

    sampler = graphics->createSampler(SamplerCreateInfo{});
    layout = graphics->createDescriptorLayout("pTerrainData");
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 2,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 3,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    layout->create();
    pipelineLayout = graphics->createPipelineLayout("TerrainLayout");
    pipelineLayout->addDescriptorLayout(viewParamsLayout);
    pipelineLayout->addDescriptorLayout(layout);
    ShaderCompilationInfo s{
        .name = "TerrainShaders",
        .modules = {"TerrainPass"},
        .entryPoints =
            {
                {"taskMain", "TerrainPass"},
                {"meshMain", "TerrainPass"},
                {"fragmentMain", "TerrainPass"},
            },
        .rootSignature = pipelineLayout,
        .dumpIntermediate = false,
    };
    graphics->beginShaderCompilation(s);
    task = graphics->createTaskShader({0});
    mesh = graphics->createMeshShader({1});
    frag = graphics->createFragmentShader({2});
    pipelineLayout->create();
}

TerrainRenderer::~TerrainRenderer() {}

void TerrainRenderer::beginFrame() {
    set = layout->allocateDescriptorSet();
    set->updateBuffer(0, 0, tilesBuffer);
    set->updateTexture(1, 0, displacementMap);
    set->updateTexture(2, 0, colorMap);
    set->updateSampler(3, 0, sampler);
    set->writeChanges();
}

Gfx::ORenderCommand TerrainRenderer::render(Gfx::PDescriptorSet viewParamsSet) {
    Gfx::ORenderCommand command = graphics->createRenderCommand("TerrainRender");
    command->setViewport(viewport);
    command->bindPipeline(pipeline);
    command->bindDescriptor({viewParamsSet, set});
    command->drawMesh(tilesBuffer->getNumElements(), 4, 1);
    return command;
}

void TerrainRenderer::setViewport(Gfx::PViewport _viewport, Gfx::PRenderPass renderPass) {
    viewport = _viewport;

    Gfx::MeshPipelineCreateInfo pipelineInfo = {
        .taskShader = task,
        .meshShader = mesh,
        .fragmentShader = frag,
        .renderPass = renderPass,
        .pipelineLayout = pipelineLayout,
        .multisampleState =
            {
                .samples = viewport->getSamples(),
            },
        .rasterizationState =
            {
                //.polygonMode = Gfx::SE_POLYGON_MODE_LINE,
                .cullMode = Gfx::SE_CULL_MODE_BACK_BIT,
            },
        .colorBlend =
            {
                .attachmentCount = 1,
                .blendAttachments =
                    {
                        Gfx::ColorBlendState::BlendAttachment{
                            .blendEnable = false,
                        },
                    },
            },
    };
    pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
}