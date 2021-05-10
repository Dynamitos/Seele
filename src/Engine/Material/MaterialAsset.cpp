#include "MaterialAsset.h"
#include "Window/WindowManager.h"

using namespace Seele;

MaterialAsset::MaterialAsset()
{
}

MaterialAsset::MaterialAsset(const std::string& directory, const std::string& name) 
    : Asset(directory, name)
{
}

MaterialAsset::MaterialAsset(const std::filesystem::path& fullPath) 
    : Asset(fullPath)
{
}

MaterialAsset::~MaterialAsset()
{
}

void MaterialAsset::beginFrame() 
{
}

void MaterialAsset::endFrame() 
{
}

void MaterialAsset::updateDescriptorData() 
{
    layout->reset();
    descriptorSet = layout->allocateDescriptorSet();
    BulkResourceData uniformUpdate;
    uniformUpdate.size = uniformDataSize;
    uniformUpdate.data = uniformData;
    for(auto param : parameters)
    {
        param->updateDescriptorSet(descriptorSet, uniformData);
    }
    if(uniformUpdate.size != 0)
    {
        uniformBuffer->updateContents(uniformUpdate);
        descriptorSet->updateBuffer(uniformBinding, uniformBuffer);
    }
    descriptorSet->writeChanges();
}

const Gfx::PDescriptorSet MaterialAsset::getDescriptor() const
{
    return descriptorSet;
}