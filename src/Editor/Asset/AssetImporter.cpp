#include "AssetImporter.h"
#include "FontLoader.h"
#include "Graphics/Graphics.h"
#include "MaterialLoader.h"
#include "MeshLoader.h"
#include "TextureLoader.h"
#include "EnvironmentLoader.h"

using namespace Seele;

void AssetImporter::importMesh(MeshImportArgs args) {
    if (get().registry->getOrCreateFolder(args.importPath)->meshes.contains(args.filePath.stem().string())) {
        // skip importing duplicates
        return;
    }
    get().meshLoader->importAsset(args);
}

void AssetImporter::importTexture(TextureImportArgs args) {
    if (get().registry->getOrCreateFolder(args.importPath)->textures.contains(args.filePath.stem().string())) {
        // skip importing duplicates
        return;
    }
    get().textureLoader->importAsset(args);
}

void AssetImporter::importFont(FontImportArgs args) {
    if (get().registry->getOrCreateFolder(args.importPath)->fonts.contains(args.filePath.stem().string())) {
        // skip importing duplicates
        return;
    }
    get().fontLoader->importAsset(args);
}

void AssetImporter::importMaterial(MaterialImportArgs args) {
    if (get().registry->getOrCreateFolder(args.importPath)->materials.contains(args.filePath.stem().string())) {
        // skip importing duplicates
        return;
    }
    get().materialLoader->importAsset(args);
}

void Seele::AssetImporter::importEnvironmentMap(EnvironmentImportArgs args) {
    if (get().registry->getOrCreateFolder(args.importPath)->materials.contains(args.filePath.stem().string())) {
        // skip importing duplicates
        return;
    }
    get().environmentLoader->importAsset(args);
}

void AssetImporter::init(Gfx::PGraphics graphics) {
    get().registry = AssetRegistry::getInstance();
    get().meshLoader = new MeshLoader(graphics);
    get().textureLoader = new TextureLoader(graphics);
    get().materialLoader = new MaterialLoader(graphics);
    get().fontLoader = new FontLoader(graphics);
    get().environmentLoader = new EnvironmentLoader(graphics);
}

AssetImporter& AssetImporter::get() {
    static AssetImporter instance;
    return instance;
}
