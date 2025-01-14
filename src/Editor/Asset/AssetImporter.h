#pragma once
#include "Asset/AssetRegistry.h"
#include "MinimalEngine.h"


namespace Seele {
DECLARE_REF(TextureLoader)
DECLARE_REF(FontLoader)
DECLARE_REF(MeshLoader)
DECLARE_REF(MaterialLoader)
class AssetImporter {
  public:
    static void importMesh(struct MeshImportArgs args);
    static void importTexture(struct TextureImportArgs args);
    static void importFont(struct FontImportArgs args);
    static void importMaterial(struct MaterialImportArgs args);
    static void init(Gfx::PGraphics graphics);

  private:
    static AssetImporter& get();
    UPTextureLoader textureLoader;
    UPFontLoader fontLoader;
    UPMeshLoader meshLoader;
    UPMaterialLoader materialLoader;
    AssetRegistry* registry;
};
} // namespace Seele