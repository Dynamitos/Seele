#include "MaterialInstance.h"
#include "Material.h"
#include "Graphics/WindowManager.h"

using namespace Seele;

MaterialInstance::MaterialInstance()
{
}

MaterialInstance::MaterialInstance(const std::string& directory, const std::string& name) 
    : MaterialAsset(directory, name)
{
}

MaterialInstance::MaterialInstance(const std::filesystem::path& fullPath)
    : MaterialAsset(fullPath)
{
}

MaterialInstance::~MaterialInstance()
{
}

void MaterialInstance::save() 
{
    
}

void MaterialInstance::load() 
{
    baseMaterial = nullptr; // TODO: actually load the file
    UniformBufferCreateInfo uniformInitializer;
    uniformInitializer.resourceData.size = baseMaterial->uniformDataSize;
    uniformInitializer.resourceData.data = nullptr;
    uniformBuffer = WindowManager::getGraphics()->createUniformBuffer(uniformInitializer);
    descriptorSet = baseMaterial->layout->allocatedDescriptorSet();
}

const Material* MaterialInstance::getRenderMaterial() const
{
    return baseMaterial;    
}
