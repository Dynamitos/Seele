#include "AssetRegistry.h"
#include "MeshAsset.h"
#include "TextureAsset.h"
#include "TextureLoader.h"
#include "MaterialLoader.h"
#include "MeshLoader.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"
#include "Graphics/WindowManager.h"
#include <iostream>

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
    std::filesystem::path fsPath = std::filesystem::path(filePath);
    std::string extension = fsPath.extension().string();
    std::cout << extension << std::endl;
    if (extension.compare(".fbx") == 0 
     || extension.compare(".obj") == 0)
    {
        get().registerMesh(fsPath);
    }
    if (extension.compare(".png") == 0
     || extension.compare(".jpg") == 0)
    {
        get().registerTexture(fsPath);
    }
    if (extension.compare(".semat") == 0)
    {
        get().registerMaterial(fsPath);
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

void AssetRegistry::init(const std::filesystem::path &rootFolder, Gfx::PGraphics graphics)
{
    AssetRegistry &reg = get();
    reg.rootFolder = rootFolder;
    reg.meshLoader = new MeshLoader(graphics);
    reg.textureLoader = new TextureLoader(graphics);
    reg.materialLoader = new MaterialLoader(graphics);
}

void AssetRegistry::registerMesh(const std::filesystem::path &filePath)
{
    meshLoader->importAsset(filePath);
}

void AssetRegistry::registerTexture(const std::filesystem::path &filePath)
{
    textureLoader->importAsset(filePath);
}

void AssetRegistry::registerMaterial(const std::filesystem::path &filePath)
{
    materialLoader->queueAsset(filePath);
}