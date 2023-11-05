#include "MaterialAsset.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"
#include "MaterialInstanceAsset.h"

using namespace Seele;

MaterialAsset::MaterialAsset()
{
}

MaterialAsset::MaterialAsset(std::string_view folderPath, std::string_view name)
    : Asset(folderPath, name)
{
}

MaterialAsset::~MaterialAsset()
{
}

void MaterialAsset::save(ArchiveBuffer& buffer) const
{
    material->save(buffer);
}

void MaterialAsset::load(ArchiveBuffer& buffer)
{
    material = new Material();
    material->load(buffer);
    material->compile();
}

OMaterialInstanceAsset Seele::MaterialAsset::instantiate(const InstantiationParameter& params)
{
    OMaterialInstance instance = material->instantiate();
    instance->setBaseMaterial(this);
    OMaterialInstanceAsset asset = new MaterialInstanceAsset(params.folderPath, params.name);
    asset->setHandle(std::move(instance));
    return asset;
}


