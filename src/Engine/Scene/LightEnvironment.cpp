#include "LightEnvironment.h"
#include "Asset/AssetRegistry.h"
#include "Asset/EnvironmentMapAsset.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Enums.h"
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
        .name = "shadowMap",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "shadowSampler",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
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
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "prefilteredMap",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "brdfLUT",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "lutSampler",
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
    directionalTransforms.clear();
    points.clear();
}

void LightEnvironment::addDirectionalLight(const Component::DirectionalLight& dirLight, const Component::Transform& transform) {
    dirs.add(ShaderDirectionalLight{
        .color = Vector4(dirLight.color, dirLight.intensity),
        .direction = Vector4(transform.getForward(), 0),
    });
    directionalTransforms.add(transform);
}

void LightEnvironment::addPointLight(const Component::PointLight& pointLight, const Component::Transform& transform) {
    points.add(ShaderPointLight{
        .position_WS = Vector4(transform.getPosition(), pointLight.intensity),
        .colorRange = Vector4(pointLight.color, pointLight.attenuation),
    });
}

void LightEnvironment::commit() {
    directionalLights->rotateBuffer(sizeof(ShaderDirectionalLight) * dirs.size());
    directionalLights->updateContents(0, sizeof(ShaderDirectionalLight) * dirs.size(), dirs.data());
    directionalLights->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                       Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                       Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                           Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    pointLights->rotateBuffer(sizeof(ShaderPointLight) * points.size());
    pointLights->updateContents(0, sizeof(ShaderPointLight) * points.size(), points.data());
    pointLights->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                 Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                 Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                     Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    uint32 numPointLights = (uint32)points.size();
    uint32 numDirectionalLights = (uint32)dirs.size();
    set->updateConstants("numDirectionalLights", 0, &numDirectionalLights);
    set->updateBuffer("directionalLights", 0, directionalLights);
    set->updateConstants("numPointLights", 0, &numPointLights);
    set->updateBuffer("pointLights", 0, pointLights);
    set->updateTexture("irradianceMap", 0, environment->getIrradianceMap()->getDefaultView());
    set->updateSampler("irradianceSampler", 0, environmentSampler);
    set->updateTexture("prefilteredMap", 0, environment->getPrefilteredMap()->getDefaultView());
    set->updateTexture("brdfLUT", 0, environment->getBrdfLUT()->getDefaultView());
    set->updateSampler("lutSampler", 0, environment->getLUTSampler());
    set->writeChanges();
}

const Gfx::PDescriptorLayout LightEnvironment::getDescriptorLayout() const { return layout; }

Gfx::PDescriptorSet LightEnvironment::getDescriptorSet() { return set; }
