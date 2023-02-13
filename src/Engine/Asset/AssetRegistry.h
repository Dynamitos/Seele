#pragma once
#include "MinimalEngine.h"
#include "Asset.h"
#include "Containers/Map.h"
#include <string>

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
    static void init(const std::string& rootFolder, Gfx::PGraphics graphics);

    static std::filesystem::path getRootFolder();

    static PMeshAsset findMesh(const std::string& filePath);
    static PTextureAsset findTexture(const std::string& filePath);
    static PFontAsset findFont(const std::string& name);
    static PMaterialAsset findMaterial(const std::string& filePath);

    static std::ofstream createWriteStream(const std::string& relativePath, std::ios_base::openmode openmode = std::ios::out);
    static std::ifstream createReadStream(const std::string& relativePath, std::ios_base::openmode openmode = std::ios::in);

    static void importMesh(struct MeshImportArgs args);
    static void importTexture(struct TextureImportArgs args);
    static void importFont(struct FontImportArgs args);
    static void importMaterial(struct MaterialImportArgs args);
    static void set(AssetRegistry registry);

    static void loadRegistry();
    static void saveRegistry();

    AssetRegistry();
private:
    struct AssetFolder
    {
        std::string folderPath;
        Map<std::string, AssetFolder*> children;
        //Todo: Seele::Map doesn't really work with strings for some reason, so just use std::map for now
        Map<std::string, PTextureAsset> textures;
        Map<std::string, PFontAsset> fonts;
        Map<std::string, PMeshAsset> meshes;
        Map<std::string, PMaterialAsset> materials;
        AssetFolder(std::string_view folderPath);
        ~AssetFolder();
    };

    static AssetRegistry& get();

    void initialize(const std::filesystem::path& rootFolder, Gfx::PGraphics graphics);
    void loadRegistryInternal();
    void peekFolder(AssetFolder* folder);
    void loadFolder(AssetFolder* folder);
    void saveRegistryInternal();
    void saveFolder(const std::filesystem::path& folderPath, AssetFolder* folder);
    void saveAsset(PAsset asset, uint64 identifier, const std::filesystem::path& folderPath, std::string name);

    void registerMesh(PMeshAsset mesh);
    void registerTexture(PTextureAsset texture);
    void registerFont(PFontAsset font);
    void registerMaterial(PMaterialAsset material);

    AssetFolder* getOrCreateFolder(std::string foldername);

    std::ofstream internalCreateWriteStream(const std::string& relativePath, std::ios_base::openmode openmode = std::ios::out);
    std::ifstream internalCreateReadStream(const std::string& relaitvePath, std::ios_base::openmode openmode = std::ios::in);

    std::filesystem::path rootFolder;
    AssetFolder* assetRoot;
    Gfx::PGraphics graphics;
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