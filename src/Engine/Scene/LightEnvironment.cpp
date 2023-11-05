#include "LightEnvironment.h"
#include "Graphics/Graphics.h"

using namespace Seele;

LightEnvironment::LightEnvironment(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    layout = graphics->createDescriptorLayout("LightEnvironment");
    layout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    layout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    layout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    layout->create();
    lightEnvBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData = {
            .size = sizeof(LightEnv),
            .data = (uint8*) &lightEnv,
        },
        .dynamic = true,
    });
    directionalLights = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(Component::DirectionalLight) * MAX_DIRECTIONAL_LIGHTS,
            .data = (uint8*)dirs.data(),
        },
        .stride = sizeof(Component::DirectionalLight),
        .dynamic = true,
    });
    pointLights = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(Component::PointLight) * MAX_POINT_LIGHTS,
            .data = (uint8*)dirs.data(),
        },
        .stride = sizeof(Component::PointLight),
        .dynamic = true,
    });
}

LightEnvironment::~LightEnvironment()
{
}

void LightEnvironment::reset()
{
    layout->reset();
    set = layout->allocateDescriptorSet();
    dirs.clear();
    points.clear();
}

void LightEnvironment::addDirectionalLight(Component::DirectionalLight dirLight)
{
    dirs.add(dirLight);
}

void LightEnvironment::addPointLight(Component::PointLight pointLight)
{
    points.add(pointLight);
}

void LightEnvironment::commit()
{
    lightEnv.numDirectionalLights = dirs.size();
    lightEnv.numPointLights = points.size();
    lightEnvBuffer->updateContents(DataSource{
        .size = sizeof(LightEnv),
        .data = (uint8*) & lightEnv,
        });
    directionalLights->updateContents(DataSource{
        .size = sizeof(Component::DirectionalLight) * dirs.size(),
        .data = (uint8*)dirs.data(),
        });
    pointLights->updateContents(DataSource{
        .size = sizeof(Component::PointLight) * points.size(),
        .data = (uint8*)points.data(),
        });
    set->updateBuffer(0, lightEnvBuffer);
    set->updateBuffer(1, directionalLights);
    set->updateBuffer(2, pointLights);
}

const Gfx::PDescriptorLayout Seele::LightEnvironment::getDescriptorLayout() const
{
    return layout;
}

Gfx::PDescriptorSet Seele::LightEnvironment::getDescriptorSet()
{
    return set;
}
