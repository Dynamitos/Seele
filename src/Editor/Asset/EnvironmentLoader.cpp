#include "EnvironmentLoader.h"
#include "Asset/AssetRegistry.h"
#include "Asset/EnvironmentMapAsset.h"
#include "Graphics/Descriptor.h"
#include "stb_image.h"

using namespace Seele;

static constexpr uint32 SOURCE_RESOLUTION = 2048;
static constexpr uint32 CONVOLUTED_RESOLUTION = 64;

EnvironmentLoader::EnvironmentLoader(Gfx::PGraphics graphics) : graphics(graphics) {
    cubeRenderViewport = graphics->createViewport(nullptr, ViewportCreateInfo{
                                                               .dimensions =
                                                                   {
                                                                       .size = {SOURCE_RESOLUTION, SOURCE_RESOLUTION},
                                                                       .offset = {0, 0},
                                                                   },
                                                               .fieldOfView = glm::radians(90.0f),
                                                           });
    convolutionViewport = graphics->createViewport(nullptr, ViewportCreateInfo{
                                                                .dimensions =
                                                                    {
                                                                        .size = {CONVOLUTED_RESOLUTION, CONVOLUTED_RESOLUTION},
                                                                        .offset = {0, 0},
                                                                    },
                                                                .fieldOfView = glm::radians(90.0f),
                                                            });
    cubeSampler = graphics->createSampler({
        .magFilter = Gfx::SE_FILTER_LINEAR,
        .minFilter = Gfx::SE_FILTER_LINEAR,
        .addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    });

    cubeRenderLayout = graphics->createDescriptorLayout("pViewParams");
    cubeRenderLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "view",
        .uniformLength = sizeof(Matrix4) * 6,
        .descriptorCount = 24,
    });
    cubeRenderLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "projection",
        .uniformLength = sizeof(Matrix4),
        .descriptorCount = 4,
    });
    cubeRenderLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "equirectangularMap",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    cubeRenderLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "sampler",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    cubeRenderLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "cubeMap",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    cubeRenderLayout->create();

    cubePipelineLayout = graphics->createPipelineLayout("CubeRenderLayout");
    cubePipelineLayout->addDescriptorLayout(cubeRenderLayout);

    graphics->beginShaderCompilation(ShaderCompilationInfo{
        .name = "CubeRenderPipeline",
        .modules = {"EnvironmentMapping"},
        .entryPoints =
            {
                {"vertMain", "EnvironmentMapping"},
                {"computeCubemap", "EnvironmentMapping"},
                {"convolveCubemap", "EnvironmentMapping"},
            },
        .rootSignature = cubePipelineLayout,
    });
    cubePipelineLayout->create();

    cubeRenderVertex = graphics->createVertexShader({0});
    cubeRenderFrag = graphics->createFragmentShader({1});
    convolutionFrag = graphics->createFragmentShader({2});
}

EnvironmentLoader::~EnvironmentLoader() {}

void EnvironmentLoader::importAsset(EnvironmentImportArgs args) {
    std::filesystem::path assetPath = args.filePath.filename();
    assetPath.replace_extension("asset");
    OEnvironmentMapAsset asset = new EnvironmentMapAsset(args.importPath, assetPath.stem().string());
    asset->setStatus(Asset::Status::Loading);
    // the registry takes ownership, but we need to edit the reference
    PEnvironmentMapAsset ref = asset;
    AssetRegistry::get().registerEnvironmentMap(std::move(asset));
    import(args, ref);
}

void EnvironmentLoader::import(EnvironmentImportArgs args, PEnvironmentMapAsset asset) {
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(args.filePath.string().c_str(), &width, &height, &nrComponents, 4);
    stbi_set_flip_vertically_on_load(false);
    assert(data);
    Gfx::OTexture2D hdrTexture = graphics->createTexture2D(TextureCreateInfo{
        .sourceData =
            {
                .size = width * height * 4 * sizeof(float),
                .data = (uint8*)data,
            },
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = (uint32)width,
        .height = (uint32)height,
        .name = "HDRRaw",
    });
    Matrix4 captureProjection = cubeRenderViewport->getProjectionMatrix(0.1f, 10.0f);
    Matrix4 captureViews[] = {
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(1.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f)),
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(-1.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f)),
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f)),
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, -1.0f, 0.0f), Vector(0.0f, 0.0f, -1.0f)),
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, -1.0f), Vector(0.0f, 1.0f, 0.0f)),
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f), Vector(0.0f, 1.0f, 0.0f)),
    };
    Gfx::PDescriptorSet set = cubeRenderLayout->allocateDescriptorSet();
    set->updateConstants("view", 0, captureViews);
    set->updateConstants("projection", 0, &captureProjection);
    set->updateTexture("equirectangularMap", 0, hdrTexture->getDefaultView());
    set->updateSampler("sampler", 0, cubeSampler);
    set->writeChanges();
    Gfx::OTextureCube cubeMap = graphics->createTextureCube(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = SOURCE_RESOLUTION,
        .height = SOURCE_RESOLUTION,
        .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | Gfx::SE_IMAGE_USAGE_SAMPLED_BIT,
    });
    Array<Gfx::OTextureView> cubeViews;
    cubeViews.add(cubeMap->createTextureView(0, 1, 0, 1));
    cubeViews.add(cubeMap->createTextureView(0, 1, 1, 1));
    cubeViews.add(cubeMap->createTextureView(0, 1, 2, 1));
    cubeViews.add(cubeMap->createTextureView(0, 1, 3, 1));
    cubeViews.add(cubeMap->createTextureView(0, 1, 4, 1));
    cubeViews.add(cubeMap->createTextureView(0, 1, 5, 1));
    for(uint32 i = 0; i < 6; ++i) {
        Gfx::ORenderPass cubeRenderPass = graphics->createRenderPass(
            Gfx::RenderTargetLayout{
                .colorAttachments =
                    {
                        Gfx::RenderTargetAttachment(cubeViews[i], Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                    Gfx::SE_ATTACHMENT_LOAD_OP_DONT_CARE, Gfx::SE_ATTACHMENT_STORE_OP_STORE),
                    },
            },
            {
                Gfx::SubPassDependency{
                    .srcSubpass = 0,
                    .dstSubpass = ~0U,
                    .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .dstStage = Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    .dstAccess = Gfx::SE_ACCESS_SHADER_READ_BIT,
                },
            },
            {
                .size = {SOURCE_RESOLUTION, SOURCE_RESOLUTION},
                .offset = {0, 0},
            },
            "EnvironmentRenderPass");
        cubeRenderPipeline = graphics->createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo{
            .vertexShader = cubeRenderVertex,
            .fragmentShader = cubeRenderFrag,
            .renderPass = cubeRenderPass,
            .pipelineLayout = cubePipelineLayout,
            .colorBlend =
                {
                    .attachmentCount = 1,
                },
        });
        graphics->beginRenderPass(cubeRenderPass);
        Gfx::ORenderCommand renderCommand = graphics->createRenderCommand("CubeMapRender");
        renderCommand->bindPipeline(cubeRenderPipeline);
        renderCommand->bindDescriptor({set});
        renderCommand->setViewport(cubeRenderViewport);
        renderCommand->draw(6, 1, i*6, 0);
        graphics->executeCommands(std::move(renderCommand));
        graphics->endRenderPass();
    }
    
    set = cubeRenderLayout->allocateDescriptorSet();
    set->updateConstants("view", 0, captureViews);
    set->updateConstants("projection", 0, &captureProjection);
    set->updateSampler("sampler", 0, cubeSampler);
    set->updateTexture("cubeMap", 0, cubeMap->getDefaultView());
    set->writeChanges();
    
    Gfx::OTextureCube convolutedMap = graphics->createTextureCube(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = CONVOLUTED_RESOLUTION,
        .height = CONVOLUTED_RESOLUTION,
        .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    });
    Array<Gfx::OTextureView> convolutedViews;
    convolutedViews.add(cubeMap->createTextureView(0, 1, 0, 1));
    convolutedViews.add(cubeMap->createTextureView(0, 1, 1, 1));
    convolutedViews.add(cubeMap->createTextureView(0, 1, 2, 1));
    convolutedViews.add(cubeMap->createTextureView(0, 1, 3, 1));
    convolutedViews.add(cubeMap->createTextureView(0, 1, 4, 1));
    convolutedViews.add(cubeMap->createTextureView(0, 1, 5, 1));
    for(uint32 i = 0; i < 6; ++i) {
        Gfx::ORenderPass convolutionPass = graphics->createRenderPass(
            Gfx::RenderTargetLayout{
                .colorAttachments = {Gfx::RenderTargetAttachment(convolutedViews[i], Gfx::SE_IMAGE_LAYOUT_UNDEFINED,
                                                                Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                                Gfx::SE_ATTACHMENT_LOAD_OP_DONT_CARE, Gfx::SE_ATTACHMENT_STORE_OP_STORE)},
            },
            {},
            {
                .size = {CONVOLUTED_RESOLUTION, CONVOLUTED_RESOLUTION},
                .offset = {0, 0},
            },
            "EnvironmentRenderPass");
        convolutionPipeline = graphics->createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo{
            .vertexShader = cubeRenderVertex,
            .fragmentShader = convolutionFrag,
            .renderPass = convolutionPass,
            .pipelineLayout = cubePipelineLayout,
            .colorBlend =
                {
                    .attachmentCount = 1,
                },
        });

        graphics->beginRenderPass(convolutionPass);
        Gfx::ORenderCommand cmd = graphics->createRenderCommand("ConvolutionPass");
        cmd->bindPipeline(convolutionPipeline);
        cmd->bindDescriptor({set});
        cmd->setViewport(convolutionViewport);
        cmd->draw(6, 1, i*6, 0);
        graphics->executeCommands(std::move(cmd));
        graphics->endRenderPass();

    }
    
    asset->skybox = std::move(cubeMap);
    asset->irradianceMap = std::move(convolutedMap);
    graphics->waitDeviceIdle();
}
