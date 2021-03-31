#include "Vector.h"
#include <regex>

using namespace Seele;

Vector Seele::parseVector(const char* str)
{
    //regex pattern consisting of 'float3(xComp, yComp, zComp)', more also matches for invalid floats, but that will throw later
    std::regex pattern("float3\\(\\s*([0-9.]+f?)\\s*,\\s*([0-9.]+f?)\\s*,\\s*([0-9.]+f?)\\s*\\)");
    std::cmatch base_match;
    std::regex_match(str, base_match, pattern);
    //match 0 is the whole expression
    float x = std::stof(base_match[1].str());
    float y = std::stof(base_match[2].str());
    float z = std::stof(base_match[3].str());
    return Vector(x, y, z);
}