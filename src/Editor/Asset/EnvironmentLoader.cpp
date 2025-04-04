#include "EnvironmentLoader.h"
#include "Asset/AssetRegistry.h"
#include "Asset/EnvironmentMapAsset.h"
#include "Graphics/Descriptor.h"
#include "stb_image.h"

using namespace Seele;

EnvironmentLoader::EnvironmentLoader(Gfx::PGraphics graphics) : graphics(graphics) {
    cubeRenderViewport = graphics->createViewport(nullptr, ViewportCreateInfo{
                                                               .dimensions =
                                                                   {
                                                                       .size = {1024, 1024},
                                                                       .offset = {0, 0},
                                                                   },
                                                           });

    cubeRenderLayout = graphics->createDescriptorLayout("pViewParams");
    cubeRenderLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "view",
        .uniformLength = sizeof(Matrix4) * 6,
    });
    cubeRenderLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "projection",
        .uniformLength = sizeof(Matrix4),
    });
    cubeRenderLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "equirectangularMap",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    cubeRenderLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "sampler",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
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
                {"fragMain", "EnvironmentMapping"},
            },
        .rootSignature = cubePipelineLayout,
    });
    cubePipelineLayout->create();

    cubeRenderVertex = graphics->createVertexShader({0});
    cubeRenderFrag = graphics->createFragmentShader({1});

    cubeSampler = graphics->createSampler({
        .magFilter = Gfx::SE_FILTER_LINEAR,
        .minFilter = Gfx::SE_FILTER_LINEAR,
        .addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    });
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
    Matrix4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    Matrix4 captureViews[] = {
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f), Vector(0.0f, 1.0f, 0.0f)),
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, -1.0f), Vector(0.0f, 1.0f, 0.0f)),
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, -1.0f, 0.0f), Vector(1.0f, 0.0f, 0.0f)),
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f), Vector(1.0f, 0.0f, 0.0f)),
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(-1.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f)),
        glm::lookAt(Vector(0.0f, 0.0f, 0.0f), Vector(1.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f)),
    };
    Gfx::PDescriptorSet set = cubeRenderLayout->allocateDescriptorSet();
    set->updateConstants("view", 0, captureViews);
    set->updateConstants("projection", 0, &captureProjection);
    set->updateTexture("equirectangularMap", 0, Gfx::PTexture2D(hdrTexture));
    set->updateSampler("sampler", 0, cubeSampler);
    set->writeChanges();
    Gfx::OTextureCube cubeMap = graphics->createTextureCube(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = 1024,
        .height = 1024,
        .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    });
    cubeRenderPass = graphics->createRenderPass(
        Gfx::RenderTargetLayout{
            .colorAttachments = {Gfx::RenderTargetAttachment(cubeMap, Gfx::SE_IMAGE_LAYOUT_UNDEFINED,
                                                             Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                             Gfx::SE_ATTACHMENT_LOAD_OP_DONT_CARE, Gfx::SE_ATTACHMENT_STORE_OP_STORE)},
        },
        {},
        URect{
            .size = {1024, 1024},
            .offset = {0, 0},
        },
        "EnvironmentRenderPass", {0b111111}, {0b111111});
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
    renderCommand->draw(6, 1, 0, 0);
    graphics->executeCommands(std::move(renderCommand));
    graphics->endRenderPass();
    graphics->waitDeviceIdle();
    asset->diffuseMap = std::move(cubeMap);
}
