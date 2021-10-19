#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include <thread>
#include <future>
#include <filesystem>

namespace Seele
{
DECLARE_REF(MaterialAsset)
DECLARE_NAME_REF(Gfx, Graphics)
class MaterialLoader
{
public:
    MaterialLoader(Gfx::PGraphics graphic);
    ~MaterialLoader();
    PMaterialAsset queueAsset(const std::filesystem::path& filePath);
    PMaterialAsset getPlaceHolderMaterial();
private:
    Gfx::PGraphics graphics;
    List<std::future<void>> futures;
    PMaterialAsset placeholderMaterial;
};
DEFINE_REF(MaterialLoader)
} // namespace Seele