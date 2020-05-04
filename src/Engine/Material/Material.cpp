#include "Material.h"
#include <nlohmann/json.hpp>

using namespace Seele;
using json = nlohmann::json;

Material::Material()
{
}

Material::Material(const std::string& directory, const std::string& name) 
    : FileAsset(directory, name)
{
}

Material::Material(const std::string& fullPath) 
    : FileAsset(fullPath)
{   
}

Material::~Material()
{
}

void Material::compile()
{
    auto& stream = getReadStream();
    stream.seekg(0);
    json j;
    stream >> j;
    std::cout << j["test"] << std::endl;
}

Gfx::PGraphicsPipeline Material::getPipeline()
{
    return pipeline;
}