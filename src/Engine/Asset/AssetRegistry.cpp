#include "AssetRegistry.h"
#include "FontAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Mesh.h"
#include "MaterialAsset.h"
#include "MaterialInstanceAsset.h"
#include "MeshAsset.h"
#include "TextureAsset.h"
#include "Window/WindowManager.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace Seele;

AssetRegistry* _instance = new AssetRegistry();

AssetRegistry::~AssetRegistry() { delete assetRoot; }

void AssetRegistry::init(std::filesystem::path rootFolder, Gfx::PGraphics graphics) { get().initialize(rootFolder, graphics); }

PMeshAsset AssetRegistry::findMesh(std::string_view folderPath, std::string_view filePath) {
    std::unique_lock l(get().assetLock);
    AssetFolder* folder = get().assetRoot;
    if (!folderPath.empty()) {
        folder = get().getOrCreateFolder(folderPath);
    }
    return folder->meshes.at(std::string(filePath));
}

PTextureAsset AssetRegistry::findTexture(std::string_view folderPath, std::string_view filePath) {
    std::unique_lock l(get().assetLock);
    AssetFolder* folder = get().assetRoot;
    if (!folderPath.empty()) {
        folder = get().getOrCreateFolder(folderPath);
    }
    return folder->textures.at(std::string(filePath));
}

PFontAsset AssetRegistry::findFont(std::string_view folderPath, std::string_view filePath) {
    std::unique_lock l(get().assetLock);
    AssetFolder* folder = get().assetRoot;
    if (!folderPath.empty()) {
        folder = get().getOrCreateFolder(folderPath);
    }
    return folder->fonts.at(std::string(filePath));
}

PMaterialAsset AssetRegistry::findMaterial(std::string_view folderPath, std::string_view filePath) {
    std::unique_lock l(get().assetLock);
    AssetFolder* folder = get().assetRoot;
    if (!folderPath.empty()) {
        folder = get().getOrCreateFolder(folderPath);
    }
    return folder->materials.at(std::string(filePath));
}

PMaterialInstanceAsset AssetRegistry::findMaterialInstance(std::string_view folderPath, std::string_view filePath) {
    std::unique_lock l(get().assetLock);
    AssetFolder* folder = get().assetRoot;
    if (!folderPath.empty()) {
        folder = get().getOrCreateFolder(folderPath);
    }
    return folder->instances.at(std::string(filePath));
}

void AssetRegistry::registerMesh(OMeshAsset mesh) {
    std::unique_lock l(get().assetLock);
    get().registerMeshInternal(std::move(mesh));
}

void AssetRegistry::registerTexture(OTextureAsset texture) {
    std::unique_lock l(get().assetLock);
    get().registerTextureInternal(std::move(texture));
}

void AssetRegistry::registerFont(OFontAsset font) {
    std::unique_lock l(get().assetLock);
    get().registerFontInternal(std::move(font));
}

void AssetRegistry::registerMaterial(OMaterialAsset material) {
    std::unique_lock l(get().assetLock);
    get().registerMaterialInternal(std::move(material));
}

void AssetRegistry::registerMaterialInstance(OMaterialInstanceAsset instance) {
    std::unique_lock l(get().assetLock);
    get().registerMaterialInstanceInternal(std::move(instance));
}

std::ofstream AssetRegistry::createWriteStream(const std::filesystem::path& relativePath, std::ios_base::openmode openmode) {
    return get().internalCreateWriteStream(relativePath, openmode);
}

std::ifstream AssetRegistry::createReadStream(const std::filesystem::path& relativePath, std::ios_base::openmode openmode) {
    return get().internalCreateReadStream(relativePath, openmode);
}

void AssetRegistry::loadAsset(ArchiveBuffer& buffer) {
    // Read asset type
    uint64 identifier;
    Serialization::load(buffer, identifier);

    // Read name
    std::string name;
    Serialization::load(buffer, name);

    // Read folder
    std::string folderPath;
    Serialization::load(buffer, folderPath);
    AssetFolder* folder = get().getOrCreateFolder(folderPath);

    PAsset asset;
    switch (identifier) {
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

void AssetRegistry::saveAsset(PAsset asset, uint64 identifier, const std::filesystem::path& folderPath, std::string name) {
    if (name.empty())
        return;

    std::string path = (folderPath / name).string().append(".asset");
    auto assetStream = createWriteStream(std::move(path), std::ios::binary);
    ArchiveBuffer buffer(get().graphics);
    // write identifier
    Serialization::save(buffer, identifier);
    // write name
    Serialization::save(buffer, name);
    // write folder
    Serialization::save(buffer, folderPath.string());
    // write asset data
    asset->save(buffer);
    buffer.writeToStream(assetStream);
}

AssetRegistry& AssetRegistry::get() { return *_instance; }

AssetRegistry::AssetRegistry() : assetRoot(nullptr) {}

AssetRegistry* AssetRegistry::getInstance() { return _instance; }

void AssetRegistry::loadRegistry() {
    get().loadRegistryInternal();
}

void AssetRegistry::saveRegistry() {
    get().saveRegistryInternal();
}

AssetRegistry::AssetFolder* AssetRegistry::getOrCreateFolder(std::string_view fullPath) {
    AssetFolder* result = assetRoot;
    std::string temp = std::string(fullPath);
    while (!temp.empty()) {
        size_t slashLoc = temp.find("/");
        if (slashLoc == std::string::npos) {
            if (!result->children.contains(temp)) {
                result->children[temp] = new AssetFolder(temp);
            }
            return result->children[temp];
        }
        std::string folderName = temp.substr(0, slashLoc);
        if (!result->children.contains(folderName)) {
            result->children[folderName] = new AssetFolder(fullPath);
        }
        result = result->children[folderName];
        temp = temp.substr(slashLoc + 1, temp.size());
    }
    return result;
}

void AssetRegistry::initialize(const std::filesystem::path& _rootFolder, Gfx::PGraphics _graphics) {
    this->graphics = _graphics;
    this->rootFolder = _rootFolder;
    this->assetRoot = new AssetFolder("");
    loadRegistryInternal();
}

void AssetRegistry::loadRegistryInternal() {
    List<Pair<PAsset, ArchiveBuffer>> peeked; 
    {
        std::unique_lock l(get().assetLock);
        peeked = peekFolder(assetRoot);
    }
    for (auto& [asset, buffer] : peeked) {
        asset->load(buffer);
    }
}

List<Pair<PAsset, ArchiveBuffer>> AssetRegistry::peekFolder(AssetFolder* folder) {
    List<Pair<PAsset, ArchiveBuffer>> peeked;
    for (const auto& entry : std::filesystem::directory_iterator(rootFolder / folder->folderPath)) {
        const auto& stem = entry.path().stem().string();
        if (entry.is_directory()) {
            if (folder->folderPath.empty()) {
                folder->children[stem] = new AssetFolder(stem);
            } else {
                folder->children[stem] = new AssetFolder(folder->folderPath + "/" + stem);
            }
            auto temp = peekFolder(folder->children[stem]);
            for (auto t : temp) {
                peeked.add(t);
            }
            continue;
        }
        if (entry.path().filename().compare(".DS_Store") == 0) {
            continue;
        }
        auto stream = std::ifstream(entry.path(), std::ios::binary);

        ArchiveBuffer buffer(graphics);
        buffer.readFromStream(stream);
        peeked.add(peekAsset(buffer));
    }
    return peeked;
}

Pair<PAsset, ArchiveBuffer> AssetRegistry::peekAsset(ArchiveBuffer& buffer) {
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
    switch (identifier) {
    case TextureAsset::IDENTIFIER:
        folder->textures[name] = new TextureAsset(folderPath, name);
        asset = PTextureAsset(folder->textures[name]);
        break;
    case MeshAsset::IDENTIFIER:
        folder->meshes[name] = new MeshAsset(folderPath, name);
        asset = PMeshAsset(folder->meshes[name]);
        break;
    case MaterialAsset::IDENTIFIER:
        folder->materials[name] = new MaterialAsset(folderPath, name);
        asset = PMaterialAsset(folder->materials[name]);
        break;
    case MaterialInstanceAsset::IDENTIFIER:
        folder->instances[name] = new MaterialInstanceAsset(folderPath, name);
        asset = PMaterialInstanceAsset(folder->instances[name]);
        break;
    case FontAsset::IDENTIFIER:
        folder->fonts[name] = new FontAsset(folderPath, name);
        asset = PFontAsset(folder->fonts[name]);
        break;
    default:
        throw new std::logic_error("Unknown Identifier");
    }
    return {asset, buffer};
}


void AssetRegistry::saveRegistryInternal() { saveFolder("", assetRoot); }

void AssetRegistry::saveFolder(const std::filesystem::path& folderPath, AssetFolder* folder) {
    std::filesystem::create_directory(rootFolder / folderPath);
    for (const auto& [name, texture] : folder->textures) {
        saveAsset(PTextureAsset(texture), TextureAsset::IDENTIFIER, folderPath, name);
    }
    for (const auto& [name, mesh] : folder->meshes) {
        saveAsset(PMeshAsset(mesh), MeshAsset::IDENTIFIER, folderPath, name);
    }
    for (const auto& [name, material] : folder->materials) {
        saveAsset(PMaterialAsset(material), MaterialAsset::IDENTIFIER, folderPath, name);
    }
    for (const auto& [name, material] : folder->instances) {
        saveAsset(PMaterialInstanceAsset(material), MaterialInstanceAsset::IDENTIFIER, folderPath, name);
    }
    for (const auto& [name, font] : folder->fonts) {
        saveAsset(PFontAsset(font), FontAsset::IDENTIFIER, folderPath, name);
    }
    for (auto& [name, child] : folder->children) {
        saveFolder(folderPath / name, child);
    }
}

std::filesystem::path AssetRegistry::getRootFolder() { return get().rootFolder; }

void AssetRegistry::registerMeshInternal(OMeshAsset mesh) {
    AssetFolder* folder = getOrCreateFolder(mesh->getFolderPath());
    folder->meshes[mesh->getName()] = std::move(mesh);
}

void AssetRegistry::registerTextureInternal(OTextureAsset texture) {
    AssetFolder* folder = getOrCreateFolder(texture->getFolderPath());
    folder->textures[texture->getName()] = std::move(texture);
}

void AssetRegistry::registerFontInternal(OFontAsset font) {
    AssetFolder* folder = getOrCreateFolder(font->getFolderPath());
    folder->fonts[font->getName()] = std::move(font);
}

void AssetRegistry::registerMaterialInternal(OMaterialAsset material) {
    AssetFolder* folder = getOrCreateFolder(material->getFolderPath());
    folder->materials[material->getName()] = std::move(material);
}

void AssetRegistry::registerMaterialInstanceInternal(OMaterialInstanceAsset material) {
    AssetFolder* folder = getOrCreateFolder(material->getFolderPath());
    folder->instances[material->getName()] = std::move(material);
}

std::ofstream AssetRegistry::internalCreateWriteStream(const std::filesystem::path& relativePath, std::ios_base::openmode openmode) {
    auto fullPath = rootFolder / relativePath;
    std::filesystem::create_directories(fullPath.parent_path());
    return std::ofstream(fullPath.string(), openmode);
}

std::ifstream AssetRegistry::internalCreateReadStream(const std::filesystem::path& relativePath, std::ios_base::openmode openmode) {
    auto fullPath = rootFolder / relativePath;
    std::filesystem::create_directories(fullPath.parent_path());
    return std::ifstream(fullPath.string(), openmode);
}

AssetRegistry::AssetFolder::AssetFolder(std::string_view folderPath) : folderPath(folderPath) {}

AssetRegistry::AssetFolder::~AssetFolder() {
    for (auto [_, child] : children) {
        delete child;
    }
}
