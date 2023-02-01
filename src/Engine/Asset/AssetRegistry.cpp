#include "AssetRegistry.h"
#include "MeshAsset.h"
#include "FontAsset.h"
#include "TextureAsset.h"
#include "MaterialAsset.h"
#include "FontLoader.h"
#include "TextureLoader.h"
#include "MaterialLoader.h"
#include "MeshLoader.h"
#include "Graphics/Mesh.h"
#include "Graphics/Graphics.h"
#include "Window/WindowManager.h"
#include "MeshAsset.h"
#include <nlohmann/json.hpp>
#include <iostream>

using namespace Seele;
using json = nlohmann::json;

extern AssetRegistry* instance;

AssetRegistry::~AssetRegistry()
{
}

void AssetRegistry::init(const std::string& rootFolder, Gfx::PGraphics graphics)
{
    get().initialize(rootFolder, graphics);
}

void AssetRegistry::importMesh(MeshImportArgs args)
{
    get().meshLoader->importAsset(args);
}

void AssetRegistry::importTexture(TextureImportArgs args)
{
    get().textureLoader->importAsset(args);
}

void AssetRegistry::importFont(FontImportArgs args)
{
    get().fontLoader->importAsset(args);
}

void AssetRegistry::importMaterial(MaterialImportArgs args)
{
    get().materialLoader->importAsset(args);
}


PMeshAsset AssetRegistry::findMesh(const std::string &filePath)
{
    AssetFolder& folder = get().assetRoot;
    std::string fileName = filePath;
    size_t slashLoc = filePath.rfind("/");
    if(slashLoc != -1)
    {
        folder = get().getOrCreateFolder(filePath.substr(0, slashLoc));
        fileName = filePath.substr(slashLoc+1, filePath.size());
    }
    auto it = folder.meshes.find(fileName);
    assert(it != folder.meshes.end());
    return it->second;
}

PTextureAsset AssetRegistry::findTexture(const std::string &filePath)
{
    std::string fileName = filePath;
    size_t slashLoc = filePath.rfind("/");
    if(slashLoc != -1)
    {
        AssetFolder& folder = get().getOrCreateFolder(filePath.substr(0, slashLoc));
        fileName = filePath.substr(slashLoc+1, filePath.size());
        return folder.textures[fileName];
    }
    else
    {
        return get().assetRoot.textures[fileName];
    }
}

PFontAsset AssetRegistry::findFont(const std::string& filePath)
{
    AssetFolder& folder = get().assetRoot;
    std::string fileName = filePath;
    size_t slashLoc = filePath.rfind("/");
    if(slashLoc != -1)
    {
        folder = get().getOrCreateFolder(filePath.substr(0, slashLoc));
        fileName = filePath.substr(slashLoc+1, filePath.size());
    }
    return folder.fonts[fileName];
}

PMaterialAsset AssetRegistry::findMaterial(const std::string &filePath)
{
    AssetFolder& folder = get().assetRoot;
    std::string fileName = filePath;
    size_t slashLoc = filePath.rfind("/");
    if(slashLoc != -1)
    {
        folder = get().getOrCreateFolder(filePath.substr(0, slashLoc));
        fileName = filePath.substr(slashLoc+1, filePath.size());
    }
    return folder.materials[fileName];
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
    return *instance;
}

AssetRegistry::AssetRegistry()
{
}

void AssetRegistry::initialize(const std::filesystem::path &_rootFolder, Gfx::PGraphics _graphics)
{
    this->graphics = _graphics;
    this->rootFolder = _rootFolder;
    this->fontLoader = new FontLoader(graphics);
    this->meshLoader = new MeshLoader(graphics);
    this->textureLoader = new TextureLoader(graphics);
    this->materialLoader = new MaterialLoader(graphics);
}

std::string AssetRegistry::getRootFolder()
{
    return get().rootFolder.generic_string();
}

void AssetRegistry::registerMesh(PMeshAsset mesh, const std::string& importPath) 
{
    AssetFolder& folder = getOrCreateFolder(importPath);
    folder.meshes[mesh->getFileName()] = mesh;
}

void AssetRegistry::registerTexture(PTextureAsset texture, const std::string& importPath) 
{
    AssetFolder& folder = getOrCreateFolder(importPath);
    folder.textures[texture->getFileName()] = texture;
}

void AssetRegistry::registerFont(PFontAsset font, const std::string& importPath)
{
    AssetFolder& folder = getOrCreateFolder(importPath);
    folder.fonts[font->getFileName()] = font;
}

void AssetRegistry::registerMaterial(PMaterialAsset material, const std::string& importPath)
{
    AssetFolder& folder = getOrCreateFolder(importPath);
    folder.materials[material->getFileName()] = material;
}

AssetRegistry::AssetFolder& AssetRegistry::getOrCreateFolder(std::string fullPath)
{
    AssetFolder& result = assetRoot;
    while(!fullPath.empty())
    {
        size_t slashLoc = fullPath.find("/");
        if(slashLoc == -1)
        {
            return result.children[fullPath];
        }
        std::string folderName = fullPath.substr(0, slashLoc);
        result = result.children[folderName];
        fullPath = fullPath.substr(slashLoc+1, fullPath.size());
    }
    return result;
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
