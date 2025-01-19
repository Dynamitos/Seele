#pragma once
#include "Asset.h"
#include "Containers/Map.h"
#include "Graphics/Texture.h"
#include "Math/Math.h"
#include <lunasvg.h>

namespace Seele {
class SVGAsset : public Asset {
  public:
    static constexpr uint64 IDENTIFIER = 0x20;
    SVGAsset();
    SVGAsset(std::string_view folderPath, std::string_view name);
    virtual ~SVGAsset();
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;

    Gfx::PTexture2D getTexture(UVector2 viewbox);

  private:
      // workaround to make vector a map key, could be solved by either implementing 
      // comparison operators for vectors, or implementing a hash map
    struct ViewBox {
        uint32 width;
        uint32 height;
        constexpr std::strong_ordering operator<=>(const ViewBox& other) const = default;
    };
    Array<char> data;
    Gfx::PGraphics graphics;
    std::unique_ptr<lunasvg::Document> document;
    Map<ViewBox, Gfx::OTexture2D> cachedTextures;
    friend class SVGLoader;
};
} // namespace Seele