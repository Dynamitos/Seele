#include "MeshLoader.h"
#include "Graphics/GraphicsResources.h"
#include "Graphics/Graphics.h"
#include "MeshAsset.h"
#include "Graphics/Mesh.h"
#include "Graphics/StaticMeshVertexInput.h"
#include "AssetRegistry.h"
#include "Material/Material.h"
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
    //futures.add(std::async(std::launch::async, &MeshLoader::import, this, path));
    import(path);
}

void loadMaterials(const aiScene* scene, Array<PMaterialAsset>& globalMaterials, Gfx::PGraphics graphics)
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
                    {"default", texPath.C_Str()}
                };
            code.push_back("result.baseColor = diffuseTexture.Sample(textureSampler, geometry.texCoord).xyz;\n");
        }
        if(material->GetTexture(aiTextureType_SPECULAR, 0, &texPath) == AI_SUCCESS)
        {
            std::string texFilename = std::filesystem::path(texPath.C_Str()).stem().string();
            matCode["params"]["specularTexture"] =
                {
                    {"type", "Texture2D"},
                    {"default", texPath.C_Str()}
                };
            code.push_back("result.specular = specularTexture.Sample(textureSampler, geometry.texCoord).xyz;\n");
        }
        if(material->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS)
        {
            std::string texFilename = std::filesystem::path(texPath.C_Str()).stem().string();
            matCode["params"]["normalTexture"] =
                {
                    {"type", "Texture2D"},
                    {"default", texPath.C_Str()}
                };
            code.push_back("float3 bumpMapNormal = normalTexture.Sample(textureSampler, geometry.texCoord).xyz;\n");
            code.push_back("bumpMapNormal = 2.0 * bumpMapNormal - float3(1.0f, 1.0f, 1.0f);\n");
            code.push_back("result.normal = geometry.transformLocalToWorld(bumpMapNormal);\n");
        }
        code.push_back("return result;\n");
        matCode["code"] = code;
        std::string outMatFilename = matCode["name"].get<std::string>().append(".asset");
        std::ofstream outMatFile = AssetRegistry::createWriteStream(outMatFilename);
        outMatFile << std::setw(4) << matCode;
        outMatFile.flush();
        outMatFile.close();
        //TODO: let the material loader handle this instead
        std::cout << matCode["name"] << std::endl;
        AssetRegistry::importFile(outMatFilename);
        PMaterialAsset asset = AssetRegistry::findMaterial(outMatFilename);
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
void loadToBuffer(Array<Vector>& buffer, const aiVector3D* sourceData, uint32 size)
{
    buffer.resize(size);
    for(uint32 i = 0; i < size; ++i)
    {
        buffer[i] = Vector(sourceData[i].x, sourceData[i].y, sourceData[i].z);
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
    vbInfo.resourceData.size = buffer.size();
    const Gfx::PVertexBuffer vertexBuffer = graphics->createVertexBuffer(vbInfo);
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
    vbInfo.resourceData.size = buffer.size();
    Gfx::PVertexBuffer vertexBuffer = graphics->createVertexBuffer(vbInfo);
    return VertexStreamComponent(vertexBuffer, 0, vbInfo.vertexSize, Gfx::SE_FORMAT_R32G32_SFLOAT);
}
void loadGlobalMeshes(const aiScene* scene, Array<PMesh>& globalMeshes, Array<PMaterialAsset> materials, Gfx::PGraphics graphics)
{
    for (uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh *mesh = scene->mMeshes[meshIndex];

        PStaticMeshVertexInput vertexShaderInput = new StaticMeshVertexInput(std::string(mesh->mName.C_Str()));
        VertexStreamComponent positionStream = createVertexStream(mesh->mNumVertices, mesh->mVertices, graphics);
        vertexShaderInput->setPositionStream(positionStream);

        for(uint32 i = 0; i < MAX_TEXCOORDS; ++i)
        {
            if(mesh->HasTextureCoords(i))
            {
                VertexStreamComponent texCoordStream = createVertexStream(mesh->mNumVertices, mesh->mTextureCoords[i], graphics);
                vertexShaderInput->setTexCoordStream(i, texCoordStream);
            }
        }
        if(mesh->HasNormals())
        {
            VertexStreamComponent normalStream = createVertexStream(mesh->mNumVertices, mesh->mNormals, graphics);
            vertexShaderInput->setTangentXStream(normalStream);
        }
        if(mesh->HasTangentsAndBitangents())
        {
            //TODO: use bitangent to calculate sign for 4th coordinate of tangentstream
            VertexStreamComponent tangentStream = createVertexStream(mesh->mNumVertices, mesh->mTangents, graphics);
            vertexShaderInput->setTangentZStream(tangentStream);
        }

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

        globalMeshes[meshIndex] = new Mesh(vertexShaderInput, indexBuffer);
        globalMeshes[meshIndex]->referencedMaterial = materials[mesh->mMaterialIndex];
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
    
    Array<PMaterialAsset> globalMaterials(scene->mNumMaterials);
    loadTextures(scene, graphics, path);
    loadMaterials(scene, globalMaterials, graphics);
    
    Array<PMesh> globalMeshes(scene->mNumMeshes);
    loadGlobalMeshes(scene, globalMeshes, globalMaterials, graphics);


    List<aiNode *> meshNodes;
    findMeshRoots(scene->mRootNode, meshNodes);
    std::filesystem::path filePath = path.filename();
    filePath.replace_extension("asset");
    PMeshAsset meshAsset = new MeshAsset(filePath.generic_string());
    for (auto meshNode : meshNodes)
    {
        for(uint32 i = 0; i < meshNode->mNumMeshes; ++i)
        {
            meshAsset->addMesh(globalMeshes[meshNode->mMeshes[i]]);
        }
    }
    meshAsset->save();
    AssetRegistry::get().registerMesh(meshAsset);
}
