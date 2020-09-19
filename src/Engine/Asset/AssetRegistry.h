#pragma once
#include "MinimalEngine.h"
#include "Asset.h"
#include <string>

namespace Seele
{
DECLARE_REF(TextureLoader);
DECLARE_REF(MeshLoader);
DECLARE_REF(MaterialLoader);
DECLARE_REF(TextureAsset);
DECLARE_REF(MeshAsset);
DECLARE_REF(MaterialAsset);
DECLARE_NAME_REF(Gfx, Graphics);
class AssetRegistry
{
public:
    ~AssetRegistry();
    static void init(const std::string& rootFolder);

    static std::string getRootFolder();

    static void importFile(const std::string& filePath);
    
    static PMeshAsset findMesh(const std::string& filePath);
    static PTextureAsset findTexture(const std::string& filePath);
    static PMaterialAsset findMaterial(const std::string& filePath);

    static std::ofstream createWriteStream(const std::string& relativePath, std::ios_base::openmode openmode = 0);
    static std::ifstream createReadStream(const std::string& relativePath, std::ios_base::openmode openmode = 0);
private:
    static AssetRegistry& get();

    AssetRegistry();
    void init(const std::filesystem::path& rootFolder, Gfx::PGraphics graphics);

    void importMesh(const std::filesystem::path& filePath);
    void importTexture(const std::filesystem::path& filePath);
    void importMaterial(const std::filesystem::path& filePath);

    void registerMesh(PMeshAsset mesh);
    void registerTexture(PTextureAsset texture);
    void registerMaterial(PMaterialAsset material);

    std::ofstream internalCreateWriteStream(const std::string& relativePath, std::ios_base::openmode openmode = 0);
    std::ifstream internalCreateReadStream(const std::string& relaitvePath, std::ios_base::openmode openmode = 0);

    std::filesystem::path rootFolder;
    Map<std::string, PTextureAsset> textures;
    Map<std::string, PMeshAsset> meshes;
    Map<std::string, PMaterialAsset> materials;
    UPTextureLoader textureLoader;
    UPMeshLoader meshLoader;
    UPMaterialLoader materialLoader;
    friend class TextureLoader;
    friend class MaterialLoader;
    friend class MeshLoader;
};
}