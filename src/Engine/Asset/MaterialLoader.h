#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include <thread>
#include <future>

namespace Seele
{
DECLARE_REF(Material)
DECLARE_NAME_REF(Gfx, Graphics);
class MaterialLoader
{
public:
    MaterialLoader(Gfx::PGraphics graphic);
    ~MaterialLoader();
    PMaterial queueAsset(const std::string& filePath);
private:
    List<std::future<void>> futures;
    PMaterial placeholderMaterial;
};
DEFINE_REF(MaterialLoader);
} // namespace Seele