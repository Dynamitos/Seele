#include "MaterialLoader.h"
#include "Graphics/Graphics.h"
#include "MaterialAsset.h"
#include "AssetRegistry.h"
#include "Material/Material.h"
#include "Material/BRDF.h"
#include "Window/WindowManager.h"
#include "Material/ShaderExpression.h"
#include "TextureAsset.h"
#include <nlohmann/json.hpp>

using namespace Seele;
using json = nlohmann::json;

MaterialLoader::MaterialLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    importAsset(std::filesystem::absolute("./shaders/Placeholder.asset"), "");
}

MaterialLoader::~MaterialLoader()
{
}

void MaterialLoader::importAsset(const std::filesystem::path& name, const std::string& importPath)
{
    std::filesystem::path assetPath = name.filename();
    assetPath.replace_extension("asset");
    PMaterialAsset asset = new MaterialAsset(assetPath.generic_string());
    asset->setStatus(Asset::Status::Loading);
    AssetRegistry::get().registerMaterial(asset, importPath);
    import(name, asset);
}

void MaterialLoader::import(std::filesystem::path name, PMaterialAsset asset)
{
    auto stream = std::ifstream(name.c_str());
    json j;
    stream >> j;
    std::string materialName = j["name"].get<std::string>() + "Material";
    Gfx::PDescriptorLayout layout = graphics->createDescriptorLayout(materialName + "Layout");
    //Shader file needs to conform to the slang standard, which prohibits _
    materialName.erase(std::remove(materialName.begin(), materialName.end(), '_'), materialName.end());
    materialName.erase(std::remove(materialName.begin(), materialName.end(), '.'), materialName.end());
    std::ofstream codeStream("./shaders/generated/"+materialName+".slang");
    std::string profile = j["profile"].get<std::string>();

    codeStream << "import Material;" << std::endl;
    codeStream << "import BRDF;" << std::endl;
    codeStream << "import MaterialParameter;" << std::endl << std::endl;

    codeStream << "struct " << materialName << " : IMaterial {" << std::endl;
    uint32 uniformBufferOffset = 0;
    uint32 bindingCounter = 0; // Uniform buffers are always binding 0
    uint32 uniformBinding = -1;
    Array<PShaderParameter> parameters;
    for(auto param : j["params"].items())
    {
        std::string type = param.value()["type"].get<std::string>();
        auto defaultValue = param.value().find("default");
        // TODO: ALIGNMENT RULES
        if(type.compare("float") == 0)
        {
            PFloatParameter p = new FloatParameter(param.key(), uniformBufferOffset, 0);
            codeStream << "\tlayout(offset = " << uniformBufferOffset << ")";
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
            parameters.add(p);
        }
        // TODO: ALIGNMENT RULES
        else if(type.compare("float3") == 0)
        {
            PVectorParameter p = new VectorParameter(param.key(), uniformBufferOffset, 0);
            codeStream << "\tlayout(offset = " << uniformBufferOffset << ")";
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
            parameters.add(p);
        }
        else if(type.compare("Texture2D") == 0)
        {
            PTextureParameter p = new TextureParameter(param.key(), 0, bindingCounter);
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
            parameters.add(p);
        }
        else if(type.compare("SamplerState") == 0)
        {
            PSamplerParameter p = new SamplerParameter(param.key(), 0, bindingCounter);
            layout->addDescriptorBinding(bindingCounter++, Gfx::SE_DESCRIPTOR_TYPE_SAMPLER);
            p->data = graphics->createSamplerState({});
            parameters.add(p);
        }
        else
        {
            std::cout << "Error unsupported parameter type" << std::endl;
        }
        codeStream << "\t" << type << " " << param.key() << ";\n";
    }
    uint32 uniformDataSize = uniformBufferOffset;
    
    BRDF* brdf = BRDF::getBRDFByName(profile);
    brdf->generateMaterialCode(codeStream, j["code"]);
    codeStream << "};";
    codeStream.close();
    layout->create();
    asset->material = new Material(
        graphics,
        std::move(parameters),
        std::move(layout),
        uniformDataSize,
        uniformBinding,
        materialName
    );
    graphics->getShaderCompiler()->registerMaterial(asset->material);
    asset->setStatus(Asset::Status::Ready);
    ////co_return;
}

PMaterialAsset MaterialLoader::getPlaceHolderMaterial() 
{
    return placeholderMaterial;
}
