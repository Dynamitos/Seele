#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include "ThreadPool.h"
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
    void importAsset(const std::filesystem::path& name);
    PMaterialAsset getPlaceHolderMaterial();
private:
    Job import(std::filesystem::path filePath, PMaterialAsset asset);
    Gfx::PGraphics graphics;
    List<std::future<void>> futures;
    PMaterialAsset placeholderMaterial;
};
DEFINE_REF(MaterialLoader)
} // namespace Seele