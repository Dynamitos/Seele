#include "AssetRegistry.h"
#include "MeshAsset.h"
#include "TextureAsset.h"
#include "TextureLoader.h"
#include "MaterialLoader.h"
#include "MeshLoader.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"
#include "graphics/WindowManager.h"

using namespace Seele;

AssetRegistry::~AssetRegistry()
{
}

void AssetRegistry::init(const std::string& rootFolder)
{
    get().init(rootFolder, WindowManager::getGraphics());
}

void AssetRegistry::importFile(const std::string &filePath)
{
    std::string extension = filePath.substr(filePath.find_last_of(".") + 1);
    std::cout << extension << std::endl;
    if (extension.compare("fbx") == 0 
     || extension.compare("obj") == 0)
    {
        get().registerMesh(filePath);
    }
    if (extension.compare("png") == 0
     || extension.compare("jpg") == 0)
    {
        get().registerTexture(filePath);
    }
    if (extension.compare("semat") == 0)
    {
        get().registerMaterial(filePath);
    }
}

PMeshAsset AssetRegistry::findMesh(const std::string &filePath)
{
    return get().meshes[filePath];
}

PMaterialAsset AssetRegistry::findMaterial(const std::string &filePath)
{
    return get().materials[filePath];
}

PTextureAsset AssetRegistry::findTexture(const std::string &filePath)
{
    return get().textures[filePath];
}

AssetRegistry &AssetRegistry::get()
{
    static AssetRegistry instance;
    return instance;
}

AssetRegistry::AssetRegistry()
{
}

void AssetRegistry::init(const std::string &rootFolder, Gfx::PGraphics graphics)
{
    AssetRegistry &reg = get();
    reg.rootFolder = rootFolder;
    reg.meshLoader = new MeshLoader(graphics);
    reg.textureLoader = new TextureLoader(graphics);
    reg.materialLoader = new MaterialLoader(graphics);
}

void AssetRegistry::registerMesh(const std::string &filePath)
{
    meshLoader->importAsset(filePath);
}

void AssetRegistry::registerTexture(const std::string &filePath)
{
    textureLoader->importAsset(filePath);
}

void AssetRegistry::registerMaterial(const std::string &filePath)
{
    materialLoader->queueAsset(filePath);
}