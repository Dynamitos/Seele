#include "MaterialInstance.h"
#include "Material.h"
#include "Graphics/Graphics.h"

using namespace Seele;

MaterialInstance::MaterialInstance()
{
}

MaterialInstance::MaterialInstance(uint64 id, 
    Gfx::PGraphics graphics, 
    Array<OShaderExpression>& expressions,
    Array<std::string> params, 
    uint32 uniformBinding, 
    uint32 uniformSize)
    : graphics(graphics)
    , uniformBinding(uniformBinding)
    , id(id)
{
    if(uniformSize > 0)
    {
        uniformBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
                .sourceData = {
                    .size = uniformSize,
                },
                .dynamic = true,
            }
        );
    }
    uniformData.resize(uniformSize);
    ArchiveBuffer buffer(graphics);
    parameters.reserve(params.size());
    for (size_t i = 0; i < params.size(); ++i)
    {
        const std::string& name = params[i];
        Serialization::save(buffer, *expressions.find([&name](const OShaderExpression& p) {return p->key == name; }));
        buffer.rewind();
        OShaderParameter param;
        Serialization::load(buffer, param);
        parameters.add(std::move(param));
        buffer.rewind();
    }
}

MaterialInstance::~MaterialInstance()
{
    
}

void MaterialInstance::updateDescriptor()
{
    Gfx::PDescriptorLayout layout = baseMaterial->getMaterial()->getDescriptorLayout();
    descriptor = layout->allocateDescriptorSet();
    for (auto& param : parameters)
    {
        param->updateDescriptorSet(descriptor, uniformData.data());
    }
    if(uniformData.size() > 0)
    {
        uniformBuffer->updateContents(DataSource{
            .size = uniformData.size(),
            .data = uniformData.data(),
        });
        descriptor->updateBuffer(uniformBinding, uniformBuffer);
    }
    descriptor->writeChanges();
}

Gfx::PDescriptorSet MaterialInstance::getDescriptorSet() const
{
    return descriptor;
}

void MaterialInstance::setBaseMaterial(PMaterialAsset asset)
{
    if(uniformData.size() > 0)
    {
        uniformBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
                .sourceData = {
                    .size = uniformData.size(),
                },
                .dynamic = true,
            }
        );
    }
    baseMaterial = asset;
    updateDescriptor();
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
