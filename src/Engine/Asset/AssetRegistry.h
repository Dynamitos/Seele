#pragma once
#include "MinimalEngine.h"
#include "Asset.h"
#include <string>
#include <map>

namespace Seele
{
DECLARE_REF(TextureLoader)
DECLARE_REF(FontLoader)
DECLARE_REF(MeshLoader)
DECLARE_REF(MaterialLoader)
DECLARE_REF(TextureAsset)
DECLARE_REF(FontAsset)
DECLARE_REF(MeshAsset)
DECLARE_REF(MaterialAsset)
DECLARE_NAME_REF(Gfx, Graphics)
class AssetRegistry
{
public:
    ~AssetRegistry();
    static void init(const std::string& rootFolder);

    static std::string getRootFolder();

    static void importFile(const std::string& filePath);
    static void importFile(const std::string& filePath, const std::string& importPath);
    
    static PMeshAsset findMesh(const std::string& filePath);
    static PTextureAsset findTexture(const std::string& filePath);
    static PFontAsset findFont(const std::string& name);
    static PMaterialAsset findMaterial(const std::string& filePath);

    static std::ofstream createWriteStream(const std::string& relativePath, std::ios_base::openmode openmode = std::ios::out);
    static std::ifstream createReadStream(const std::string& relativePath, std::ios_base::openmode openmode = std::ios::in);
private:
    struct AssetFolder
    {
        std::map<std::string, AssetFolder> children;
        //Todo: Seele::Map doesn't really work with strings for some reason, so just use std::map for now
        std::map<std::string, PTextureAsset> textures;
        std::map<std::string, PFontAsset> fonts;
        std::map<std::string, PMeshAsset> meshes;
        std::map<std::string, PMaterialAsset> materials;
    };

    static AssetRegistry& get();

    AssetRegistry();
    void init(const std::filesystem::path& rootFolder, Gfx::PGraphics graphics);

    void importMesh(const std::filesystem::path& filePath, const std::string& importPath);
    void importTexture(const std::filesystem::path& filePath, const std::string& importPath);
    void importFont(const std::filesystem::path& filePath, const std::string& importPath);
    void importMaterial(const std::filesystem::path& filePath, const std::string& importPath);

    void registerMesh(PMeshAsset mesh, const std::string& importPath);
    void registerTexture(PTextureAsset texture, const std::string& importPath);
    void registerFont(PFontAsset font, const std::string& importPath);
    void registerMaterial(PMaterialAsset material, const std::string& importPath);

    AssetFolder& getOrCreateFolder(std::string foldername);

    std::ofstream internalCreateWriteStream(const std::string& relativePath, std::ios_base::openmode openmode = std::ios::out);
    std::ifstream internalCreateReadStream(const std::string& relaitvePath, std::ios_base::openmode openmode = std::ios::in);

    std::filesystem::path rootFolder;
    AssetFolder assetRoot;
    UPTextureLoader textureLoader;
    UPFontLoader fontLoader;
    UPMeshLoader meshLoader;
    UPMaterialLoader materialLoader;
    friend class TextureLoader;
    friend class FontLoader;
    friend class MaterialLoader;
    friend class MeshLoader;
};
}