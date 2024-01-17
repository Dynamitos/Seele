#include "AssetRegistry.h"
#include "MeshAsset.h"
#include "FontAsset.h"
#include "TextureAsset.h"
#include "MaterialAsset.h"
#include "MaterialInstanceAsset.h"
#include "Graphics/Mesh.h"
#include "Graphics/Graphics.h"
#include "Window/WindowManager.h"
#include "MeshAsset.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

using namespace Seele;

AssetRegistry* _instance = new AssetRegistry();

AssetRegistry::~AssetRegistry()
{
    delete assetRoot;
}

void AssetRegistry::init(std::filesystem::path rootFolder, Gfx::PGraphics graphics)
{
    get().initialize(rootFolder, graphics);
}

PMeshAsset AssetRegistry::findMesh(const std::string &filePath)
{
    AssetFolder* folder = get().assetRoot;
    std::string fileName = filePath;
    size_t slashLoc = filePath.rfind("/");
    if(slashLoc != std::string::npos)
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
    if (slashLoc != std::string::npos)
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
    if(slashLoc != std::string::npos)
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
    if (slashLoc != std::string::npos)
    {
        folder = get().getOrCreateFolder(filePath.substr(0, slashLoc));
        fileName = filePath.substr(slashLoc + 1, filePath.size());
    }
    return folder->materials.at(fileName);
}

PMaterialInstanceAsset AssetRegistry::findMaterialInstance(const std::string &filePath)
{
    AssetFolder* folder = get().assetRoot;
    std::string fileName = filePath;
    size_t slashLoc = filePath.rfind("/");
    if (slashLoc != std::string::npos)
    {
        folder = get().getOrCreateFolder(filePath.substr(0, slashLoc));
        fileName = filePath.substr(slashLoc + 1, filePath.size());
    }
    return folder->instances.at(fileName);
}

std::ofstream AssetRegistry::createWriteStream(const std::filesystem::path& relativePath, std::ios_base::openmode openmode) 
{
    return get().internalCreateWriteStream(relativePath, openmode);
}

std::ifstream AssetRegistry::createReadStream(const std::filesystem::path& relativePath, std::ios_base::openmode openmode)
{
    return get().internalCreateReadStream(relativePath, openmode);
}

AssetRegistry &AssetRegistry::get()
{
    return *_instance;
}

AssetRegistry::AssetRegistry()
    : assetRoot(nullptr)
{
}

AssetRegistry* AssetRegistry::getInstance()
{
    return _instance;
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
        if(entry.path().filename().compare(".DS_Store") == 0)
        {
            continue;
        }
        auto stream = std::ifstream(entry.path(), std::ios::binary);

        ArchiveBuffer buffer(graphics);
        buffer.readFromStream(stream);
        peekAsset(buffer);
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

        if(entry.path().filename().compare(".DS_Store") == 0)
        {
            continue;
        }
        auto stream = std::ifstream(path, std::ios::binary);

        ArchiveBuffer buffer(graphics);
        buffer.readFromStream(stream);
        loadAsset(buffer);
    }
}

void AssetRegistry::peekAsset(ArchiveBuffer& buffer)
{
    // Read asset type
    uint64 identifier;
    Serialization::load(buffer, identifier);

    // Read name
    std::string name;
    Serialization::load(buffer, name);

    // Read folder
    std::string folderPath;
    Serialization::load(buffer, folderPath);

    uint64 assetSize;
    Serialization::load(buffer, assetSize);

    AssetFolder* folder = getOrCreateFolder(folderPath);

    OAsset asset;
    switch (identifier)
    {
    case TextureAsset::IDENTIFIER:
        asset = new TextureAsset(folderPath, name);
        folder->textures[name] = std::move(asset);
        break;
    case MeshAsset::IDENTIFIER:
        asset = new MeshAsset(folderPath, name);
        folder->meshes[name] = std::move(asset);
        break;
    case MaterialAsset::IDENTIFIER:
        asset = new MaterialAsset(folderPath, name);
        folder->materials[name] = std::move(asset);
        break;
    case MaterialInstanceAsset::IDENTIFIER:
        asset = new MaterialInstanceAsset(folderPath, name);
        folder->instances[name] = std::move(asset);
        break;
    case FontAsset::IDENTIFIER:
        asset = new FontAsset(folderPath, name);
        folder->fonts[name] = std::move(asset);
        break;
    default:
        throw new std::logic_error("Unknown Identifier");
    }
}

void AssetRegistry::loadAsset(ArchiveBuffer& buffer)
{
    // Read asset type
    uint64 identifier;
    Serialization::load(buffer, identifier);

    // Read name
    std::string name;
    Serialization::load(buffer, name);

    // Read folder
    std::string folderPath;
    Serialization::load(buffer, folderPath);

    AssetFolder* folder = getOrCreateFolder(folderPath);

    PAsset asset;
    switch (identifier)
    {
    case TextureAsset::IDENTIFIER:
        asset = PTextureAsset(folder->textures.at(name));
        break;
    case MeshAsset::IDENTIFIER:
        asset = PMeshAsset(folder->meshes.at(name));
        break;
    case MaterialAsset::IDENTIFIER:
        asset = PMaterialAsset(folder->materials.at(name));
        break;
    case MaterialInstanceAsset::IDENTIFIER:
        asset = PMaterialInstanceAsset(folder->instances.at(name));
        break;
    case FontAsset::IDENTIFIER:
        asset = PFontAsset(folder->fonts.at(name));
        break;
    default:
        throw new std::logic_error("Unknown Identifier");
    }
    asset->load(buffer);
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
        saveAsset(PTextureAsset(texture), TextureAsset::IDENTIFIER, folderPath, name);
    }
    for (const auto& [name, mesh] : folder->meshes)
    {
        saveAsset(PMeshAsset(mesh), MeshAsset::IDENTIFIER, folderPath, name);
    }
    for (const auto& [name, material] : folder->materials)
    {
        saveAsset(PMaterialAsset(material), MaterialAsset::IDENTIFIER, folderPath, name);
    }
    for (const auto& [name, material] : folder->instances)
    {
        saveAsset(PMaterialInstanceAsset(material), MaterialInstanceAsset::IDENTIFIER, folderPath, name);
    }
    for (const auto& [name, font] : folder->fonts)
    {
        saveAsset(PFontAsset(font), FontAsset::IDENTIFIER, folderPath, name);
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

    std::string path = (folderPath / name).replace_extension("asset").string();
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

void AssetRegistry::registerMesh(OMeshAsset mesh) 
{
    AssetFolder* folder = getOrCreateFolder(mesh->getFolderPath());
    folder->meshes[mesh->getName()] = std::move(mesh);
}

void AssetRegistry::registerTexture(OTextureAsset texture) 
{
    AssetFolder* folder = getOrCreateFolder(texture->getFolderPath());
    folder->textures[texture->getName()] = std::move(texture);
}

void AssetRegistry::registerFont(OFontAsset font)
{
    AssetFolder* folder = getOrCreateFolder(font->getFolderPath());
    folder->fonts[font->getName()] = std::move(font);
}

void AssetRegistry::registerMaterial(OMaterialAsset material)
{
    AssetFolder* folder = getOrCreateFolder(material->getFolderPath());
    folder->materials[material->getName()] = std::move(material);
}

void AssetRegistry::registerMaterialInstance(OMaterialInstanceAsset material)
{
    AssetFolder* folder = getOrCreateFolder(material->getFolderPath());
    folder->instances[material->getName()] = std::move(material);
}

AssetRegistry::AssetFolder* AssetRegistry::getOrCreateFolder(std::string fullPath)
{
    AssetFolder* result = assetRoot;
    std::string temp = fullPath;
    while(!fullPath.empty())
    {
        size_t slashLoc = fullPath.find("/");
        if(slashLoc == std::string::npos)
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

std::ofstream AssetRegistry::internalCreateWriteStream(const std::filesystem::path& relativePath, std::ios_base::openmode openmode)
{
    auto fullPath = rootFolder / relativePath;
    std::filesystem::create_directories(fullPath.parent_path());
    return std::ofstream(fullPath.string(), openmode);
}

std::ifstream AssetRegistry::internalCreateReadStream(const std::filesystem::path& relativePath, std::ios_base::openmode openmode)
{
    auto fullPath = rootFolder / relativePath;
    std::filesystem::create_directories(fullPath.parent_path());
    return std::ifstream(fullPath.string(), openmode);
}

AssetRegistry::AssetFolder::AssetFolder(std::string_view folderPath)
    : folderPath(folderPath)
{}

AssetRegistry::AssetFolder::~AssetFolder()
{
    for (auto [_, child] : children)
    {
        delete child;
    }
}

