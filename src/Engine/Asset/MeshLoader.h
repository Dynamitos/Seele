#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include <thread>
#include <future>

namespace Seele
{
DECLARE_REF(MeshAsset)
DECLARE_NAME_REF(Gfx, Graphics);
class MeshLoader
{
public:
    MeshLoader(Gfx::PGraphics graphic);
    ~MeshLoader();
    void importAsset(const std::string& filePath);
private:
    void import(const std::string& path);
    List<std::future<void>> futures;
    Gfx::PGraphics graphics;
};
DEFINE_REF(MeshLoader);
} // namespace Seele