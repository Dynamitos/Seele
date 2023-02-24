#include "MaterialLoader.h"
#include "Graphics/Graphics.h"
#include "Asset/MaterialAsset.h"
#include "Asset/AssetRegistry.h"
#include "Material/Material.h"
#include "Window/WindowManager.h"
#include "Material/ShaderExpression.h"
#include "Asset/TextureAsset.h"
#include <nlohmann/json.hpp>

using namespace Seele;
using json = nlohmann::json;

MaterialLoader::MaterialLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    PMaterialAsset placeholderAsset = new MaterialAsset();
    import(MaterialImportArgs{
        .filePath = std::filesystem::absolute("./shaders/Placeholder.asset"),
        .importPath = "",
        }, placeholderAsset);
    AssetRegistry::get().assetRoot->materials[""] = placeholderAsset;
}

MaterialLoader::~MaterialLoader()
{
}

void MaterialLoader::importAsset(MaterialImportArgs args)
{
    std::filesystem::path assetPath = args.filePath.filename();
    assetPath.replace_extension("asset");
    PMaterialAsset asset = new MaterialAsset(args.importPath, assetPath.stem().string());
    asset->setStatus(Asset::Status::Loading);
    AssetRegistry::get().registerMaterial(asset);
    import(args, asset);
}

void MaterialLoader::import(MaterialImportArgs args, PMaterialAsset asset)
{
    auto stream = std::ifstream(args.filePath.c_str());
    json j;
    stream >> j;
    std::string materialName = j["name"].get<std::string>() + "Material";
    Gfx::PDescriptorLayout layout = graphics->createDescriptorLayout(materialName + "Layout");
    //Shader file needs to conform to the slang standard, which prohibits _
    materialName.erase(std::remove(materialName.begin(), materialName.end(), '_'), materialName.end());
    materialName.erase(std::remove(materialName.begin(), materialName.end(), '.'), materialName.end());
    
    uint32 uniformBufferOffset = 0;
    uint32 bindingCounter = 0; // Uniform buffers are always binding 0
    uint32 uniformBinding = -1;
    Map<int32, PShaderExpression> expressions;
    int32 key = 0;
    int32 auxKey = -1;
    Map<std::string, PShaderParameter> parameters;
    for(auto param : j["params"].items())
    {
        std::string type = param.value()["type"].get<std::string>();
        auto defaultValue = param.value().find("default");
        // TODO: ALIGNMENT RULES
        if(type.compare("float") == 0)
        {
            PFloatParameter p = new FloatParameter(param.key(), uniformBufferOffset, 0);
            p->key = auxKey;
            if(uniformBinding == -1)
            {
                layout->addDescriptorBinding(bindingCounter, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
                uniformBinding = bindingCounter++;
            }
            uniformBufferOffset += 4;
            if(defaultValue != param.value().end())
            {
                p->data = std::stof(defaultValue.value().get<std::string>());
            }
            expressions[auxKey--] = p;
            parameters[param.key()] = p;
        }
        // TODO: ALIGNMENT RULES
        else if(type.compare("float3") == 0)
        {
            PVectorParameter p = new VectorParameter(param.key(), uniformBufferOffset, 0);
            p->key = auxKey;
            if(uniformBinding == -1)
            {
                layout->addDescriptorBinding(bindingCounter, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
                uniformBinding = bindingCounter++;
            }
            uniformBufferOffset += 12;
            if(defaultValue != param.value().end())
            {
                p->data = parseVector(defaultValue.value().get<std::string>().c_str());
            }
            expressions[auxKey--] = p;
            parameters[param.key()] = p;
        }
        else if(type.compare("Texture2D") == 0)
        {
            PTextureParameter p = new TextureParameter(param.key(), 0, bindingCounter);
            p->key = auxKey;
            layout->addDescriptorBinding(bindingCounter++, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
            if(defaultValue != param.value().end())
            {
                std::string defaultString = defaultValue.value().get<std::string>();
                p->data = AssetRegistry::findTexture(defaultString);
            }
            if(p->data == nullptr)
            {
                p->data = AssetRegistry::findTexture(""); // this will return placeholder texture
            }
            expressions[auxKey--] = p;
            parameters[param.key()] = p;
        }
        else if(type.compare("SamplerState") == 0)
        {
            PSamplerParameter p = new SamplerParameter(param.key(), 0, bindingCounter);
            p->key = auxKey;
            layout->addDescriptorBinding(bindingCounter++, Gfx::SE_DESCRIPTOR_TYPE_SAMPLER);
            p->data = graphics->createSamplerState({});
            expressions[auxKey--] = p;
            parameters[param.key()] = p;
        }
        else
        {
            std::cout << "Error unsupported parameter type" << std::endl;
        }
    }
    uint32 uniformDataSize = uniformBufferOffset;
    auto referenceExpression = [&auxKey, &expressions](json obj) -> PShaderExpression
    {
        if(obj.is_string())
        {
            PConstantExpression c = new ConstantExpression(obj.get<std::string>(), ExpressionType::UNKNOWN);
            c->key = auxKey;
            expressions[auxKey--] = c;
            return c;
        }
        else
        {
            return expressions[obj.get<uint32>()];
        }
    };
    MaterialNode mat;

    for(auto param : j["code"].items())
    {
        auto obj = param.value();
        std::string exp = obj["exp"].get<std::string>();
        if(exp.compare("Add") == 0)
        {
            PAddExpression p = new AddExpression();
            p->key = key;
            p->inputs["lhs"].source = referenceExpression(obj["lhs"])->key;
            p->inputs["rhs"].source = referenceExpression(obj["rhs"])->key;
            expressions[key++] = p;
        }
        if(exp.compare("Sub") == 0)
        {
            PSubExpression p = new SubExpression();
            p->key = key;
            p->inputs["lhs"].source = referenceExpression(obj["lhs"])->key;
            p->inputs["rhs"].source = referenceExpression(obj["rhs"])->key;
            expressions[key++] = p;
        }
        if(exp.compare("Mul") == 0)
        {
            PMulExpression p = new MulExpression();
            p->key = key;
            p->inputs["lhs"].source = referenceExpression(obj["lhs"])->key;
            p->inputs["rhs"].source = referenceExpression(obj["rhs"])->key;
            expressions[key++] = p;
        }
        if(exp.compare("Swizzle") == 0)
        {
            PSwizzleExpression p = new SwizzleExpression();
            p->key = key;
            p->inputs["target"].source = referenceExpression(obj["target"])->key;
            int32 i = 0;
            for(auto c : obj["comp"].items())
            {
                p->comp[i++] = c.value().get<uint32>();
            }
            expressions[key++] = p;
        }
        if(exp.compare("Sample") == 0)
        {
            PSampleExpression p = new SampleExpression();
            p->key = key;
            p->inputs["texture"].source = parameters[obj["texture"].get<std::string>()]->key;
            p->inputs["sampler"].source = parameters[obj["sampler"].get<std::string>()]->key;
            p->inputs["coords"].source = referenceExpression(obj["coords"])->key;
            expressions[key++] = p;
        }
        if(exp.compare("BRDF") == 0)
        {
            mat.profile = obj["profile"].get<std::string>();
            for(auto val : obj["values"].items())
            {
                mat.variables[val.key()] = referenceExpression(val.value());
            }
        }
    }
    layout->create();
    Array<PShaderExpression> codeExp;
    for(const auto& [_, e] : expressions)
    {
        codeExp.add(e);
    }
    Array<PShaderParameter> params;
    for(const auto& [_, p] : parameters)
    {
        params.add(p);
    }
    asset->material = new Material(
        graphics,
        std::move(params),
        std::move(layout),
        uniformDataSize,
        uniformBinding,
        materialName,
        std::move(codeExp),
        std::move(mat)
    );
    asset->material->compile();
    graphics->getShaderCompiler()->registerMaterial(asset->material);
    asset->setStatus(Asset::Status::Ready);
    ////co_return;
}

PMaterialAsset MaterialLoader::getPlaceHolderMaterial() 
{
    return placeholderMaterial;
}
