#include "MaterialAsset.h"
#include "Graphics/WindowManager.h"

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
    descriptorSet->beginFrame();
}

void MaterialAsset::endFrame() 
{
    descriptorSet->endFrame();
}

void MaterialAsset::updateDescriptorData() 
{
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
        descriptorSet->updateBuffer(0, uniformBuffer);
    }
    descriptorSet->writeChanges();
}

Gfx::PDescriptorSet MaterialAsset::getDescriptor() const
{
    return descriptorSet;
}