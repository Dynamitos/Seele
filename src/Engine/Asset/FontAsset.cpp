#include "FontAsset.h"
#include "Graphics/GraphicsResources.h"
#define TTF_FONT_PARSER_IMPLEMENTATION
#include "ttfParser.h"

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

void FontAsset::save()
{
    assert(false && "Cannot save font files");
}

void FontAsset::load()
{
    TTFFontParser::FontData font_data;
    int8_t error = TTFFontParser::parse_file(getFullPath().c_str(), &font_data, [](void*, void*, int){}, nullptr);
    assert(!error);
    for(auto pair : font_data.glyphs)
    {
        const TTFFontParser::Glyph& glyphData = pair.second;
        Glyph& glyph = glyphs[pair.first];
        glyph.advance = glyphData.advance_width;
        std::memcpy(glyph.boundingBox, glyphData.bounding_box, sizeof(glyphData.bounding_box));
        glyph.center = Vector2(glyphData.glyph_center.x, glyphData.glyph_center.y);
        glyph.index = glyphData.glyph_index;
        glyph.leftSideBearing = glyphData.left_side_bearing;
        glyph.numContours = glyphData.num_contours;
        Array<float> curveTextureData;
        
    }
}