#include "AssetRegistry.h"
#include "MeshAsset.h"
#include "FontAsset.h"
#include "TextureAsset.h"
#include "MaterialAsset.h"
#include "MaterialInstanceAsset.h"
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

extern AssetRegistry* instance;

AssetRegistry::~AssetRegistry()
{
    delete assetRoot;
}

void AssetRegistry::init(const std::string& rootFolder, Gfx::PGraphics graphics)
{
    get().initialize(rootFolder, graphics);
}

void AssetRegistry::importMesh(MeshImportArgs args)
{
    if (get().getOrCreateFolder(args.importPath)->meshes.contains(args.filePath.stem().string()))
    {
        // skip importing duplicates
        return;
    }
    get().meshLoader->importAsset(args);
}

void AssetRegistry::importTexture(TextureImportArgs args)
{
    if (get().getOrCreateFolder(args.importPath)->textures.contains(args.filePath.stem().string()))
    {
        // skip importing duplicates
        return;
    }
    get().textureLoader->importAsset(args);
}

void AssetRegistry::importFont(FontImportArgs args)
{
    if (get().getOrCreateFolder(args.importPath)->fonts.contains(args.filePath.stem().string()))
    {
        // skip importing duplicates
        return;
    }
    get().fontLoader->importAsset(args);
}

void AssetRegistry::importMaterial(MaterialImportArgs args)
{
    if (get().getOrCreateFolder(args.importPath)->materials.contains(args.filePath.stem().string()))
    {
        // skip importing duplicates
        return;
    }
    get().materialLoader->importAsset(args);
}

PMeshAsset AssetRegistry::findMesh(const std::string &filePath)
{
    AssetFolder* folder = get().assetRoot;
    std::string fileName = filePath;
    size_t slashLoc = filePath.rfind("/");
    if(slashLoc != -1)
    {
        folder = get().getOrCreateFolder(filePath.substr(0, slashLoc));
        fileName = filePath.substr(slashLoc+1, filePath.size());
    }
    return folder->meshes.at(fileName);
}

PTextureAsset AssetRegistry::findTexture(const std::string &filePath)
{
    AssetFolder* folder = get().assetRoot;
    std::string fileName = filePath;
    size_t slashLoc = filePath.rfind("/");
    if (slashLoc != -1)
    {
        folder = get().getOrCreateFolder(filePath.substr(0, slashLoc));
        fileName = filePath.substr(slashLoc + 1, filePath.size());
    }
    return folder->textures.at(fileName);
}

PFontAsset AssetRegistry::findFont(const std::string& filePath)
{
    AssetFolder* folder = get().assetRoot;
    std::string fileName = filePath;
    size_t slashLoc = filePath.rfind("/");
    if(slashLoc != -1)
    {
        folder = get().getOrCreateFolder(filePath.substr(0, slashLoc));
        fileName = filePath.substr(slashLoc+1, filePath.size());
    }
    return folder->fonts.at(fileName);
}

PMaterialAsset AssetRegistry::findMaterial(const std::string &filePath)
{
    AssetFolder* folder = get().assetRoot;
    std::string fileName = filePath;
    size_t slashLoc = filePath.rfind("/");
    if (slashLoc != -1)
    {
        folder = get().getOrCreateFolder(filePath.substr(0, slashLoc));
        fileName = filePath.substr(slashLoc + 1, filePath.size());
    }
    return folder->materials.at(fileName);
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

void AssetRegistry::loadRegistry()
{
    get().loadRegistryInternal();
}

void AssetRegistry::saveRegistry()
{
    get().saveRegistryInternal();
}

void AssetRegistry::initialize(const std::filesystem::path &_rootFolder, Gfx::PGraphics _graphics)
{
    this->graphics = _graphics;
    this->rootFolder = _rootFolder;
    this->assetRoot = new AssetFolder("");
    this->fontLoader = new FontLoader(graphics);
    this->meshLoader = new MeshLoader(graphics);
    this->textureLoader = new TextureLoader(graphics);
    this->materialLoader = new MaterialLoader(graphics);
    loadRegistryInternal();
}

void AssetRegistry::loadRegistryInternal()
{
    peekFolder(assetRoot);
    loadFolder(assetRoot);
}


void AssetRegistry::peekFolder(AssetFolder* folder)
{
    for (const auto& entry : std::filesystem::directory_iterator(rootFolder / folder->folderPath))
    {
        const auto& stem = entry.path().stem().string();
        if (entry.is_directory())
        {
            if (folder->folderPath.empty())
            {
                folder->children[stem] = new AssetFolder(stem);
            }
            else
            {
                folder->children[stem] = new AssetFolder(folder->folderPath + "/" + stem);
            }
            peekFolder(folder->children[stem]);
            continue;
        }

        auto stream = std::ifstream(entry.path(), std::ios::binary);

        ArchiveBuffer buffer(graphics);
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
            folder->textures[asset->getName()] = asset;
            break;
        case MeshAsset::IDENTIFIER:
            asset = new MeshAsset(folderPath, name);
            folder->meshes[asset->getName()] = asset;
            break;
        case MaterialAsset::IDENTIFIER:
            asset = new MaterialAsset(folderPath, name);
            folder->materials[asset->getName()] = asset;
            break;
        case MaterialInstanceAsset::IDENTIFIER:
            asset = new MaterialInstanceAsset(folderPath, name);
            // TODO
            break;
        case FontAsset::IDENTIFIER:
            asset = new FontAsset(folderPath, name);
            folder->fonts[asset->getName()] = asset;
            break;
        default:
            throw new std::logic_error("Unknown Identifier");
        }
    }
}


void AssetRegistry::loadFolder(AssetFolder* folder)
{
    for (const auto& entry : std::filesystem::directory_iterator(rootFolder / folder->folderPath))
    {
        const auto& path = entry.path();
        if (entry.is_directory())
        {
            auto relative = path.stem().string();
            loadFolder(folder->children[relative]);
            continue;
        }

        auto stream = std::ifstream(path, std::ios::binary);

        ArchiveBuffer buffer(graphics);
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
            asset = folder->textures.at(name);
            break;
        case MeshAsset::IDENTIFIER:
            asset = folder->meshes.at(name);
            break;
        case MaterialAsset::IDENTIFIER:
            asset = folder->materials.at(name);
            break;
        case MaterialInstanceAsset::IDENTIFIER:
            //asset = new MaterialInstanceAsset(path);
            // TODO
            break;
        case FontAsset::IDENTIFIER:
            asset = folder->fonts.at(name);
            break;
        default:
            throw new std::logic_error("Unknown Identifier");
        }
        asset->load(buffer);
    }
}

void AssetRegistry::saveRegistryInternal()
{   
    saveFolder("", assetRoot);
}

void AssetRegistry::saveFolder(const std::filesystem::path& folderPath, AssetFolder* folder)
{
    std::filesystem::create_directory(rootFolder / folderPath);
    for (const auto& [name, texture] : folder->textures)
    {
        saveAsset(texture, TextureAsset::IDENTIFIER, folderPath, name);
    }
    for (const auto& [name, mesh] : folder->meshes)
    {
        saveAsset(mesh, MeshAsset::IDENTIFIER, folderPath, name);
    }
    for (const auto& [name, material] : folder->materials)
    {
        saveAsset(material, MaterialAsset::IDENTIFIER, folderPath, name);
    }
    for (const auto& [name, font] : folder->fonts)
    {
        saveAsset(font, FontAsset::IDENTIFIER, folderPath, name);
    }
    for (auto& [name, child] : folder->children)
    {
        saveFolder(folderPath / name, child);
    }
}

void AssetRegistry::saveAsset(PAsset asset, uint64 identifier, const std::filesystem::path& folderPath, std::string name)
{
    if (name.empty())
        return;

    std::string path = (folderPath / name).string().append(".asset");
    auto assetStream = createWriteStream(std::move(path), std::ios::binary);
    ArchiveBuffer assetBuffer(graphics);
    // write identifier
    Serialization::save(assetBuffer, identifier);
    // write name
    Serialization::save(assetBuffer, name);
    // write folder
    Serialization::save(assetBuffer, folderPath.string());
    // write asset data
    asset->save(assetBuffer);
    assetBuffer.writeToStream(assetStream);

}

std::filesystem::path AssetRegistry::getRootFolder()
{
    return get().rootFolder;
}

void AssetRegistry::registerMesh(PMeshAsset mesh) 
{
    AssetFolder* folder = getOrCreateFolder(mesh->getFolderPath());
    folder->meshes[mesh->getName()] = mesh;
}

void AssetRegistry::registerTexture(PTextureAsset texture) 
{
    AssetFolder* folder = getOrCreateFolder(texture->getFolderPath());
    folder->textures[texture->getName()] = texture;
}

void AssetRegistry::registerFont(PFontAsset font)
{
    AssetFolder* folder = getOrCreateFolder(font->getFolderPath());
    folder->fonts[font->getName()] = font;
}

void AssetRegistry::registerMaterial(PMaterialAsset material)
{
    AssetFolder* folder = getOrCreateFolder(material->getFolderPath());
    folder->materials[material->getName()] = material;
}

AssetRegistry::AssetFolder* AssetRegistry::getOrCreateFolder(std::string fullPath)
{
    AssetFolder* result = assetRoot;
    std::string temp = fullPath;
    while(!fullPath.empty())
    {
        size_t slashLoc = fullPath.find("/");
        if(slashLoc == -1)
        {
            if (!result->children.contains(fullPath))
            {
                result->children[fullPath] = new AssetFolder(fullPath);
            }
            return result->children[fullPath];
        }
        std::string folderName = fullPath.substr(0, slashLoc);
        if (!result->children.contains(folderName))
        {
            result->children[folderName] = new AssetFolder(temp);
        }
        result = result->children[folderName];
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

AssetRegistry::AssetFolder::AssetFolder(std::string_view folderPath)
    : folderPath(folderPath)
{}

AssetRegistry::AssetFolder::~AssetFolder()
{
    for (const auto& [_, child] : children)
    {
        delete child;
    }
}
