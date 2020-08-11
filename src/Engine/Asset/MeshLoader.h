#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include <thread>
#include <future>
#include <filesystem>

namespace Seele
{
DECLARE_REF(MeshAsset)
DECLARE_NAME_REF(Gfx, Graphics);
class MeshLoader
{
public:
    MeshLoader(Gfx::PGraphics graphic);
    ~MeshLoader();
    void importAsset(const std::filesystem::path& filePath);
private:
    void import(const std::filesystem::path& path);
    List<std::future<void>> futures;
    Gfx::PGraphics graphics;
};
DEFINE_REF(MeshLoader);
} // namespace Seele