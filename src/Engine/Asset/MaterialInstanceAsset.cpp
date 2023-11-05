#include "MaterialInstanceAsset.h"
#include "Material/MaterialInstance.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"
#include "Asset/AssetRegistry.h"

using namespace Seele;

MaterialInstanceAsset::MaterialInstanceAsset()
{
}

MaterialInstanceAsset::MaterialInstanceAsset(std::string_view folderPath, std::string_view name)
    : Asset(folderPath, name)
{
}

MaterialInstanceAsset::~MaterialInstanceAsset()
{
}


void MaterialInstanceAsset::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, baseMaterial->getAssetIdentifier());
    material->save(buffer);
}

void MaterialInstanceAsset::load(ArchiveBuffer& buffer)
{
    std::string id;
    Serialization::load(buffer, id);
    material->load(buffer);
    baseMaterial = AssetRegistry::findMaterial(id);
    material->setBaseMaterial(baseMaterial);
}
