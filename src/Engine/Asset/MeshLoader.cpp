#include "MeshLoader.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsResources.h"
#include "MeshAsset.h"
#include "Graphics/Mesh.h"
#include "AssetRegistry.h"
#include <filesystem>
#include <fstream>
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
    futures.add(std::async(std::launch::async, &MeshLoader::import, this, path));
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
void loadToBuffer(Array<Vector>& buffer, const aiVector3D* sourceData, uint32 size)
{
    buffer.resize(size);
    for(uint32 i = 0; i < size; ++i)
    {
        buffer[i] = Vector(sourceData[i].x, sourceData[i].y, sourceData[i].z);
    }
}
Gfx::VertexStream createVertexStream(uint32 size, aiVector3D* sourceData, Gfx::PGraphics graphics)
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
    vbInfo.resourceData.size = buffer.size();
    Gfx::PVertexBuffer vertexBuffer = graphics->createVertexBuffer(vbInfo);
    auto stream = Gfx::VertexStream(vbInfo.vertexSize, 0, false, vertexBuffer);
    stream.addVertexElement(Gfx::VertexElement(0, Gfx::SE_FORMAT_R32G32_SFLOAT, 0));
    return stream;
}
Gfx::VertexStream createVertexStream(uint32 size, aiVector2D* sourceData, Gfx::PGraphics graphics)
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
    vbInfo.resourceData.size = buffer.size();
    Gfx::PVertexBuffer vertexBuffer = graphics->createVertexBuffer(vbInfo);
    auto stream = Gfx::VertexStream(vbInfo.vertexSize, 0, false, vertexBuffer);
    stream.addVertexElement(Gfx::VertexElement(0, Gfx::SE_FORMAT_R32G32B32_SFLOAT, 0));
    return stream;
}
void loadGlobalMeshes(const aiScene* scene, Array<PMesh>& globalMeshes, Gfx::PGraphics graphics)
{
    for (uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh *mesh = scene->mMeshes[meshIndex];

        MeshDescription description;
        Gfx::PVertexDeclaration declaration = new Gfx::VertexDeclaration();
        declaration->addVertexStream(createVertexStream(mesh->mNumVertices, mesh->mVertices, graphics));
        description.layout.add(VertexAttribute::POSITION);

        for(uint32 i = 0; i < MAX_TEX_CHANNELS; ++i)
        {
            if(mesh->HasTextureCoords(i))
            {
                declaration->addVertexStream(createVertexStream(mesh->mNumVertices, mesh->mTextureCoords[i], graphics));
                description.layout.add(VertexAttribute::TEXCOORD);
            }
        }
        if(mesh->HasNormals())
        {
            declaration->addVertexStream(createVertexStream(mesh->mNumVertices, mesh->mNormals, graphics));
            description.layout.add(VertexAttribute::NORMAL);
        }
        if(mesh->HasTangentsAndBitangents())
        {
            declaration->addVertexStream(createVertexStream(mesh->mNumVertices, mesh->mVertices, graphics));
            declaration->addVertexStream(createVertexStream(mesh->mNumVertices, mesh->mVertices, graphics));
            description.layout.add(VertexAttribute::TANGENT);
            description.layout.add(VertexAttribute::BITANGENT);
        }
        description.declaration = declaration;

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
        idxInfo.resourceData.size = indices.size();
        Gfx::PIndexBuffer indexBuffer = graphics->createIndexBuffer(idxInfo);
        indexBuffer->transferOwnership(Gfx::QueueType::GRAPHICS);

        globalMeshes[meshIndex] = new Mesh(description, indexBuffer);
    }
}
void convertAssimpARGB(unsigned char* dst, aiTexel* src, uint32 numPixels)
{
    for(uint32 i = 0; i < numPixels; ++i)
    {
        dst[i * 4 + 0] = src[i].r;
        dst[i * 4 + 1] = src[i].g;
        dst[i * 4 + 2] = src[i].b;
        dst[i * 4 + 3] = src[i].a;
    }
}
void loadTextures(const aiScene* scene, Gfx::PGraphics graphics, const std::filesystem::path& meshPath)
{
    for (uint32 i = 0; i < scene->mNumTextures; ++i)
    {
        aiTexture* tex = scene->mTextures[i];
        auto texPath = std::filesystem::path(tex->mFilename.C_Str());
        auto texPngPath = meshPath.parent_path().append(texPath.filename().string());
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

        AssetRegistry::importFile(texPngPath.string());
    }
}
void loadMaterials(const aiScene* scene, Gfx::PGraphics graphics)
{
    using json = nlohmann::json;
    for(uint32 i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* material = scene->mMaterials[i];
        json matCode;
        matCode["name"] = material->GetName().C_Str();
        matCode["profile"] = "BlinnPhong"; //TODO: other shading models
        std::vector<std::string> code;
        aiString texPath;
        //TODO make samplers based on used textures
        matCode["params"]["texSampler"] =
            {
                {"type", "SamplerState"}
            };
        if(material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
        {
            std::string texFilename = std::filesystem::path(texPath.C_Str()).stem().string();
            matCode["params"]["diffuseTexture"] = 
                {
                    {"type", "Texture2D"}, 
                    {"default", texFilename}
                };
            code.push_back("result.baseColor = diffuseTexture.Sample(textureSampler, geometry.texCoord).xyz;\n");
        }
        if(material->GetTexture(aiTextureType_SPECULAR, 0, &texPath) == AI_SUCCESS)
        {
            std::string texFilename = std::filesystem::path(texPath.C_Str()).stem().string();
            matCode["params"]["specularTexture"] =
                {
                    {"type", "Texture2D"},
                    {"default", texFilename}
                };
            code.push_back("result.specular = specularTexture.Sample(textureSampler, geometry.texCoord).xyz;\n");
        }
        if(material->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS)
        {
            std::string texFilename = std::filesystem::path(texPath.C_Str()).stem().string();
            matCode["params"]["normalTexture"] =
                {
                    {"type", "Texture2D"},
                    {"default", texFilename}
                };
            code.push_back("float3 bumpMapNormal = normalTexture.Sample(textureSampler, geometry.texCoord).xyz;\n");
            code.push_back("bumpMapNormal = 2.0 * bumpMapNormal - float3(1.0f, 1.0f, 1.0f);\n");
            code.push_back("result.normal = geometry.transformLocalToWorld(bumpMapNormal);\n");
        }
        code.push_back("return result;\n");
        matCode["code"] = code;
        std::ofstream outMatFile(material->GetName().C_Str());
        outMatFile << std::setw(4) << matCode;
        outMatFile.flush();
    }
}
void MeshLoader::import(const std::filesystem::path &path)
{
    Assimp::Importer importer;
    importer.ReadFile(path.string().c_str(),
        aiProcess_FlipUVs |
        aiProcess_ImproveCacheLocality |
        aiProcess_OptimizeMeshes |
        aiProcess_GenBoundingBoxes |
        aiProcess_Triangulate |
        aiProcess_SortByPType |
        aiProcess_GenSmoothNormals |
        aiProcess_GenUVCoords |
        aiProcess_FindDegenerates |
        aiProcess_EmbedTextures);
    const aiScene *scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
    Array<PMesh> globalMeshes(scene->mNumMeshes);
    loadGlobalMeshes(scene, globalMeshes, graphics);
    loadTextures(scene, graphics, path);
    loadMaterials(scene, graphics);

    List<aiNode *> meshNodes;
    findMeshRoots(scene->mRootNode, meshNodes);
    for (auto meshNode : meshNodes)
    {
        std::string fileName = std::string(meshNode->mName.C_Str()).append(".asset");
        PMeshAsset meshAsset = new MeshAsset(fileName);
        for(uint32 i = 0; i < meshNode->mNumMeshes; ++i)
        {
            meshAsset->addMesh(globalMeshes[meshNode->mMeshes[i]]);
        }
        AssetRegistry::get().meshes[meshAsset->getFileName()] = meshAsset;
    }
}
