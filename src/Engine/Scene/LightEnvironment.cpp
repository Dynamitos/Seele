#include "LightEnvironment.h"
#include "Graphics/Graphics.h"

using namespace Seele;

LightEnvironment::LightEnvironment(Gfx::PGraphics graphics) : graphics(graphics) {
    layout = graphics->createDescriptorLayout("pLightEnv");
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 2,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    layout->create();
    lightEnvBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .dynamic = true,
    });
    directionalLights = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .dynamic = true,
    });
    pointLights = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .dynamic = true,
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
    lightEnv.numDirectionalLights = dirs.size();
    lightEnv.numPointLights = points.size();
    lightEnvBuffer->updateContents(DataSource{
        .size = sizeof(LightEnv),
        .data = (uint8*)&lightEnv,
    });
    lightEnvBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                    Gfx::SE_ACCESS_UNIFORM_READ_BIT | Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                    Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                        Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    directionalLights->rotateBuffer(sizeof(Component::DirectionalLight) * dirs.size());
    directionalLights->updateContents({
        .sourceData =
            {
                .size = sizeof(Component::DirectionalLight) * dirs.size(),
                .data = (uint8*)dirs.data(),
            },
        .numElements = dirs.size(),
    });
    directionalLights->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                       Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                       Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                           Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    pointLights->rotateBuffer(sizeof(Component::PointLight) * points.size());
    pointLights->updateContents({
        .sourceData =
            {
                .size = sizeof(Component::PointLight) * points.size(),
                .data = (uint8*)points.data(),
            },
        .numElements = points.size(),
    });
    pointLights->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                 Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                 Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                     Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    set->updateBuffer(0, lightEnvBuffer);
    set->updateBuffer(1, directionalLights);
    set->updateBuffer(2, pointLights);
    set->writeChanges();
}

const Gfx::PDescriptorLayout LightEnvironment::getDescriptorLayout() const { return layout; }

Gfx::PDescriptorSet LightEnvironment::getDescriptorSet() { return set; }
