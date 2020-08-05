#include "Material.h"
#include "Asset/AssetRegistry.h"
#include <nlohmann/json.hpp>
#include <sstream>

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
    std::stringstream codeStream;
    materialName = j["name"].get<std::string>();
    std::string profile = j["profile"].get<std::string>();
    codeStream << "import LightEnv;" << std::endl;
    codeStream << "import Material;" << std::endl;
    codeStream << "import BRDF;" << std::endl;
    codeStream << "import InputGeometry;" << std::endl;

    codeStream << "struct " << materialName << ": IMaterial {" << std::endl;
    for(auto param : j["params"].items())
    {
        std::string type = param.value()["type"].get<std::string>();
        
        auto default = param.value().find("default");
        if(type.compare("float") == 0)
        {
            PFloatParameter p = new FloatParameter();
            if(default != param.value().end())
            {
                p->defaultValue = std::stof(default.value().get<std::string>());
            }
            parameters.add(p);
        }
        else if(type.compare("float3") == 0)
        {
            PVectorParameter p = new VectorParameter();
            if(default != param.value().end())
            {
                p->defaultValue = parseVector(default.value().get<std::string>().c_str());
            }
            parameters.add(p);
        }
        else if(type.compare("Texture2D") == 0)
        {
            PTextureParameter p = new TextureParameter();
            if(default != param.value().end())
            {
                
            }
            parameters.add(p);
        }
        else if(type.compare("SamplerState") == 0)
        {
            PSamplerParameter p = new SamplerParameter();
            parameters.add(p);
        }
        else
        {
            std::cout << "Error unsupported parameter type" << std::endl;
        }
        codeStream << type << " " << param.key();
    }
    codeStream << "typedef " << profile << " BRDF;" << std::endl;
    codeStream << profile << " prepare(MaterialPixelParameter geometry){" << std::endl;
    codeStream << profile << " result;" << std::endl;
    for(auto c : j["code"].items())
    {
        codeStream << c.value().get<std::string>() << std::endl;
    }

    codeStream << "}};" << std::endl;
    materialCode = codeStream.str();
}