#include "MeshLoader.h"
#include "Graphics/Graphics.h"
#include "Asset/MeshAsset.h"
#include "Graphics/Mesh.h"
#include "Graphics/StaticMeshVertexData.h"
#include "Asset/AssetImporter.h"
#include "Asset/MaterialAsset.h"
#include <set>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stb_image_write.h>
#include <assimp/config.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <Asset/MaterialLoader.h>
#include <Asset/TextureLoader.h>

using namespace Seele;

MeshLoader::MeshLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
}

MeshLoader::~MeshLoader()
{
}

void MeshLoader::importAsset(MeshImportArgs args)
{
    std::filesystem::path assetPath = args.filePath.filename();
    assetPath.replace_extension("asset");
    OMeshAsset asset = new MeshAsset(args.importPath, assetPath.stem().string());
    PMeshAsset ref = asset;
    asset->setStatus(Asset::Status::Loading);
    AssetRegistry::get().registerMesh(std::move(asset));
    import(args, ref);
}

void MeshLoader::loadMaterials(const aiScene* scene, const std::string& baseName, const std::filesystem::path& meshDirectory, const std::string& importPath, Array<PMaterialAsset>& globalMaterials)
{
    using json = nlohmann::json;
    for(uint32 i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* material = scene->mMaterials[i];
        json matCode;
        matCode["name"] = baseName + material->GetName().C_Str();
        matCode["profile"] = "BlinnPhong"; //TODO: other shading models
        aiString texPath;
        //TODO make samplers based on used textures
        matCode["params"]["textureSampler"] =
            {
                {"type", "SamplerState"}
            };
        if(material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
        {
            std::string texFilename = std::filesystem::path(texPath.C_Str()).replace_extension("asset").stem().string();
            matCode["params"]["diffuseTexture"] = 
                {
                    {"type", "Texture2D"}, 
                    {"default", texFilename}
                };
            matCode["code"].push_back(
                {
                    { "exp",  "Sample" },
                    { "texture", "diffuseTexture" },
                    { "sampler", "textureSampler" },
                    { "coords", "input.texCoords[0]"}
                }
            );
            matCode["code"].push_back(
                {
                    { "exp", "Swizzle" },
                    { "target", 0 },
                    { "comp", json::array({0, 1, 2}) },
                }
            );
            matCode["code"].push_back(
                {
                    { "exp", "BRDF" },
                    { "profile", "BlinnPhong" },
                    { "values", {
                        {"baseColor", 1}
                    }}
                }
            );
        }
        else
        {
            matCode["code"].push_back(
                {
                    { "exp", "BRDF" },
                    { "profile", "BlinnPhong" },
                    { "values", {
                        {"baseColor", "input.vertexColor.xyz"}
                    }}
                }
            );
        }
        if(material->GetTexture(aiTextureType_SPECULAR, 0, &texPath) == AI_SUCCESS)
        {
        }
        if(material->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS)
        {
        }
        std::string outMatFilename = matCode["name"].get<std::string>().append(".asset");
        std::ofstream outMatFile = std::ofstream(meshDirectory / outMatFilename);
        outMatFile << std::setw(4) << matCode;
        outMatFile.flush();
        outMatFile.close();

        std::cout << "writing json to " << outMatFilename << std::endl;
        AssetImporter::importMaterial(MaterialImportArgs{
            .filePath = meshDirectory / outMatFilename,
            .importPath = importPath,
            });
        globalMaterials[i] = AssetRegistry::findMaterial(matCode["name"].get<std::string>());
    }
}

void findMeshRoots(aiNode *node, List<aiNode *> &meshNodes)
{
    if (node->mNumMeshes > 0)
    {
        meshNodes.add(node);
        return;
    }
    for (uint32 i = 0; i < node->mNumChildren; ++i)
    {
        findMeshRoots(node->mChildren[i], meshNodes);
    }
}

void MeshLoader::loadGlobalMeshes(const aiScene* scene, const Array<PMaterialAsset>& materials, Array<OMesh>& globalMeshes, Component::Collider& collider)
{
    for (uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh *mesh = scene->mMeshes[meshIndex];
        collider.boundingbox.adjust(Vector(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z));
        collider.boundingbox.adjust(Vector(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z));

        // assume static mesh for now
        Array<Vector> positions(mesh->mNumVertices);
        Array<Vector2> texCoords(mesh->mNumVertices);
        Array<Vector> normals(mesh->mNumVertices);
        Array<Vector> tangents(mesh->mNumVertices);
        Array<Vector> biTangents(mesh->mNumVertices);

        StaticMeshVertexData* vertexData = StaticMeshVertexData::getInstance();

        for(uint32 i = 0; i < mesh->mNumVertices; ++i)
        {
            positions[i] = Vector(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            texCoords[i] = Vector2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].x);
            normals[i] = Vector(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            tangents[i] = Vector(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            biTangents[i] = Vector(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        }

        MeshId id = vertexData->allocateVertexData(mesh->mNumVertices);
        vertexData->loadPositions(id, positions);
        vertexData->loadTexCoords(id, texCoords);
        vertexData->loadNormals(id, normals);
        vertexData->loadTangents(id, tangents);
        vertexData->loadBiTangents(id, biTangents);

        Array<uint32> indices(mesh->mNumFaces * 3);
        for (size_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
        {
            indices[faceIndex * 3 + 0] = mesh->mFaces[faceIndex].mIndices[0];
            indices[faceIndex * 3 + 1] = mesh->mFaces[faceIndex].mIndices[1];
            indices[faceIndex * 3 + 2] = mesh->mFaces[faceIndex].mIndices[2];
        }

        Array<Meshlet> meshlets;
        if (Gfx::useMeshShading)
        {
            meshlets.reserve(indices.size() / (3ull * Gfx::numPrimitivesPerMeshlet));
            std::set<uint32> uniqueVertices;
            Meshlet current = {
                .numVertices = 0,
                .numPrimitives = 0,
            };
            auto insertAndGetIndex = [&uniqueVertices, &current](uint32 index) -> int8_t
            {
                auto [it, inserted] = uniqueVertices.insert(index);
                if (inserted)
                {
                    if (current.numVertices == Gfx::numVerticesPerMeshlet)
                    {
                        return -1;
                    }
                    current.uniqueVertices[current.numVertices] = index;
                    return current.numVertices++;
                }
                else
                {
                    for (uint32 i = 0; i < current.numVertices; ++i)
                    {
                        if (current.uniqueVertices[i] == index)
                        {
                            return i;
                        }
                    }
                    assert(false);
                }
            };
            auto completeMeshlet = [&meshlets, &current, &uniqueVertices]() {
                meshlets.add(current);
                current = {
                    .numVertices = 0,
                    .numPrimitives = 0,
                };
                uniqueVertices.clear();
            };
            for (size_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
            {
                auto i1 = insertAndGetIndex(mesh->mFaces[faceIndex].mIndices[0]);
                auto i2 = insertAndGetIndex(mesh->mFaces[faceIndex].mIndices[1]);
                auto i3 = insertAndGetIndex(mesh->mFaces[faceIndex].mIndices[2]);
                if (i1 == -1 || i2 == -1 || i3 == -1)
                {
                    completeMeshlet();
                }
                current.primitiveLayout[current.numPrimitives * 3 + 0] = i1;
                current.primitiveLayout[current.numPrimitives * 3 + 1] = i2;
                current.primitiveLayout[current.numPrimitives * 3 + 2] = i3;
                current.numPrimitives++;
                if (current.numPrimitives == Gfx::numPrimitivesPerMeshlet)
                {
                    completeMeshlet();
                }
            }
            vertexData->loadMesh(id, meshlets);
        }
        else
        {
            // \! todo
        }
        

        collider.physicsMesh.addCollider(positions, indices, Matrix4(1.0f));

        IndexBufferCreateInfo idxInfo;
        idxInfo.indexType = Gfx::SE_INDEX_TYPE_UINT32;
        idxInfo.sourceData.data = (uint8 *)indices.data();
        idxInfo.sourceData.owner = Gfx::QueueType::DEDICATED_TRANSFER;
        idxInfo.sourceData.size = sizeof(uint32) * indices.size();
        Gfx::PIndexBuffer indexBuffer = graphics->createIndexBuffer(idxInfo);
        indexBuffer->transferOwnership(Gfx::QueueType::GRAPHICS);

        globalMeshes[meshIndex] = new Mesh();
        globalMeshes[meshIndex]->vertexData = vertexData;
        globalMeshes[meshIndex]->id = id;
        globalMeshes[meshIndex]->referencedMaterial = materials[mesh->mMaterialIndex]->getMaterial()->instantiate();
        globalMeshes[meshIndex]->meshlets = std::move(meshlets);
        globalMeshes[meshIndex]->vertexCount = mesh->mNumVertices;
    }
}

void MeshLoader::convertAssimpARGB(unsigned char* dst, aiTexel* src, uint32 numPixels)
{
    for(uint32 i = 0; i < numPixels; ++i)
    {
        dst[i * 4 + 0] = src[i].r;
        dst[i * 4 + 1] = src[i].g;
        dst[i * 4 + 2] = src[i].b;
        dst[i * 4 + 3] = src[i].a;
    }
}
void MeshLoader::loadTextures(const aiScene* scene, const std::filesystem::path& meshDirectory, const std::string& importPath)
{
    for (uint32 i = 0; i < scene->mNumTextures; ++i)
    {
        aiTexture* tex = scene->mTextures[i];
        auto texPath = std::filesystem::path(tex->mFilename.C_Str());
        auto texPngPath = meshDirectory;
        texPngPath.append(texPath.filename().string());
        if(tex->mHeight == 0)
        {
            // already compressed, just dump it to the disk
            std::ofstream file(texPngPath, std::ios::binary);
            file.write((const char*)tex->pcData, tex->mWidth);
            file.flush();
        }
        else
        {
            // recompress data so that the TextureLoader can read it
            unsigned char* texData = new unsigned char[tex->mWidth * tex->mHeight * 4];
            convertAssimpARGB(texData, tex->pcData, tex->mWidth * tex->mHeight);
            stbi_write_png(texPngPath.string().c_str(), tex->mWidth, tex->mHeight, 4, tex->pcData, tex->mWidth * 32);
            delete[] texData;
        }
        std::cout << "Loading model texture " << texPngPath.string() << std::endl;
        AssetImporter::importTexture(TextureImportArgs {
            .filePath = texPath,
            .importPath = importPath,
            });
    }
}
void MeshLoader::import(MeshImportArgs args, PMeshAsset meshAsset)
{
    std::cout << "Starting to import "<<args.filePath<< std::endl;
    meshAsset->setStatus(Asset::Status::Loading);
    Assimp::Importer importer;
    importer.ReadFile(args.filePath.string().c_str(), (uint32)(
        aiProcess_FlipUVs |
        aiProcess_Triangulate |
        aiProcess_SortByPType |
        aiProcess_GenBoundingBoxes |
        aiProcess_GenSmoothNormals |
        aiProcess_GenUVCoords |
        aiProcess_FindDegenerates));
    const aiScene *scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
    
    Array<PMaterialAsset> globalMaterials(scene->mNumMaterials);
    loadTextures(scene, args.filePath.parent_path(), args.importPath);
    loadMaterials(scene, args.filePath.stem().string(), args.filePath.parent_path(), args.importPath, globalMaterials);
    
    Array<OMesh> globalMeshes(scene->mNumMeshes);
    Component::Collider collider;
    loadGlobalMeshes(scene, globalMaterials, globalMeshes, collider);

    List<aiNode *> meshNodes;
    findMeshRoots(scene->mRootNode, meshNodes);
    
    Array<OMesh> meshes;
    for (auto meshNode : meshNodes)
    {
        for(uint32 i = 0; i < meshNode->mNumMeshes; ++i)
        {
            meshes.add(std::move(globalMeshes[meshNode->mMeshes[i]]));
        }
    }
    meshAsset->meshes = std::move(meshes);
    meshAsset->physicsMesh = std::move(collider);


    auto stream = AssetRegistry::createWriteStream((std::filesystem::path(meshAsset->getFolderPath()) / meshAsset->getName()).replace_extension("asset").string(), std::ios::binary);

    ArchiveBuffer archive;
    Serialization::save(archive, MeshAsset::IDENTIFIER);
    Serialization::save(archive, meshAsset->getName());
    Serialization::save(archive, meshAsset->getFolderPath());
    meshAsset->save(archive);
    archive.writeToStream(stream);

    meshAsset->setStatus(Asset::Status::Ready);
    std::cout << "Finished loading " << args.filePath << std::endl;
}
