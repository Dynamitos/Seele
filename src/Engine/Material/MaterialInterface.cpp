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
    for (auto param : parameters)
    {
        if(param->name.compare(name) == 0)
        {
            return param;
        }
    }
    return nullptr;
}