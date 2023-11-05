#include "MaterialInstance.h"
#include "Material.h"
#include "Graphics/Graphics.h"

using namespace Seele;

MaterialInstance::MaterialInstance()
{
}

MaterialInstance::MaterialInstance(uint64 id, 
    Gfx::PGraphics graphics, 
    Map<std::string, OShaderExpression>& expressions,
    Array<std::string> params, 
    uint32 uniformBinding, 
    uint32 uniformSize)
    : id(id)
    , graphics(graphics)
    , uniformBinding(uniformBinding)
{
    uniformBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
            .sourceData = {
                .size = uniformSize,
            },
            .dynamic = true,
        }
    );
    uniformData.resize(uniformSize);
    ArchiveBuffer buffer(graphics);
    parameters.reserve(params.size());
    for (int i = 0; i < params.size(); ++i)
    {
        Serialization::save(buffer, params[i]);
        buffer.rewind();
        OShaderParameter param;
        Serialization::load(buffer, param);
        parameters.add(std::move(param));
    }
}

MaterialInstance::~MaterialInstance()
{
    
}

void MaterialInstance::updateDescriptor()
{
    Gfx::PDescriptorLayout layout = baseMaterial->getMaterial()->getDescriptorLayout();
    layout->reset();
    descriptor = layout->allocateDescriptorSet();
    for (auto& param : parameters)
    {
        param->updateDescriptorSet(descriptor, uniformData.data());
    }
    descriptor->writeChanges();
}

Gfx::PDescriptorSet MaterialInstance::getDescriptorSet() const
{
    return descriptor;
}

void MaterialInstance::setBaseMaterial(PMaterialAsset asset)
{
    uniformBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
            .sourceData = {
                .size = uniformData.size(),
            },
            .dynamic = true,
        }
    );
    baseMaterial = asset;
}

void MaterialInstance::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, uniformData);
    Serialization::save(buffer, uniformBinding);
    Serialization::save(buffer, parameters);
    Serialization::save(buffer, id);
}

void MaterialInstance::load(ArchiveBuffer& buffer)
{
    graphics = buffer.getGraphics();
    Serialization::load(buffer, uniformData);
    Serialization::load(buffer, uniformBinding);
    Serialization::load(buffer, parameters);
    Serialization::load(buffer, id);
}
