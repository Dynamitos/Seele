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

    static void importFile(const std::string& filePath);
    
    static PMeshAsset findMesh(const std::string& filePath);
    static PTextureAsset findTexture(const std::string& filePath);
    static PMaterialAsset findMaterial(const std::string& filePath);
private:
    static AssetRegistry& get();

    AssetRegistry();
    void init(const std::string& rootFolder, Gfx::PGraphics graphics);

    void registerMesh(const std::string& filePath);
    void registerTexture(const std::string& filePath);
    void registerMaterial(const std::string& filePath);
    
    std::string rootFolder;
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