#include "MaterialInstance.h"
#include "Material.h"

using namespace Seele;

MaterialInstance::MaterialInstance()
{
}

MaterialInstance::MaterialInstance(const std::string& directory, const std::string& name) 
    : Asset(directory, name)
{
}

MaterialInstance::MaterialInstance(const std::string& fullPath) 
    : Asset(fullPath)
{   
}

MaterialInstance::MaterialInstance(const std::filesystem::path& fullPath)
    : Asset(fullPath)
{
}

MaterialInstance::~MaterialInstance()
{
}

PMaterial MaterialInstance::getBaseMaterial() const
{
    return baseMaterial;
}

Gfx::PDescriptorSet MaterialInstance::getDescriptor()
{
    return nullptr;
}