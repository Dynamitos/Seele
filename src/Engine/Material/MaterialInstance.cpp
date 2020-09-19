#include "MaterialInstance.h"
#include "Material.h"

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
    
}

PMaterial MaterialInstance::getRenderMaterial() 
{
    return baseMaterial;    
}

PMaterial MaterialInstance::getBaseMaterial() const
{
    return baseMaterial;
}

Gfx::PDescriptorSet MaterialInstance::getDescriptor()
{
    return nullptr;
}