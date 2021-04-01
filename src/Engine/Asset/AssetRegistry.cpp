#include "AssetRegistry.h"
#include "MeshAsset.h"
#include "TextureAsset.h"
#include "TextureLoader.h"
#include "MaterialLoader.h"
#include "MeshLoader.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"
#include "Window/WindowManager.h"
#include "MeshAsset.h"
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
    if (extension.compare(".fbx") == 0 
     || extension.compare(".obj") == 0)
    {
        get().importMesh(fsPath);
    }
    if (extension.compare(".png") == 0
     || extension.compare(".jpg") == 0)
    {
        get().importTexture(fsPath);
    }
    if (extension.compare(".semat") == 0)
    {
        get().importMaterial(fsPath);
    }
}

PMeshAsset AssetRegistry::findMesh(const std::string &filePath)
{
    return get().meshes[filePath];
}

PTextureAsset AssetRegistry::findTexture(const std::string &filePath)
{
    PTextureAsset result = get().textures[filePath];
    if(result == nullptr)
    {
        return get().textureLoader->getPlaceholderTexture();
    }
    return result;
}

PMaterialAsset AssetRegistry::findMaterial(const std::string &filePath)
{
    return get().materials[filePath];
}

std::ofstream AssetRegistry::createWriteStream(const std::string& relativePath, std::ios_base::openmode openmode) 
{
    return get().internalCreateWriteStream(relativePath, openmode);
}

std::ifstream AssetRegistry::createReadStream(const std::string& relativePath, std::ios_base::openmode openmode) 
{
    return get().internalCreateReadStream(relativePath, openmode);
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

std::string AssetRegistry::getRootFolder()
{
    return get().rootFolder.generic_string();
}

void AssetRegistry::importMesh(const std::filesystem::path &filePath)
{
    meshLoader->importAsset(filePath);
}

void AssetRegistry::importTexture(const std::filesystem::path &filePath)
{
    textureLoader->importAsset(filePath);
}

void AssetRegistry::importMaterial(const std::filesystem::path &filePath)
{
    materialLoader->queueAsset(filePath);
}

void AssetRegistry::registerMesh(PMeshAsset mesh) 
{
    PMeshAsset existingMesh = meshes[mesh->getFileName()];
    if(existingMesh != nullptr)
    {
        auto newMeshes = mesh->getMeshes();
        for(uint32 i = 0; i < newMeshes.size(); ++i)
        {
            existingMesh->addMesh(newMeshes[i]);
        }
    }
    else
    {
        meshes[mesh->getFileName()] = mesh;
    }
}

void AssetRegistry::registerMaterial(PMaterialAsset material)
{
    materials[material->getFileName()] = material;
}

std::ofstream AssetRegistry::internalCreateWriteStream(const std::string& relativePath, std::ios_base::openmode openmode) 
{
    auto fullPath = rootFolder;
    fullPath.append(relativePath);
    return std::ofstream(fullPath.string(), openmode);
}

std::ifstream AssetRegistry::internalCreateReadStream(const std::string& relativePath, std::ios_base::openmode openmode)
{
    auto fullPath = rootFolder;
    fullPath.append(relativePath);
    return std::ifstream(fullPath.string(), openmode);
}
