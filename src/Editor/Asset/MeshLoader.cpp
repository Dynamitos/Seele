#include "MeshLoader.h"
#include "Asset/AssetImporter.h"
#include "Asset/MaterialAsset.h"
#include "Asset/MeshAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Graphics/StaticMeshVertexData.h"
#include "ThreadPool.h"
#include <Asset/MaterialLoader.h>
#include <Asset/TextureLoader.h>
#include <assimp/Importer.hpp>
#include <assimp/config.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <set>
#include <stb_image_write.h>

using namespace Seele;

MeshLoader::MeshLoader(Gfx::PGraphics graphics) : graphics(graphics) {}

MeshLoader::~MeshLoader() {}

void MeshLoader::importAsset(MeshImportArgs args) {
    std::filesystem::path assetPath = args.filePath.filename();
    assetPath.replace_extension("asset");
    OMeshAsset asset = new MeshAsset(args.importPath, assetPath.stem().string());
    PMeshAsset ref = asset;
    asset->setStatus(Asset::Status::Loading);
    AssetRegistry::get().registerMesh(std::move(asset));
    import(args, ref);
}

void MeshLoader::convertAssimpARGB(unsigned char* dst, aiTexel* src, uint32 numPixels) {
    for (uint32 i = 0; i < numPixels; ++i) {
        dst[i * 4 + 0] = src[i].r;
        dst[i * 4 + 1] = src[i].g;
        dst[i * 4 + 2] = src[i].b;
        dst[i * 4 + 3] = src[i].a;
    }
}
void MeshLoader::loadTextures(const aiScene* scene, const std::filesystem::path& meshDirectory, const std::string& importPath,
                              Array<PTextureAsset>& textures) {
    std::cout << "Loading Textures" << std::endl;
    for (uint32 i = 0; i < scene->mNumTextures; ++i) {
        aiTexture* tex = scene->mTextures[i];
        auto texPath = std::filesystem::path(tex->mFilename.C_Str());
        if (std::filesystem::exists(texPath)) {
        } else if (std::filesystem::exists(meshDirectory / texPath)) {
            texPath = meshDirectory / texPath;
        } else {
            if (tex->mFilename.length == 0) {
                texPath = (meshDirectory / fmt::format("Texture{0}", i));
            } else {
                texPath = (meshDirectory / texPath);
            }
            texPath = texPath.replace_extension(tex->achFormatHint);
            if (tex->mHeight == 0) {
                std::cout << "Dumping texture " << texPath << std::endl;
                // already compressed, just dump it to the disk
                std::ofstream file(texPath, std::ios::binary);
                file.write((const char*)tex->pcData, tex->mWidth);
                file.flush();
            } else {
                std::cout << "Writing extracted png " << texPath << std::endl;
                // recompress data so that the TextureLoader can read it
                unsigned char* texData = new unsigned char[tex->mWidth * tex->mHeight * 4];
                convertAssimpARGB(texData, tex->pcData, tex->mWidth * tex->mHeight);
                stbi_write_png(texPath.string().c_str(), tex->mWidth, tex->mHeight, 4, tex->pcData, tex->mWidth * 32);
                delete[] texData;
            }
        }
        std::cout << "Loading model texture " << texPath.string() << std::endl;
        AssetImporter::importTexture(TextureImportArgs{
            .filePath = texPath,
            .importPath = importPath,
        });
        textures.add(AssetRegistry::findTexture(importPath, texPath.stem().string()));
        std::cout << "Loaded " << i << "/" << scene->mNumTextures << std::endl;
    }
}

constexpr const char* KEY_ALPHA = "k_alpha";
constexpr const char* KEY_DIFFUSE_COLOR = "k_d";
constexpr const char* KEY_SPECULAR_COLOR = "k_s";
constexpr const char* KEY_AMBIENT_COLOR = "k_a";
constexpr const char* KEY_SHININESS = "k_shiny";
constexpr const char* KEY_ROUGHNESS = "k_r";
constexpr const char* KEY_METALLIC = "k_m";

constexpr const char* KEY_DIFFUSE_TEXTURE = "tex_d";
constexpr const char* KEY_SPECULAR_TEXTURE = "tex_s";
constexpr const char* KEY_AMBIENT_TEXTURE = "tex_a";
constexpr const char* KEY_NORMAL_TEXTURE = "tex_n";
constexpr const char* KEY_SHININESS_TEXTURE = "tex_shiny";
constexpr const char* KEY_ROUGHNESS_TEXTURE = "tex_r";
constexpr const char* KEY_METALLIC_TEXTURE = "tex_m";
constexpr const char* KEY_AMBIENT_OCCLUSION_TEXTURE = "tex_ao";

void MeshLoader::loadMaterials(const aiScene* scene, const Array<PTextureAsset>& textures, const std::string& baseName,
                               const std::filesystem::path& meshDirectory, const std::string& importPath,
                               Array<PMaterialInstanceAsset>& globalMaterials) {
    for (uint32 m = 0; m < scene->mNumMaterials; ++m) {
        aiMaterial* material = scene->mMaterials[m];
        aiString texPath;
        std::string materialName = fmt::format("M{0}{1}{2}", baseName, material->GetName().C_Str(), m);
        materialName.erase(std::remove(materialName.begin(), materialName.end(), '.'),
                           materialName.end()); // dots break adding the .asset extension later
        materialName.erase(std::remove(materialName.begin(), materialName.end(), ':'),
                           materialName.end()); // dots break adding the .asset extension later
        materialName.erase(std::remove(materialName.begin(), materialName.end(), '-'),
                           materialName.end()); // dots break adding the .asset extension later
        materialName.erase(std::remove(materialName.begin(), materialName.end(), ' '),
                           materialName.end()); // dots break adding the .asset extension later
        materialName.erase(std::remove(materialName.begin(), materialName.end(), '('),
                           materialName.end()); // dots break adding the .asset extension later
        materialName.erase(std::remove(materialName.begin(), materialName.end(), ')'),
                           materialName.end()); // dots break adding the .asset extension later
        Array<OShaderExpression> expressions;
        Array<std::string> parameters;
        uint32 numTextures = 0;
        uint32 numSamplers = 0;
        uint32 numFloats = 0;
        auto addScalarParameter = [&](std::string paramKey, const char* matKey, int type, int index) {
            float scalar;
            material->Get(matKey, type, index, scalar);
            expressions.add(new FloatParameter(paramKey, scalar, numFloats++));
            parameters.add(paramKey);
        };

        auto addVectorParameter = [&](std::string paramKey, const char* matKey, int type, int index) {
            aiColor3D color;
            material->Get(matKey, type, index, color);
            expressions.add(new VectorParameter(paramKey, Vector(color.r, color.g, color.b), numFloats));
            numFloats += 3;
            parameters.add(paramKey);
        };

        auto addTextureParameter = [&](std::string paramKey, aiTextureType type, int index, std::string& result,
                                       StaticArray<int32, 4> extractMask = {0, 1, 2, -1}, std::string* alpha = nullptr) {
            aiString texPath;
            aiTextureMapping mapping;
            uint32 uvIndex = 0;
            aiTextureMapMode mapMode = aiTextureMapMode_Clamp;
            float blend = std::numeric_limits<float>::max();
            aiTextureOp op;
            if (material->GetTexture(type, index, &texPath, &mapping, &uvIndex, nullptr, nullptr, nullptr) != AI_SUCCESS) {
                std::cout << "fuck" << std::endl;
            }
            std::string textureKey = fmt::format("{0}Texture{1}", paramKey, index);
            auto texFilename = std::filesystem::path(texPath.C_Str());
            PTextureAsset texture;

            if (texFilename.string()[0] == '*') {
                texture = textures[atoi(texFilename.string().substr(1).c_str())];
            } else if (std::filesystem::exists(texFilename)) {
                AssetImporter::importTexture(TextureImportArgs{
                    .filePath = texFilename,
                    .importPath = importPath,
                });
                texture = AssetRegistry::findTexture(importPath, texFilename.stem().string());
            } else if (std::filesystem::exists(meshDirectory / texFilename)) {
                AssetImporter::importTexture(TextureImportArgs{
                    .filePath = meshDirectory / texFilename,
                    .importPath = importPath,
                    .type = type == aiTextureType_NORMALS ? TextureImportType::TEXTURE_NORMAL : TextureImportType::TEXTURE_2D,
                });
                texture = AssetRegistry::findTexture(importPath, texFilename.stem().string());
            } else if (std::filesystem::exists(meshDirectory.parent_path() / "textures" / texFilename)) {
                AssetImporter::importTexture(TextureImportArgs{
                    .filePath = meshDirectory.parent_path() / "textures" / texFilename,
                    .importPath = importPath,
                    .type = type == aiTextureType_NORMALS ? TextureImportType::TEXTURE_NORMAL : TextureImportType::TEXTURE_2D,
                });
                texture = AssetRegistry::findTexture(importPath, texFilename.stem().string());
            } else {
                std::cout << "couldnt find " << texPath.C_Str() << std::endl;
                return;
            }
            expressions.add(new TextureParameter(textureKey, texture, numTextures++));
            parameters.add(textureKey);

            std::string samplerKey = fmt::format("{0}Sampler{1}", paramKey, index);
            SamplerCreateInfo samplerInfo = {};
            switch (mapMode) {
            case aiTextureMapMode_Wrap:
                samplerInfo.addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeW = Gfx::SE_SAMPLER_ADDRESS_MODE_REPEAT;
                break;
            case aiTextureMapMode_Clamp:
                samplerInfo.addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeW = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                break;
            case aiTextureMapMode_Decal:
                samplerInfo.addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                samplerInfo.addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                samplerInfo.addressModeW = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                break;
            case aiTextureMapMode_Mirror:
                samplerInfo.addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                samplerInfo.addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                samplerInfo.addressModeW = Gfx::SE_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                break;
            }
            expressions.add(new SamplerParameter(samplerKey, graphics->createSampler(samplerInfo), numSamplers++));
            parameters.add(samplerKey);

            std::string sampleKey = fmt::format("{0}Sample{1}", paramKey, index);
            expressions.add(new SampleExpression());
            expressions.back()->key = sampleKey;
            expressions.back()->inputs["texture"].source = textureKey;
            expressions.back()->inputs["sampler"].source = samplerKey;
            expressions.back()->inputs["coords"].source = fmt::format("input.texCoords[{0}]", uvIndex);

            std::string colorExtract = fmt::format("{0}Extract{1}", paramKey, index);
            expressions.add(new SwizzleExpression(extractMask));
            expressions.back()->key = colorExtract;
            expressions.back()->inputs["target"].source = sampleKey;

            if (alpha != nullptr) {
                std::string alphaExtract = fmt::format("{0}Alpha{1}", paramKey, index);
                expressions.add(new SwizzleExpression({3, -1, -1, -1}));
                expressions.back()->key = alphaExtract;
                expressions.back()->inputs["target"].source = sampleKey;

                // std::string alphaMul = fmt::format("{0}AlphaMul{1}", paramKey, index);
                // expressions.add(new MulExpression());
                // expressions.back()->key = alphaMul;
                // expressions.back()->inputs["lhs"].source = *alpha;
                // expressions.back()->inputs["rhs"].source = alphaExtract;
                *alpha = alphaExtract;
            }

            if (blend == std::numeric_limits<float>::max()) {
                result = colorExtract;
                return;
            }
            std::string blendFactorKey = fmt::format("{0}BlendFactor{1}", paramKey, index);
            expressions.add(new FloatParameter(blendFactorKey, blend, numFloats++));
            parameters.add(blendFactorKey);

            std::string strengthKey = fmt::format("{0}Strength{1}", paramKey, index);
            expressions.add(new MulExpression());
            expressions.back()->key = strengthKey;
            expressions.back()->inputs["lhs"].source = colorExtract;
            expressions.back()->inputs["rhs"].source = blendFactorKey;

            std::string blendKey = fmt::format("{0}Blend{1}", paramKey, index);
            switch (op) {
                /** T = T1 * T2 */
            case aiTextureOp_Multiply:
                expressions.add(new MulExpression());
                break;

                /** T = T1 - T2 */
            case aiTextureOp_Subtract:
                expressions.add(new SubExpression());
                break;

                /** T = T1 / T2 */
            case aiTextureOp_Divide:
                // expressions[blendKey] = new DivExpression();
                throw std::logic_error("Not implemented");

                /** T = (T1 + T2) - (T1 * T2) */
            case aiTextureOp_SmoothAdd:
                throw std::logic_error("Not implemented");

                /** T = T1 + (T2-0.5) */
            case aiTextureOp_SignedAdd:
                throw std::logic_error("Not implemented");

                /** T = T1 + T2 */
            case aiTextureOp_Add:
            default:
                expressions.add(new AddExpression());
                break;
            }
            expressions.back()->key = blendKey;
            expressions.back()->inputs["lhs"].source = result;
            expressions.back()->inputs["rhs"].source = strengthKey;

            result = blendKey;
        };
        // Diffuse
        addVectorParameter(KEY_DIFFUSE_COLOR, AI_MATKEY_COLOR_DIFFUSE);
        std::string outputDiffuse = KEY_DIFFUSE_COLOR;
        addScalarParameter(KEY_ALPHA, AI_MATKEY_OPACITY);
        std::string outputAlpha = KEY_ALPHA;
        uint32 numDiffuseTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
        for (uint32 i = 0; i < numDiffuseTextures; ++i) {
            addTextureParameter(KEY_DIFFUSE_TEXTURE, aiTextureType_DIFFUSE, i, outputDiffuse, {0, 1, 2, -1}, &outputAlpha);
        }

        // Specular
        addVectorParameter(KEY_SPECULAR_COLOR, AI_MATKEY_COLOR_SPECULAR);
        std::string outputSpecular = KEY_SPECULAR_COLOR;
        uint32 numSpecular = material->GetTextureCount(aiTextureType_SPECULAR);
        for (uint32 i = 0; i < numSpecular; ++i) {
            addTextureParameter(KEY_SPECULAR_TEXTURE, aiTextureType_SPECULAR, i, outputSpecular);
        }

        // Normal
        std::string outputNormal = "";
        uint32 numNormal = material->GetTextureCount(aiTextureType_NORMALS);
        for (uint32 i = 0; i < numNormal; ++i) {
            addTextureParameter(KEY_NORMAL_TEXTURE, aiTextureType_NORMALS, i, outputNormal);
        }

        // Ambient Color
        addVectorParameter(KEY_AMBIENT_COLOR, AI_MATKEY_COLOR_AMBIENT);
        std::string outputAmbient = KEY_AMBIENT_COLOR;
        uint32 numAmbient = material->GetTextureCount(aiTextureType_AMBIENT);
        for (uint32 i = 0; i < numAmbient; ++i) {
            addTextureParameter(KEY_AMBIENT_TEXTURE, aiTextureType_AMBIENT, i, outputAmbient);
        }

        // Shininess
        addScalarParameter(KEY_SHININESS, AI_MATKEY_SHININESS);
        std::string outputShininess = KEY_SHININESS;
        uint32 numShiny = material->GetTextureCount(aiTextureType_SHININESS);
        for (uint32 i = 0; i < numShiny; ++i) {
            addTextureParameter(KEY_SHININESS_TEXTURE, aiTextureType_SHININESS, i, outputShininess, {0, -1, -1, -1});
        }

        // Roughness
        addScalarParameter(KEY_ROUGHNESS, AI_MATKEY_ROUGHNESS_FACTOR);
        std::string outputRoughness = KEY_ROUGHNESS;
        uint32 numRoughness = material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);
        for (uint32 i = 0; i < numRoughness; ++i) {
            addTextureParameter(KEY_ROUGHNESS_TEXTURE, aiTextureType_DIFFUSE_ROUGHNESS, i, outputRoughness, {0, -1, -1, -1});
        }

        // Metallic
        addScalarParameter(KEY_METALLIC, AI_MATKEY_METALLIC_FACTOR);
        std::string outputMetallic = KEY_METALLIC;
        uint32 numMetallic = material->GetTextureCount(aiTextureType_METALNESS);
        for (uint32 i = 0; i < numMetallic; ++i) {
            addTextureParameter(KEY_METALLIC_TEXTURE, aiTextureType_METALNESS, i, outputMetallic, {0, -1, -1, -1});
        }

        // Ambient Occlusion
        std::string outputAO = "";
        uint32 numAO = material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION);
        for (uint32 i = 0; i < numAO; ++i) {
            addTextureParameter(KEY_AMBIENT_OCCLUSION_TEXTURE, aiTextureType_AMBIENT_OCCLUSION, i, outputAO, {0, -1, -1, -1});
        }

        MaterialNode brdf;
        brdf.variables["baseColor"] = outputDiffuse;
        brdf.variables["alpha"] = outputAlpha;
        if (!outputNormal.empty()) {
            expressions.add(new MulExpression());
            expressions.back()->key = "NormalMul";
            expressions.back()->inputs["lhs"].source = "2";
            expressions.back()->inputs["rhs"].source = outputNormal;

            expressions.add(new SubExpression());
            expressions.back()->key = "NormalSub";
            expressions.back()->inputs["lhs"].source = "NormalMul";
            expressions.back()->inputs["rhs"].source = "float3(1,1,1)";

            brdf.variables["normal"] = "NormalSub";
        }
        aiShadingMode mode;
        material->Get(AI_MATKEY_SHADING_MODEL, mode);
        switch (mode) {
        case aiShadingMode_Blinn:
            brdf.profile = "BlinnPhong";
            brdf.variables["specularColor"] = outputSpecular;
            brdf.variables["ambient"] = outputAmbient;
            brdf.variables["shininess"] = outputShininess;
            break;
        case aiShadingMode_Phong:
            brdf.profile = "Phong";
            brdf.variables["specular"] = outputSpecular;
            brdf.variables["ambient"] = outputAmbient;
            brdf.variables["shininess"] = outputShininess;
            break;
        case aiShadingMode_Toon:
            brdf.profile = "CelShading";
            break;
        default:
        case aiShadingMode_CookTorrance:
            brdf.profile = "CookTorrance";
            brdf.variables["roughness"] = outputRoughness;
            brdf.variables["metallic"] = outputMetallic;
            if (!outputAO.empty()) {
                brdf.variables["ambientOcclusion"] = outputAmbient;
            }
            break;
        };
        uint32 twoSided = false;
        float opacity = 1.0f;
        aiString matName = material->GetName();
        const char* mat = matName.C_Str();

        if (strcmp(mat, "Leaves0119_14_S") == 0) {
            opacity = 0.5f;
            twoSided = true;
        }
        if (strcmp(mat, "TexturesCom_Leaves0119_1_alphamasked_S") == 0) {
            opacity = 0.5f;
            twoSided = true;
        }
        if (strcmp(mat, "TexturesCom_Leaves0119_2_alphamasked_S") == 0) {
            opacity = 0.5f;
            twoSided = true;
        }
        if (strcmp(mat, "3td_Africa_Grass01") == 0) {
            opacity = 0.5f;
            twoSided = true;
        }
        if (strcmp(mat, "DryWeeds-CC0") == 0) {
            opacity = 0.5f;
            twoSided = true;
        }
        if (strcmp(mat, "fgrass1_v2_256") == 0) {
            opacity = 0.5f;
            twoSided = true;
        }
        if (strcmp(mat, "arbre-feuille") == 0) {
            opacity = 0.5f;
            twoSided = true;
        }
        if (strcmp(mat, "arbre-feuille-variante") == 0) {
            opacity = 0.5f;
            twoSided = true;
        }
        if (strcmp(mat, "arbre-feuille-variante2") == 0) {
            opacity = 0.5f;
            twoSided = true;
        }
        OMaterialAsset baseMat = new MaterialAsset(importPath, materialName);
        baseMat->material = new Material(graphics, numTextures, numSamplers, numFloats, twoSided, opacity, materialName,
                                         std::move(expressions), std::move(parameters), std::move(brdf));
        baseMat->material->compile();
        graphics->getShaderCompiler()->registerMaterial(baseMat->material);
        globalMaterials[m] = baseMat->instantiate(InstantiationParameter{
            .name = fmt::format("{0}_Inst_0", baseMat->getName()),
            .folderPath = baseMat->getFolderPath(),
        });
        AssetRegistry::get().saveAsset(PMaterialAsset(baseMat), MaterialAsset::IDENTIFIER, baseMat->getFolderPath(), baseMat->getName());
        AssetRegistry::get().registerMaterial(std::move(baseMat));
    }
}

void MeshLoader::findMeshRoots(aiNode* node, List<aiNode*>& meshNodes) {
    if (node->mNumMeshes > 0) {
        meshNodes.add(node);
        return;
    }
    for (uint32 i = 0; i < node->mNumChildren; ++i) {
        findMeshRoots(node->mChildren[i], meshNodes);
    }
}

uint32 MeshLoader::encodeQTangent(Matrix3 m) {
    float r = (glm::determinant(m) ? -1.0 : 1.0);
    m[2] *= r;
    float t = m[0][0] + (m[1][1] + m[2][2]);
    Vector4 q;
    if (t > 2.9999999) {
        q = Vector4(0.0, 0.0, 0.0, 1.0);
    } else if (t > 0.0000001) {
        float s = sqrt(1.0 + t) * 2.0;
        q = Vector4(Vector(m[1][2] - m[2][1], m[2][0] - m[0][2], m[0][1] - m[1][0]) / s, s * 0.25);
    } else if ((m[0][0] > m[1][1]) && (m[0][0] > m[2][2])) {
        float s = sqrt(1.0 + (m[0][0] - (m[1][1] + m[2][2]))) * 2.0;
        q = Vector4(s * 0.25, Vector(m[1][0] + m[0][1], m[2][0] + m[0][2], m[1][2] - m[2][1]) / s);
    } else if (m[1][1] > m[2][2]) {
        float s = sqrt(1.0 + (m[1][1] - (m[0][0] + m[2][2]))) * 2.0;
        q = Vector4(Vector(m[1][0] + m[0][1], m[2][1] + m[1][2], m[2][0] - m[0][2]) / s, s * 0.25);
        q = Vector4(q.x, q.w, q.y, q.z);
    } else {
        float s = sqrt(1.0 + (m[2][2] - (m[0][0] + m[1][1]))) * 2.0;
        q = Vector4(Vector(m[2][0] + m[0][2], m[2][1] + m[1][2], m[0][1] - m[1][0]) / s, s * 0.25);
        q = Vector4(q.x, q.y, q.w, q.z);
    }
    Vector4 qAbs = abs(q = glm::normalize(q));
    int maxComponentIndex = (qAbs.x > qAbs.y) ? ((qAbs.x > qAbs.z) ? ((qAbs.x > qAbs.w) ? 0 : 3) : ((qAbs.z > qAbs.w) ? 2 : 3))
                                              : ((qAbs.y > qAbs.z) ? ((qAbs.y > qAbs.w) ? 1 : 3) : ((qAbs.z > qAbs.w) ? 2 : 3));
    Vector components[4] = {Vector(q.y, q.z, q.w), Vector(q.x, q.z, q.w), Vector(q.x, q.y, q.w), Vector(q.x, q.y, q.z)};

    q = Vector4(components[maxComponentIndex] * float(((q[maxComponentIndex] < 0.0) ? -1.0 : 1.0) * 1.4142135623730951), q.w);
    return ((uint32(round(glm::clamp(q.x * 511.0, -511.0, 511.0) + 512.0)) & 0x3ffu) << 0u) |
           ((uint32(round(glm::clamp(q.y * 511.0, -511.0, 511.0) + 512.0)) & 0x3ffu) << 10u) |
           ((uint32(round(glm::clamp(q.z * 255.0, -255.0, 255.0) + 256.0)) & 0x1ffu) << 20u) |
           ((uint32(((dot(cross(m[0], m[2]), m[1]) * r) < 0.0) ? 1u : 0u) & 0x1u) << 29u) | ((uint32(maxComponentIndex) & 0x3u) << 30u);
}

void MeshLoader::loadGlobalMeshes(const aiScene* scene, const Array<PMaterialInstanceAsset>& materials, Array<OMesh>& globalMeshes,
                                  Component::Collider& collider) {
    List<std::function<void()>> work;
    for (int32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        if (!(mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE))
            continue;
        globalMeshes[meshIndex] = new Mesh();

        StaticMeshVertexData* vertexData = StaticMeshVertexData::getInstance();
        MeshId id = vertexData->allocateVertexData(mesh->mNumVertices);
        uint64 offset = vertexData->getMeshOffset(id);
        collider.boundingbox.adjust(Vector(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z));
        collider.boundingbox.adjust(Vector(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z));
        work.add([=, this, &globalMeshes]() {
            // assume static mesh for now
            Array<Vector> positions(mesh->mNumVertices);
            StaticArray<Array<U16Vector2>, MAX_TEXCOORDS> texCoords;
            for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
                texCoords[i].resize(mesh->mNumVertices);
            }
            Array<uint32> normals(mesh->mNumVertices);
            Array<U16Vector> colors(mesh->mNumVertices);

            for (int32 i = 0; i < mesh->mNumVertices; ++i) {
                positions[i] = Vector(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
                for (size_t j = 0; j < MAX_TEXCOORDS; ++j) {
                    if (mesh->HasTextureCoords(j)) {
                        texCoords[j][i] = U16Vector2(mesh->mTextureCoords[j][i].x * 65535, mesh->mTextureCoords[j][i].y * 65535);
                    } else {
                        texCoords[j][i] = U16Vector2(0, 0);
                    }
                }
                Vector normal = Vector(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
                Vector tangent = Vector(1, 0, 0);
                Vector biTangent = Vector(0, 0, 1);
                if (mesh->HasTangentsAndBitangents()) {
                    tangent = Vector(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
                    biTangent = Vector(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
                }
                Matrix3 tbn = {normal, biTangent, tangent};

                normals[i] = encodeQTangent(tbn);

                if (mesh->HasVertexColors(0)) {
                    colors[i] = U16Vector(mesh->mColors[0][i].r * 65535, mesh->mColors[0][i].g * 65535, mesh->mColors[0][i].b * 65535);
                } else {
                    colors[i] = U16Vector(1, 1, 1);
                }
            }
            vertexData->loadPositions(offset, positions);

            for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
                vertexData->loadTexCoords(offset, i, texCoords[i]);
            }
            vertexData->loadNormals(offset, normals);
            vertexData->loadColors(offset, colors);

            Array<uint32> indices(mesh->mNumFaces * 3);
            for (uint32 faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
                indices[faceIndex * 3 + 0] = mesh->mFaces[faceIndex].mIndices[0];
                indices[faceIndex * 3 + 1] = mesh->mFaces[faceIndex].mIndices[1];
                indices[faceIndex * 3 + 2] = mesh->mFaces[faceIndex].mIndices[2];
            }

            Array<Meshlet> meshlets;
            meshlets.reserve(indices.size() / (3ull * Gfx::numPrimitivesPerMeshlet));
            Meshlet::build(positions, indices, meshlets);
            vertexData->loadMesh(id, indices, meshlets);

            // collider.physicsMesh.addCollider(positions, indices, Matrix4(1.0f));

            globalMeshes[meshIndex]->vertexData = vertexData;
            globalMeshes[meshIndex]->id = id;
            globalMeshes[meshIndex]->referencedMaterial = materials[mesh->mMaterialIndex];
            globalMeshes[meshIndex]->vertexCount = mesh->mNumVertices;
            globalMeshes[meshIndex]->blas = graphics->createBottomLevelAccelerationStructure(Gfx::BottomLevelASCreateInfo{
                .mesh = globalMeshes[meshIndex],
            });
            // vertexData->registerBottomLevelAccelerationStructure(globalMeshes[meshIndex]->blas);
        });
    }
    getThreadPool().runAndWait(std::move(work));
}

Matrix4 convertMatrix(aiMatrix4x4 matrix) {
    return Matrix4(matrix.a1, matrix.b1, matrix.c1, matrix.d1, matrix.a2, matrix.b2, matrix.c2, matrix.d2, matrix.a3, matrix.b3, matrix.c3,
                   matrix.d3, matrix.a4, matrix.b4, matrix.c4, matrix.d4);
}

aiMatrix4x4 loadNodeTransform(aiNode* node) {
    aiMatrix4x4 parent = aiMatrix4x4();
    if (node->mParent != nullptr) {
        parent = loadNodeTransform(node->mParent);
    }
    return node->mTransformation * parent;
}

void MeshLoader::import(MeshImportArgs args, PMeshAsset meshAsset) {
    std::cout << "Starting to import " << args.filePath << std::endl;
    meshAsset->setStatus(Asset::Status::Loading);
    Assimp::Importer importer;
    importer.ReadFile(args.filePath.string().c_str(),
                      (uint32)(aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenBoundingBoxes |
                               aiProcess_GenUVCoords | aiProcess_FindDegenerates));
    const aiScene* scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
    std::cout << importer.GetErrorString() << std::endl;

    Array<PTextureAsset> textures;
    loadTextures(scene, args.filePath.parent_path(), args.importPath, textures);
    Array<PMaterialInstanceAsset> globalMaterials(scene->mNumMaterials);
    loadMaterials(scene, textures, args.filePath.stem().string(), args.filePath.parent_path(), args.importPath, globalMaterials);

    Array<OMesh> globalMeshes(scene->mNumMeshes);
    Component::Collider collider;
    loadGlobalMeshes(scene, globalMaterials, globalMeshes, collider);

    List<aiNode*> meshNodes;
    findMeshRoots(scene->mRootNode, meshNodes);

    Array<OMesh> meshes;
    for (auto meshNode : meshNodes) {
        for (uint32 i = 0; i < meshNode->mNumMeshes; ++i) {
            if (globalMeshes[meshNode->mMeshes[i]] == nullptr) {
                continue;
            }
            meshes.add(std::move(globalMeshes[meshNode->mMeshes[i]]));
            meshes.back()->transform = convertMatrix(loadNodeTransform(meshNode));
        }
    }
    meshAsset->meshes = std::move(meshes);
    meshAsset->physicsMesh = std::move(collider);

    AssetRegistry::saveAsset(meshAsset, MeshAsset::IDENTIFIER, meshAsset->getFolderPath(), meshAsset->getName());

    meshAsset->setStatus(Asset::Status::Ready);
}
