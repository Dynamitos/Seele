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
                accessorStream << "\t\t" << code.value().get<std::string>() << ";" << std::endl;
            }
        }
        else
        {
            accessorStream << "\t\treturn " << defaultVal << ";\n";
        }
        
    };
    accessorStream << "\tfloat3 getBaseColor(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "baseColor", "float3(0.5f, 0.5f, 0.5f)");
    accessorStream << "\t}\n";

    accessorStream << "\tfloat getMetallic(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "metallic", "0.f");
    accessorStream << "\t}\n";
    
    accessorStream << "\tfloat3 getNormal(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "normal", "float3(0, 1, 0)");
    accessorStream << "\t}\n";
    
    accessorStream << "\tfloat getSpecular(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "specular", "0.f");
    accessorStream << "\t}\n";

    accessorStream << "\tfloat getRoughness(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "roughness", "0.f");
    accessorStream << "\t}\n";

    accessorStream << "\tfloat getSheen(MaterialFragmentParameter input) {\n";
    generateAccessor(accessorStream, "sheen", "0.f");
    accessorStream << "\t}\n";
    
    accessorStream << "\tfloat3 getWorldOffset() {\n";
    generateAccessor(accessorStream, "worldOffset", "0.f");
    accessorStream << "\t}\n";

    codeStream << accessorStream.str();

    codeStream << "\ttypedef " << name << " BRDF;" << std::endl;
    codeStream << "\t" << name << " prepare(MaterialFragmentParameter geometry) {" << std::endl;
    codeStream << "\t\t" << name << " result;" << std::endl;
    codeStream << "\t\tresult.baseColor = getBaseColor(geometry);" << std::endl;
    codeStream << "\t\tresult.metallic = getMetallic(geometry);" << std::endl;
    codeStream << "\t\tresult.normal = getNormal(geometry);" << std::endl;
    codeStream << "\t\tresult.specular = getSpecular(geometry);" << std::endl;
    codeStream << "\t\tresult.roughness = getRoughness(geometry);" << std::endl;
    codeStream << "\t\tresult.sheen = getSheen(geometry);" << std::endl;
    codeStream << "\t\treturn result;" << std::endl;
    codeStream << "\t}" << std::endl;
}

IMPLEMENT_BRDF(BlinnPhong)