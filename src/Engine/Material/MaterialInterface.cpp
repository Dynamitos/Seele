#include "MaterialInterface.h"
#include "Window/WindowManager.h"
#include "Graphics/Graphics.h"

using namespace Seele;

MaterialInterface::MaterialInterface(Gfx::PGraphics graphics, Array<PShaderParameter> parameter, uint32 uniformDataSize, uint32 uniformBinding)
    : graphics(graphics)
    , parameters(parameter)
    , uniformDataSize(uniformDataSize)
    , uniformBinding(uniformBinding)
{
    if(uniformDataSize != 0)
    {
        uniformData.resize(uniformDataSize);
        UniformBufferCreateInfo uniformInitializer = {
            .resourceData = {
                .size = uniformDataSize,
                .data = nullptr,
            }
        };
        uniformBuffer = graphics->createUniformBuffer(uniformInitializer);
    }
}

MaterialInterface::~MaterialInterface()
{
}


PShaderParameter MaterialInterface::getParameter(const std::string& name)
{
    for (const auto& param : parameters)
    {
        if(param->name.compare(name) == 0)
        {
            return param;
        }
    }
    return nullptr;
}

void MaterialInterface::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, uniformDataSize);
    Serialization::save(buffer, uniformBinding);
    uint64 length = parameters.size();
    Serialization::save(buffer, length);
    for (const auto& param : parameters)
    {
        Serialization::save(buffer, param);
    }
}

void MaterialInterface::load(ArchiveBuffer& buffer)
{
    graphics = buffer.getGraphics();
    Serialization::load(buffer, uniformDataSize);
    Serialization::load(buffer, uniformBinding);

    if (uniformDataSize != 0)
    {
        uniformData.resize(uniformDataSize);
        UniformBufferCreateInfo uniformInitializer = {
            .resourceData = {
                .size = uniformDataSize,
                .data = nullptr,
            }
        };
        uniformBuffer = graphics->createUniformBuffer(uniformInitializer);
    }
    uint64 length;
    Serialization::load(buffer, length);
    parameters.resize(length);
    for (auto& param : parameters)
    {
        Serialization::load(buffer, param);
    }
}
