#include "MeshLoader.h"
#include "Graphics/GraphicsResources.h"
#include "Graphics/Graphics.h"
#include "MeshAsset.h"
#include "Graphics/Mesh.h"
#include "Graphics/StaticMeshVertexInput.h"
#include "AssetRegistry.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stb_image_write.h>
#include <assimp/config.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

using namespace Seele;

MeshLoader::MeshLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
}

MeshLoader::~MeshLoader()
{
}

void MeshLoader::importAsset(const std::filesystem::path &path)
{
    std::filesystem::path assetPath = path.filename();
    assetPath.replace_extension("asset");
    PMeshAsset asset = new MeshAsset(assetPath.generic_string());
    asset->setStatus(Asset::Status::Loading);
    AssetRegistry::get().registerMesh(asset);
    import(path, asset);
}

void MeshLoader::loadMaterials(const aiScene* scene, Array<PMaterialAsset>& globalMaterials, Gfx::PGraphics graphics)
{
    using json = nlohmann::json;
    for(uint32 i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* material = scene->mMaterials[i];
        json matCode;
        matCode["name"] = material->GetName().C_Str();
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
            matCode["code"]["baseColor"] = "return diffuseTexture.Sample(textureSampler, input.texCoords[0]).xyz;";
        }
        if(material->GetTexture(aiTextureType_SPECULAR, 0, &texPath) == AI_SUCCESS)
        {
            std::string texFilename = std::filesystem::path(texPath.C_Str()).replace_extension("asset").stem().string();
            matCode["params"]["specularTexture"] =
                {
                    {"type", "Texture2D"},
                    {"default", texFilename}
                };
            matCode["code"]["specular"] = "return specularTexture.Sample(textureSampler, input.texCoords[0]).x;";
        }
        if(material->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS)
        {
            std::string texFilename = std::filesystem::path(texPath.C_Str()).replace_extension("asset").stem().string();
            matCode["params"]["normalTexture"] =
                {
                    {"type", "Texture2D"},
                    {"default", texFilename}
                };
            matCode["code"]["normal"] = "return normalTexture.Sample(textureSampler, input.texCoords[0]).xyz;";
        }
        std::string outMatFilename = matCode["name"].get<std::string>().append(".asset");
        std::ofstream outMatFile = AssetRegistry::createWriteStream(outMatFilename);
        outMatFile << std::setw(4) << matCode;
        outMatFile.flush();
        outMatFile.close();

        std::cout << "writing json to " << outMatFilename << std::endl;
        PMaterialAsset result = new MaterialAsset(outMatFilename);
        result->load();
        graphics->getShaderCompiler()->registerMaterial(result);
        AssetRegistry::get().registerMaterial(result);
        PMaterialAsset asset = AssetRegistry::findMaterial(result->getFileName());
        globalMaterials[i] = asset;
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
VertexStreamComponent createVertexStream(uint32 size, aiVector3D* sourceData, Gfx::PGraphics graphics)
{
    Array<Vector> buffer(size);
    for(uint32 i = 0; i < size; ++i)
    {
        buffer[i] = Vector(sourceData[i].x, sourceData[i].y, sourceData[i].z);
    }    
    VertexBufferCreateInfo vbInfo;
    vbInfo.numVertices = size;
    vbInfo.vertexSize = sizeof(Vector);
    vbInfo.resourceData.data = (uint8 *)buffer.data();
    vbInfo.resourceData.owner = Gfx::QueueType::DEDICATED_TRANSFER;
    vbInfo.resourceData.size = sizeof(Vector) * (uint32)buffer.size();
    Gfx::PVertexBuffer vertexBuffer = graphics->createVertexBuffer(vbInfo);
    vertexBuffer->transferOwnership(Gfx::QueueType::GRAPHICS);
    return VertexStreamComponent(vertexBuffer, 0, vbInfo.vertexSize, Gfx::SE_FORMAT_R32G32B32_SFLOAT);
}
VertexStreamComponent createVertexStream(uint32 size, aiVector2D* sourceData, Gfx::PGraphics graphics)
{
    Array<Vector2> buffer(size);
    for(uint32 i = 0; i < size; ++i)
    {
        buffer[i] = Vector2(sourceData[i].x, sourceData[i].y);
    }    
    VertexBufferCreateInfo vbInfo;
    vbInfo.numVertices = size;
    vbInfo.vertexSize = sizeof(Vector2);
    vbInfo.resourceData.data = (uint8 *)buffer.data();
    vbInfo.resourceData.owner = Gfx::QueueType::DEDICATED_TRANSFER;
    vbInfo.resourceData.size = sizeof(Vector2) * (uint32)buffer.size();
    Gfx::PVertexBuffer vertexBuffer = graphics->createVertexBuffer(vbInfo);
    vertexBuffer->transferOwnership(Gfx::QueueType::GRAPHICS);
    return VertexStreamComponent(vertexBuffer, 0, vbInfo.vertexSize, Gfx::SE_FORMAT_R32G32_SFLOAT);
}
void MeshLoader::loadGlobalMeshes(const aiScene* scene, Array<PMesh>& globalMeshes, const Array<PMaterialAsset>& materials, Gfx::PGraphics graphics)
{
    for (uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh *mesh = scene->mMeshes[meshIndex];

        PStaticMeshVertexInput vertexShaderInput = new StaticMeshVertexInput(std::string(mesh->mName.C_Str()));
        StaticMeshDataType data;
        data.positionStream = createVertexStream(mesh->mNumVertices, mesh->mVertices, graphics);

        for(uint32 i = 0; i < MAX_TEXCOORDS; ++i)
        {
            if(mesh->HasTextureCoords(i))
            {
                data.textureCoordinates.add(createVertexStream(mesh->mNumVertices, mesh->mTextureCoords[i], graphics));
            }
        }
        if(mesh->HasNormals())
        {
            data.tangentBasisComponents[0] = createVertexStream(mesh->mNumVertices, mesh->mNormals, graphics);
        }
        if(mesh->HasTangentsAndBitangents())
        {
            //TODO: use bitangent to calculate sign for 4th coordinate of tangentstream
            data.tangentBasisComponents[1] = createVertexStream(mesh->mNumVertices, mesh->mTangents, graphics);
            data.tangentBasisComponents[2] = createVertexStream(mesh->mNumVertices, mesh->mBitangents, graphics);
        }
        if(mesh->HasVertexColors(0))
        {
            //data.colorComponent = createVertexStream(mesh->mNumVertices, mesh->mColors[0], graphics);
        }
        vertexShaderInput->setData(std::move(data));
        vertexShaderInput->init(graphics);

        Array<uint32> indices(mesh->mNumFaces * 3);
        for (uint32 faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
        {
            indices[faceIndex * 3 + 0] = mesh->mFaces[faceIndex].mIndices[0];
            indices[faceIndex * 3 + 1] = mesh->mFaces[faceIndex].mIndices[1];
            indices[faceIndex * 3 + 2] = mesh->mFaces[faceIndex].mIndices[2];
        }
        IndexBufferCreateInfo idxInfo;
        idxInfo.indexType = Gfx::SE_INDEX_TYPE_UINT32;
        idxInfo.resourceData.data = (uint8 *)indices.data();
        idxInfo.resourceData.owner = Gfx::QueueType::DEDICATED_TRANSFER;
        idxInfo.resourceData.size = sizeof(uint32) * (uint32)indices.size();
        Gfx::PIndexBuffer indexBuffer = graphics->createIndexBuffer(idxInfo);
        indexBuffer->transferOwnership(Gfx::QueueType::GRAPHICS);

        globalMeshes[meshIndex] = new Mesh(vertexShaderInput, indexBuffer);
        globalMeshes[meshIndex]->referencedMaterial = materials[mesh->mMaterialIndex];
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
void MeshLoader::loadTextures(const aiScene* scene, const std::filesystem::path& meshDirectory)
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
            delete texData;
        }
        std::cout << "Loading model texture " << texPngPath.string() << std::endl;
        AssetRegistry::importFile(texPngPath.string());
    }
}
void MeshLoader::import(std::filesystem::path path, PMeshAsset meshAsset)
{
    std::cout << "Starting to import "<<path << std::endl;
    meshAsset->setStatus(Asset::Status::Loading);
    Assimp::Importer importer;
    importer.ReadFile(path.string().c_str(),
        aiProcess_FlipUVs |
        aiProcess_Triangulate |
        aiProcess_SortByPType |
        aiProcess_GenSmoothNormals |
        aiProcess_GenUVCoords |
        aiProcess_FindDegenerates);
    const aiScene *scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
    
    Array<PMaterialAsset> globalMaterials(scene->mNumMaterials);
    loadTextures(scene, path.parent_path());
    loadMaterials(scene, globalMaterials, graphics);
    
    Array<PMesh> globalMeshes(scene->mNumMeshes);
    loadGlobalMeshes(scene, globalMeshes, globalMaterials, graphics);


    List<aiNode *> meshNodes;
    findMeshRoots(scene->mRootNode, meshNodes);
    
    for (auto meshNode : meshNodes)
    {
        for(uint32 i = 0; i < meshNode->mNumMeshes; ++i)
        {
            meshAsset->addMesh(globalMeshes[meshNode->mMeshes[i]]);
        }
    }
    meshAsset->setStatus(Asset::Status::Ready);
    meshAsset->save();
    std::cout << "Finished loading " << path << std::endl;
}
