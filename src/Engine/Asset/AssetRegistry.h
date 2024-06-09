#pragma once
#include "Asset.h"
#include "Containers/Map.h"
#include "MinimalEngine.h"
#include <filesystem>
#include <string>


namespace Seele {
DECLARE_REF(TextureAsset)
DECLARE_REF(FontAsset)
DECLARE_REF(MeshAsset)
DECLARE_REF(MaterialAsset)
DECLARE_REF(MaterialInstanceAsset)
DECLARE_NAME_REF(Gfx, Graphics)
class AssetRegistry {
  public:
    ~AssetRegistry();
    static void init(std::filesystem::path path, Gfx::PGraphics graphics);

    static std::filesystem::path getRootFolder();

    static PMeshAsset findMesh(std::string_view folderPath, std::string_view filePath);
    static PTextureAsset findTexture(std::string_view folderPath, std::string_view filePath);
    static PFontAsset findFont(std::string_view folderPath, std::string_view name);
    static PMaterialAsset findMaterial(std::string_view folderPath, std::string_view filePath);
    static PMaterialInstanceAsset findMaterialInstance(std::string_view folderPath, std::string_view filePath);

    static std::ofstream createWriteStream(const std::filesystem::path& relativePath, std::ios_base::openmode openmode = std::ios::out);
    static std::ifstream createReadStream(const std::filesystem::path& relativePath, std::ios_base::openmode openmode = std::ios::in);

    static void set(AssetRegistry registry);

    static void loadRegistry();
    static void saveRegistry();
    struct AssetFolder {
        std::string folderPath;
        Map<std::string, AssetFolder*> children;
        Map<std::string, OTextureAsset> textures;
        Map<std::string, OFontAsset> fonts;
        Map<std::string, OMeshAsset> meshes;
        Map<std::string, OMaterialAsset> materials;
        Map<std::string, OMaterialInstanceAsset> instances;
        AssetFolder(std::string_view folderPath);
        ~AssetFolder();
    };
    AssetFolder* getOrCreateFolder(std::string_view foldername);
    AssetRegistry();
    static AssetRegistry* getInstance();

  private:
    static AssetRegistry& get();

    void initialize(const std::filesystem::path& rootFolder, Gfx::PGraphics graphics);
    void loadRegistryInternal();
    void peekFolder(AssetFolder* folder);
    void loadFolder(AssetFolder* folder);
    void peekAsset(ArchiveBuffer& buffer);
    void loadAsset(ArchiveBuffer& buffer);
    void saveRegistryInternal();
    void saveFolder(const std::filesystem::path& folderPath, AssetFolder* folder);
    void saveAsset(PAsset asset, uint64 identifier, const std::filesystem::path& folderPath, std::string name);

    void registerMesh(OMeshAsset mesh);
    void registerTexture(OTextureAsset texture);
    void registerFont(OFontAsset font);
    void registerMaterial(OMaterialAsset material);
    void registerMaterialInstance(OMaterialInstanceAsset instance);

    std::ofstream internalCreateWriteStream(const std::filesystem::path& relativePath, std::ios_base::openmode openmode = std::ios::out);
    std::ifstream internalCreateReadStream(const std::filesystem::path& relaitvePath, std::ios_base::openmode openmode = std::ios::in);

    std::filesystem::path rootFolder;
    AssetFolder* assetRoot;
    Gfx::PGraphics graphics;
    bool release = false;
    friend class MaterialAsset;
    friend class TextureLoader;
    friend class FontLoader;
    friend class MaterialLoader;
    friend class MeshLoader;
};
} // namespace Seele