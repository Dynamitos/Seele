#include "FontAsset.h"
#include "Graphics/Graphics.h"

using namespace Seele;

FontAsset::FontAsset()
{
}

FontAsset::FontAsset(const std::string& directory, const std::string& name)
    : Asset(directory, name)
{
}

FontAsset::FontAsset(const std::filesystem::path& fullPath)
    : Asset(fullPath)
{
}

FontAsset::~FontAsset()
{
    
}

void FontAsset::save(Gfx::PGraphics graphics)
{
    assert(false && "Cannot save font files");
}


void FontAsset::load(Gfx::PGraphics graphics)
{
}