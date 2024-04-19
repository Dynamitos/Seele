#include "LightEnvironment.h"
#include "Graphics/Graphics.h"

using namespace Seele;

LightEnvironment::LightEnvironment(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    layout = graphics->createDescriptorLayout("pLightEnv");
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
            .size = sizeof(Component::DirectionalLight) * INIT_DIRECTIONAL_LIGHTS,
            .data = nullptr,
        },
        .numElements = INIT_DIRECTIONAL_LIGHTS,
        .dynamic = true,
    });
    pointLights = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(Component::PointLight) * INIT_POINT_LIGHTS,
            .data = nullptr,
        },
        .numElements = INIT_POINT_LIGHTS,
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

    lightEnvBuffer->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT
    );
    pointLights->pipelineBarrier( 
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT
    );
    directionalLights->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT
    );
    lightEnv.numDirectionalLights = dirs.size();
    lightEnv.numPointLights = points.size();
    lightEnvBuffer->updateContents(DataSource{
        .size = sizeof(LightEnv),
        .data = (uint8*) & lightEnv,
        });
    if(directionalLights->getNumElements() < dirs.size())
    {
        directionalLights = graphics->createShaderBuffer({
            .sourceData = {
                .size = sizeof(Component::DirectionalLight) * dirs.size(),
                .data = (uint8*)dirs.data(),
            },
            .numElements = dirs.size(),
            .dynamic = true,
        });
    }
    else
    {
        directionalLights->updateContents(DataSource{
            .size = sizeof(Component::DirectionalLight) * dirs.size(),
            .data = (uint8*)dirs.data(),
            });
    }
    if(pointLights->getNumElements() < points.size())
    {
        pointLights = graphics->createShaderBuffer({
            .sourceData = {
                .size = sizeof(Component::PointLight) * points.size(),
                .data = (uint8*)points.data()
            },
            .numElements = points.size(),
            .dynamic = true
        });
    }
    else
    {
        pointLights->updateContents(DataSource{
            .size = sizeof(Component::PointLight) * points.size(),
            .data = (uint8*)points.data(),
            });
    }
    lightEnvBuffer->pipelineBarrier(
        Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_ALL_COMMANDS_BIT
    );
    pointLights->pipelineBarrier(
        Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_ALL_COMMANDS_BIT
    );
    directionalLights->pipelineBarrier(
        Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_ALL_COMMANDS_BIT
    );
    set->updateBuffer(0, lightEnvBuffer);
    set->updateBuffer(1, directionalLights);
    set->updateBuffer(2, pointLights);
    set->writeChanges();
}

const Gfx::PDescriptorLayout Seele::LightEnvironment::getDescriptorLayout() const
{
    return layout;
}

Gfx::PDescriptorSet Seele::LightEnvironment::getDescriptorSet()
{
    return set;
}
