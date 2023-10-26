#include "MaterialInstance.h"
#include "Material.h"
#include "Graphics/Graphics.h"

using namespace Seele;

MaterialInstance::MaterialInstance(uint64 id, Gfx::PGraphics graphics, PMaterial baseMaterial, Gfx::PDescriptorLayout descriptor, Array<PShaderParameter> params, uint32 uniformBinding, uint32 uniformSize)
    : id(id), graphics(graphics), baseMaterial(baseMaterial), layout(layout), parameters(params), uniformBinding(uniformBinding)
{
    uniformBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
            .resourceData = {
                .size = uniformSize,
            },
            .bDynamic = true,
        }
    );
    uniformData.resize(uniformSize);
}

MaterialInstance::~MaterialInstance()
{
    
}

void Seele::MaterialInstance::updateDescriptor()
{
}
