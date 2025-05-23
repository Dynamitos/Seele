#include "SVGLoader.h"
#include "Asset/SVGAsset.h"
#include "Asset/AssetRegistry.h"
#include <fstream>

using namespace Seele;

SVGLoader::SVGLoader(Gfx::PGraphics) {}

SVGLoader::~SVGLoader() {}

void SVGLoader::importAsset(SVGImportArgs args) {
    std::filesystem::path assetPath = args.filePath.filename();
    assetPath.replace_extension("asset");
    OSVGAsset asset = new SVGAsset(args.importPath, assetPath.stem().string());
    asset->setStatus(Asset::Status::Loading);
    // the registry takes ownership, but we need to edit the reference
    PSVGAsset ref = asset;
    AssetRegistry::get().registerSVG(std::move(asset));
    import(args, ref);
}

void SVGLoader::import(SVGImportArgs args, PSVGAsset asset) {
    std::ifstream stream(args.filePath.c_str(), std::ios::binary | std::ios::ate);
    Array<char> svgFile(stream.tellg());
    stream.seekg(0);
    stream.read((char*)svgFile.data(), svgFile.size());
    asset->document = lunasvg::Document::loadFromData(svgFile.data());
    asset->data = std::move(svgFile);
    asset->graphics = graphics;

    AssetRegistry::saveAsset(asset, SVGAsset::IDENTIFIER, asset->getFolderPath(), asset->getName());

    asset->setStatus(Asset::Status::Ready);
}
