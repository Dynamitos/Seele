#include "MaterialInstance.h"
#include "Material.h"
#include "Graphics/Graphics.h"

using namespace Seele;

MaterialInstance::MaterialInstance(Gfx::PGraphics graphics, PMaterial baseMaterial)
    : MaterialInterface(graphics, baseMaterial->parameters, baseMaterial->uniformDataSize, baseMaterial->uniformBinding)
    , baseMaterial(baseMaterial)
{
}

MaterialInstance::~MaterialInstance()
{
    
}

Gfx::PDescriptorSet MaterialInstance::createDescriptorSet()
{
    Gfx::PDescriptorSet descriptorSet = baseMaterial->layout->allocateDescriptorSet();
    BulkResourceData uniformUpdate;
    uniformUpdate.size = uniformDataSize;
    uniformUpdate.data = (uint8*)uniformData.data();
    for(auto param : parameters)
    {
        param->updateDescriptorSet(descriptorSet, uniformData.data());
    }
    if(uniformUpdate.size != 0)
    {
        uniformBuffer->updateContents(uniformUpdate);
        descriptorSet->updateBuffer(uniformBinding, uniformBuffer);
    }
    descriptorSet->writeChanges();
    return descriptorSet;
}

Gfx::PDescriptorLayout MaterialInstance::getDescriptorLayout() const
{
    return baseMaterial->getDescriptorLayout();
}

const std::string& MaterialInstance::getName()
{
    return baseMaterial->getName();
}

const Gfx::ShaderCollection* Seele::MaterialInstance::getShaders(Gfx::RenderPassType renderPass, VertexInputType* vertexInput) const
{
    return baseMaterial->getShaders(renderPass, vertexInput);
}

Gfx::ShaderCollection& Seele::MaterialInstance::createShaders(Gfx::PGraphics graphics, Gfx::RenderPassType renderPass, VertexInputType* vertexInput)
{
    return baseMaterial->createShaders(graphics, renderPass, vertexInput);
}
