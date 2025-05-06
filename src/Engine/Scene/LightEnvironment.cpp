#include "LightEnvironment.h"
#include "Asset/AssetRegistry.h"
#include "Asset/EnvironmentMapAsset.h"
#include "Graphics/Graphics.h"

using namespace Seele;

LightEnvironment::LightEnvironment(Gfx::PGraphics graphics)
    : graphics(graphics), environment(AssetRegistry::findEnvironmentMap("", "newport_loft")) {
    layout = graphics->createDescriptorLayout("pLightEnv");
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "directionalLights",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "numDirectionalLights",
        .uniformLength = sizeof(uint32),
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "pointLights",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "numPointLights",
        .uniformLength = sizeof(uint32),
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "irradianceMap",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "irradianceSampler",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    layout->create();

    directionalLights = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .name = "DirectionalLights",
    });
    pointLights = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .name = "PointLights",
    });
    environmentSampler = graphics->createSampler({
        .magFilter = Gfx::SE_FILTER_LINEAR,
        .minFilter = Gfx::SE_FILTER_LINEAR,
        .addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    });
}

LightEnvironment::~LightEnvironment() {}

void LightEnvironment::reset() {
    layout->reset();
    set = layout->allocateDescriptorSet();
    dirs.clear();
    points.clear();
}

void LightEnvironment::addDirectionalLight(Component::DirectionalLight dirLight) { dirs.add(dirLight); }

void LightEnvironment::addPointLight(Component::PointLight pointLight) { points.add(pointLight); }

void LightEnvironment::commit() {
    directionalLights->rotateBuffer(sizeof(Component::DirectionalLight) * dirs.size());
    directionalLights->updateContents(0, sizeof(Component::DirectionalLight) * dirs.size(), dirs.data());
    directionalLights->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                       Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                       Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                           Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    pointLights->rotateBuffer(sizeof(Component::PointLight) * points.size());
    pointLights->updateContents(0, sizeof(Component::PointLight) * points.size(), points.data());
    pointLights->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                 Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                 Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                     Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    uint32 numPointLights = (uint32)points.size();
    uint32 numDirectionalLights = (uint32)dirs.size();
    set->updateConstants("numPointLights", 0, &numPointLights);
    set->updateBuffer("pointLights", 0, pointLights);
    set->updateConstants("numDirectionalLights", 0, &numDirectionalLights);
    set->updateBuffer("directionalLights", 0, directionalLights);
    set->updateTexture("irradianceMap", 0, environment->getIrradianceMap());
    set->updateSampler("irradianceSampler", 0, environmentSampler);
    set->writeChanges();

    while (shadowMapArray.size() < dirs.size()) {
        shadowMapArray.add(graphics->createTexture2D(TextureCreateInfo{
            .format = Gfx::SE_FORMAT_D32_SFLOAT,
            .width = 2048,
            .height = 2048,
            .elements = (uint32)dirs.size(),
            .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | Gfx::SE_IMAGE_USAGE_SAMPLED_BIT,
            .name = "ShadowMap",
        }));
    }
}

const Gfx::PDescriptorLayout LightEnvironment::getDescriptorLayout() const { return layout; }

Gfx::PDescriptorSet LightEnvironment::getDescriptorSet() { return set; }
