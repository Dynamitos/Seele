#include "MaterialInstanceAsset.h"
#include "Material/MaterialInstance.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"

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
}

void MaterialInstanceAsset::load(ArchiveBuffer& buffer)
{
}
