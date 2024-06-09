#include "MaterialLoader.h"
#include "Asset/AssetRegistry.h"
#include "Asset/MaterialAsset.h"
#include "Asset/TextureAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include "Material/Material.h"
#include "Material/ShaderExpression.h"
#include "Window/WindowManager.h"
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>


using namespace Seele;
using json = nlohmann::json;

MaterialLoader::MaterialLoader(Gfx::PGraphics graphics) : graphics(graphics) {
    OMaterialAsset placeholderAsset = new MaterialAsset();
    import(
        MaterialImportArgs{
            .filePath = std::filesystem::absolute("./shaders/Placeholder.json"),
            .importPath = "",
        },
        placeholderAsset);
    AssetRegistry::get().assetRoot->materials[""] = std::move(placeholderAsset);
}

MaterialLoader::~MaterialLoader() {}

void MaterialLoader::importAsset(MaterialImportArgs args) {
    std::filesystem::path assetPath = args.filePath.filename();
    assetPath.replace_extension("asset");
    OMaterialAsset asset = new MaterialAsset(args.importPath, assetPath.stem().string());
    asset->setStatus(Asset::Status::Loading);
    PMaterialAsset ref = asset;
    AssetRegistry::get().registerMaterial(std::move(asset));
    import(args, ref);
}

void MaterialLoader::import(MaterialImportArgs args, PMaterialAsset asset) {
    auto jsonstream = std::ifstream(args.filePath.c_str());
    json j;
    jsonstream >> j;
    std::string materialName = j["name"].get<std::string>() + "Material";
    Gfx::ODescriptorLayout layout = graphics->createDescriptorLayout("pMaterial");
    // Shader file needs to conform to the slang standard, which prohibits _
    materialName.erase(std::remove(materialName.begin(), materialName.end(), '_'), materialName.end());
    materialName.erase(std::remove(materialName.begin(), materialName.end(), '-'), materialName.end());

    uint32 uniformBufferOffset = 0;
    uint32 bindingCounter = 0; // Uniform buffers are always binding 0
    int32 uniformBinding = -1;
    Array<OShaderExpression> expressions;
    uint32 key = 0;
    uint32 auxKey = 0;
    Array<std::string> parameters;
    for (auto& param : j["params"].items()) {
        std::string type = param.value()["type"].get<std::string>();
        auto defaultValue = param.value().find("default");
        // TODO: ALIGNMENT RULES
        if (type.compare("float") == 0) {
            float defaultData = 0.f;
            if (defaultValue != param.value().end()) {
                defaultData = std::stof(defaultValue.value().get<std::string>());
            }
            OFloatParameter p = new FloatParameter(param.key(), defaultData, uniformBufferOffset, uniformBinding);
            if (uniformBinding == -1) {
                layout->addDescriptorBinding(Gfx::DescriptorBinding{
                    .binding = bindingCounter,
                    .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                });
                uniformBinding = bindingCounter++;
            }
            uniformBufferOffset += 4;
            parameters.add(p->key);
            expressions.add(std::move(p));
        }
        // TODO: ALIGNMENT RULES
        else if (type.compare("float3") == 0) {
            Vector defaultData = Vector(0, 0, 0);
            if (defaultValue != param.value().end()) {
                defaultData = parseVector(defaultValue.value().get<std::string>().c_str());
            }
            OVectorParameter p = new VectorParameter(param.key(), defaultData, uniformBufferOffset, uniformBinding);
            if (uniformBinding == -1) {
                layout->addDescriptorBinding(Gfx::DescriptorBinding{
                    .binding = bindingCounter,
                    .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                });
                uniformBinding = bindingCounter++;
            }
            uniformBufferOffset += 16;
            parameters.add(p->key);
            expressions.add(std::move(p));
        } else if (type.compare("Texture2D") == 0) {
            PTextureAsset texture;
            if (defaultValue != param.value().end()) {
                std::string defaultString = defaultValue.value().get<std::string>();
                auto slashPos = defaultString.rfind("/");
                std::string folder = "";
                if (slashPos != std::string::npos) {
                    folder = defaultString.substr(0, slashPos - 1);
                    defaultString = defaultString.substr(slashPos, defaultString.length());
                }

                texture = AssetRegistry::findTexture(folder, defaultString);
            }
            if (texture == nullptr) {
                texture = AssetRegistry::findTexture("", ""); // this will return placeholder texture
            }
            OTextureParameter p = new TextureParameter(param.key(), texture, bindingCounter);
            layout->addDescriptorBinding(Gfx::DescriptorBinding{
                .binding = bindingCounter++,
                .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            });
            parameters.add(p->key);
            expressions.add(std::move(p));
        } else if (type.compare("Sampler") == 0) {
            OSamplerParameter p = new SamplerParameter(param.key(), graphics->createSampler({}), bindingCounter);
            layout->addDescriptorBinding(Gfx::DescriptorBinding{
                .binding = bindingCounter++,
                .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
            });
            parameters.add(p->key);
            expressions.add(std::move(p));
        } else if (type.compare("Sampler2D") == 0) {
            PTextureAsset texture;
            if (defaultValue != param.value().end()) {
                std::string defaultString = defaultValue.value().get<std::string>();
                auto slashPos = defaultString.rfind("/");
                std::string folder = "";
                if (slashPos != std::string::npos) {
                    folder = defaultString.substr(0, slashPos - 1);
                    defaultString = defaultString.substr(slashPos, defaultString.length());
                }

                texture = AssetRegistry::findTexture(folder, defaultString);
            }
            if (texture == nullptr) {
                texture = AssetRegistry::findTexture("", ""); // this will return placeholder texture
            }
            OCombinedTextureParameter p = new CombinedTextureParameter(param.key(), texture, graphics->createSampler({}), bindingCounter);
            layout->addDescriptorBinding(Gfx::DescriptorBinding{
                .binding = bindingCounter++,
                .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
            });
            parameters.add(p->key);
            expressions.add(std::move(p));
        } else {
            std::cout << "Error unsupported parameter type" << std::endl;
        }
    }
    uint32 uniformDataSize = uniformBufferOffset;
    auto referenceExpression = [&parameters, &auxKey, &expressions](json obj) -> std::string {
        if (obj.is_string()) {
            std::string str = obj.get<std::string>();
            if (parameters.find(str) != parameters.end()) {
                return str;
            }
            OConstantExpression c = new ConstantExpression(str, ExpressionType::UNKNOWN);
            std::string name = fmt::format("const_{0}", auxKey++);
            c->key = name;
            expressions.add(std::move(c));
            return name;
        } else {
            return fmt::format("{0}", obj.get<uint32>());
        }
    };
    MaterialNode mat;

    for (auto& param : j["code"].items()) {
        auto& obj = param.value();
        std::string exp = obj["exp"].get<std::string>();
        if (exp.compare("Const") == 0) {
            OConstantExpression p = new ConstantExpression();
            std::string name = fmt::format("{0}", key++);
            p->key = name;
            p->expr = obj["value"];
            expressions.add(std::move(p));
        }
        if (exp.compare("Add") == 0) {
            OAddExpression p = new AddExpression();
            std::string name = fmt::format("{0}", key++);
            p->key = name;
            p->inputs["lhs"].source = referenceExpression(obj["lhs"]);
            p->inputs["rhs"].source = referenceExpression(obj["rhs"]);
            expressions.add(std::move(p));
        }
        if (exp.compare("Sub") == 0) {
            OSubExpression p = new SubExpression();
            std::string name = fmt::format("{0}", key++);
            p->key = name;
            p->inputs["lhs"].source = referenceExpression(obj["lhs"]);
            p->inputs["rhs"].source = referenceExpression(obj["rhs"]);
            expressions.add(std::move(p));
        }
        if (exp.compare("Mul") == 0) {
            OMulExpression p = new MulExpression();
            std::string name = fmt::format("{0}", key++);
            p->key = name;
            p->inputs["lhs"].source = referenceExpression(obj["lhs"]);
            p->inputs["rhs"].source = referenceExpression(obj["rhs"]);
            expressions.add(std::move(p));
        }
        if (exp.compare("Swizzle") == 0) {
            OSwizzleExpression p = new SwizzleExpression();
            std::string name = fmt::format("{0}", key++);
            p->key = name;
            p->inputs["target"].source = referenceExpression(obj["target"]);
            int32 i = 0;
            for (auto& c : obj["comp"].items()) {
                p->comp[i++] = c.value().get<uint32>();
            }
            expressions.add(std::move(p));
        }
        if (exp.compare("Sample") == 0) {
            OSampleExpression p = new SampleExpression();
            std::string name = fmt::format("{0}", key++);
            p->key = name;
            if (obj.contains("texture")) {
                p->inputs["texture"].source = referenceExpression(obj["texture"]);
            }
            p->inputs["sampler"].source = referenceExpression(obj["sampler"]);
            p->inputs["coords"].source = referenceExpression(obj["coords"]);
            expressions.add(std::move(p));
        }
        if (exp.compare("BRDF") == 0) {
            mat.profile = obj["profile"].get<std::string>();
            for (auto& val : obj["values"].items()) {
                mat.variables[val.key()] = referenceExpression(val.value());
            }
        }
    }
    layout->create();
    asset->material = new Material(graphics, std::move(layout), uniformDataSize, uniformBinding, materialName, std::move(expressions),
                                   std::move(parameters), std::move(mat));

    asset->material->compile();
    graphics->getShaderCompiler()->registerMaterial(asset->material);
    asset->setStatus(Asset::Status::Ready);

    if (asset->getName().empty()) {
        return;
    }

    auto stream = AssetRegistry::createWriteStream((std::filesystem::path(asset->folderPath) / asset->getName()).string().append(".asset"),
                                                   std::ios::binary);

    ArchiveBuffer archive;
    Serialization::save(archive, MaterialAsset::IDENTIFIER);
    Serialization::save(archive, asset->getName());
    Serialization::save(archive, asset->getFolderPath());
    asset->save(archive);
    archive.writeToStream(stream);
    ////co_return;
}

PMaterialAsset MaterialLoader::getPlaceHolderMaterial() { return placeholderMaterial; }
