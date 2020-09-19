#include "BRDF.h"
#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace Seele;
using json = nlohmann::json;

List<BRDF*> BRDF::globalBRDFList;

BRDF::BRDF(const char* name) 
    : name(name)
{
    globalBRDFList.add(this);
}

BRDF::~BRDF() 
{
    globalBRDFList.remove(globalBRDFList.find(this));
}


BRDF* BRDF::getBRDFByName(const std::string& name) 
{
    for(auto brdf : globalBRDFList)
    {
        if(name.compare(brdf->name) == 0)
        {
            return brdf;
        }
    }
    return nullptr;
}

List<BRDF*> BRDF::getBRDFList()
{
    return globalBRDFList;
}

BlinnPhong::BlinnPhong(const char* name) 
    : BRDF(name)
{   
}

BlinnPhong::~BlinnPhong()
{
}

void BlinnPhong::generateMaterialCode(std::ofstream& codeStream, json codeJson) 
{
    std::stringstream accessorStream;

    auto generateAccessor = [codeJson](std::stringstream& accessorStream, const std::string& key, const std::string& defaultVal)
    {
        if(codeJson.contains(key))
        {
            for(auto code : codeJson[key].items())
            {
                accessorStream << code.value().get<std::string>() << ";" << std::endl;
            }
        }
        else
        {
            accessorStream << "return " << defaultVal << ";";
        }
        
    };
    accessorStream << "float3 getBaseColor(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "baseColor", "float3(0, 0, 0)");
    accessorStream << "}";

    accessorStream << "float getMetallic(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "metallic", "0.f");
    accessorStream << "}";
    
    accessorStream << "float3 getNormal(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "normal", "float3(0, 1, 0)");
    accessorStream << "}";
    
    accessorStream << "float getSpecular(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "specular", "0.f");
    accessorStream << "}";

    accessorStream << "float getRoughness(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "roughness", "0.f");
    accessorStream << "}";

    accessorStream << "float getSheen(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "sheen", "0.f");
    accessorStream << "}";
    
    /*accessorStream << "float getSpecularTint(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "specularTint", "0.f");
    accessorStream << "}";
    
    accessorStream << "float getAnisotropic(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "anisotropic", "0.f");
    accessorStream << "}";


    accessorStream << "float getSheenTint(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "sheenTint", "0.f");
    accessorStream << "}";
    
    accessorStream << "float getClearCoat(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "clearCoat", "0.f");
    accessorStream << "}";
    
    accessorStream << "float getClearCoatGloss(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "clearCoatGloss", "0.f");
    accessorStream << "}";*/
    
    accessorStream << "float3 getWorldOffset() {\n";
    generateAccessor(accessorStream, "worldOffset", "0.f");
    accessorStream << "}";

    codeStream << accessorStream.str();

    codeStream << "typedef " << name << " BRDF;" << std::endl;
    codeStream << name << " prepare(MaterialFragmentParameter geometry){" << std::endl;
    codeStream << name << " result;" << std::endl;
    codeStream << "result.baseColor = getBaseColor(geometry);" << std::endl;
    codeStream << "result.metallic = getMetallic(geometry);" << std::endl;
    codeStream << "result.normal = geometry.transformNormalTexture(getNormal(geometry));" << std::endl;
    codeStream << "result.specular = getSpecular(geometry);" << std::endl;
    codeStream << "result.roughness = getRoughness(geometry);" << std::endl;
    codeStream << "result.sheen = getSheen(geometry);" << std::endl;
    codeStream << "return result;" << std::endl;
    codeStream << "}" << std::endl;
}

IMPLEMENT_BRDF(BlinnPhong);