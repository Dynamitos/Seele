#include "Asset/Asset.h"
#include "Asset/MeshAsset.h"
#include "Asset/FontAsset.h"
#include "Asset/TextureAsset.h"
#include "Asset/MaterialAsset.h"
#include "Asset/MaterialInstanceAsset.h"
#include "Graphics/Vulkan/VulkanGraphics.h"
#include "Asset/AssetRegistry.h"

using namespace Seele;

AssetRegistry * instance = new AssetRegistry();

int main(int argc, char** argv)
{
    //if(argc < 2)
    //{
    //    return -1;
    //}
    std::filesystem::path path = "C:\\Users\\Dynamitos\\TrackClear\\Assets\\Dirt.asset";
    std::ifstream stream = std::ifstream(path, std::ios::binary);

    ArchiveBuffer buffer;
    buffer.readFromStream(stream);

    // Read asset type
    uint64 identifier;
    Serialization::load(buffer, identifier);

    // Read name
    std::string name;
    Serialization::load(buffer, name);

    // Read folder
    std::string folderPath;
    Serialization::load(buffer, folderPath);

    PAsset asset;
    switch (identifier)
    {
    case TextureAsset::IDENTIFIER:
        asset = new TextureAsset(folderPath, name);
        break;
    case MeshAsset::IDENTIFIER:
        asset = new MeshAsset(folderPath, name);
        break;
    case MaterialAsset::IDENTIFIER:
        asset = new MaterialAsset(folderPath, name);
        break;
    case MaterialInstanceAsset::IDENTIFIER:
        asset = new MaterialInstanceAsset(folderPath, name);
        // TODO
        break;
    case FontAsset::IDENTIFIER:
        asset = new FontAsset(folderPath, name);
        break;
    default:
        throw new std::logic_error("Unknown Identifier");
    }
    asset->load(buffer);
    std::cin.get();
}