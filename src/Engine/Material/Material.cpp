#include "Material.h"
#include "Asset/AssetRegistry.h"
#include "Graphics/VertexShaderInput.h"
#include "BRDF.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <iostream>

Gfx::ShaderMap Material::shaderMap;
std::mutex Material::shaderMapLock;

using namespace Seele;
using json = nlohmann::json;

Material::Material()
{
}

Material::Material(const std::string& directory, const std::string& name) 
    : MaterialAsset(directory, name)
{
}

Material::Material(const std::filesystem::path& fullPath)
    : MaterialAsset(fullPath)
{
}

Material::~Material()
{
}

void Material::save() 
{
    
}

void Material::load() 
{
    
}


void Material::compile()
{
    auto& stream = getReadStream();
    json j;
    stream >> j;
    materialName = j["name"].get<std::string>();
    std::ofstream codeStream("./shaders/generated/"+materialName+".slang");
    std::string profile = j["profile"].get<std::string>();

    codeStream << "import VERTEX_INPUT_IMPORT;" << std::endl;
    codeStream << "import LightEnv;" << std::endl;
    codeStream << "import Material;" << std::endl;
    codeStream << "import BRDF;" << std::endl;
    codeStream << "import MaterialParameter;" << std::endl;

    codeStream << "struct " << materialName << ": IMaterial {" << std::endl;
    for(auto param : j["params"].items())
    {
        std::string type = param.value()["type"].get<std::string>();
        
        auto default = param.value().find("default");
        if(type.compare("float") == 0)
        {
            PFloatParameter p = new FloatParameter();
            p->name = param.key();
            if(default != param.value().end())
            {
                p->defaultValue = std::stof(default.value().get<std::string>());
            }
            parameters.add(p);
        }
        else if(type.compare("float3") == 0)
        {
            PVectorParameter p = new VectorParameter();
            p->name = param.key();
            if(default != param.value().end())
            {
                p->defaultValue = parseVector(default.value().get<std::string>().c_str());
            }
            parameters.add(p);
        }
        else if(type.compare("Texture2D") == 0)
        {
            PTextureParameter p = new TextureParameter();
            p->name = param.key();
            if(default != param.value().end())
            {
                
            }
            parameters.add(p);
        }
        else if(type.compare("SamplerState") == 0)
        {
            PSamplerParameter p = new SamplerParameter();
            p->name = param.key();
            parameters.add(p);
        }
        else
        {
            std::cout << "Error unsupported parameter type" << std::endl;
        }
        codeStream << type << " " << param.key() << ";\n";
    }
    BRDF* brdf = BRDF::getBRDFByName(profile);
    brdf->generateMaterialCode(codeStream, j["code"]);
    codeStream << "};";
    codeStream.close();
}

const Gfx::ShaderCollection* Material::getShaders(Gfx::RenderPassType renderPass, VertexInputType* vertexInput) const
{
    Gfx::ShaderPermutation permutation;
    std::string materialName = getFileName();
    std::string vertexInputName = vertexInput->getName();
    std::memcpy(permutation.materialName, materialName.c_str(), sizeof(permutation.materialName));
    std::memcpy(permutation.materialName, vertexInputName.c_str(), sizeof(permutation.materialName));
    return shaderMap.findShaders(Gfx::PermutationId(permutation));
}

Gfx::ShaderCollection& Material::createShaders(Gfx::PGraphics graphics, Gfx::RenderPassType renderPass, VertexInputType* vertexInput) 
{
    std::lock_guard lock(shaderMapLock);
    return shaderMap.createShaders(graphics, renderPass, this, vertexInput, false);
}
