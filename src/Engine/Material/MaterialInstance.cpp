#include "MaterialInstance.h"
#include "Material.h"
#include "Graphics/Graphics.h"

using namespace Seele;

MaterialInstance::MaterialInstance()
{
}

MaterialInstance::MaterialInstance(uint64 id, Gfx::PGraphics graphics, PMaterial baseMaterial, Gfx::PDescriptorLayout layout, Array<OShaderParameter> params, uint32 uniformBinding, uint32 uniformSize)
    : id(id), graphics(graphics), baseMaterial(baseMaterial), layout(layout), parameters(params), uniformBinding(uniformBinding)
{
    uniformBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
            .sourceData = {
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

void MaterialInstance::updateDescriptor()
{
    descriptor = layout->allocateDescriptorSet();
    for (auto& param : parameters)
    {
        param->updateDescriptorSet(descriptor, uniformData.data());
    }
    descriptor->writeChanges();
}

Gfx::PDescriptorSet Seele::MaterialInstance::getDescriptorSet() const
{
    return descriptor;
}

void MaterialInstance::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, uniformData);
    Serialization::save(buffer, uniformBinding);
    Serialization::save(buffer, uniformBuffer);
    Serialization::save(buffer, parameters);
    Serialization::save(buffer, id);
}

void MaterialInstance::load(ArchiveBuffer& buffer)
{
    graphics = buffer.getGraphics();
    Serialization::load(buffer, uniformData);
    Serialization::load(buffer, uniformBinding);
    Serialization::load(buffer, uniformBuffer);
    Serialization::load(buffer, parameters);
    Serialization::load(buffer, id);
}

void MaterialInstance::setBaseMaterial(PMaterial material)
{
    layout = material->getDescriptorLayout();
    baseMaterial = material;
    descriptor = layout->allocateDescriptorSet();
    for (auto& param : parameters)
    {
        param->updateDescriptorSet(descriptor, uniformData.data());
    }
    descriptor->writeChanges();
}
