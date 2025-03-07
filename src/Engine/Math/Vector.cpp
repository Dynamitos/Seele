#include "Vector.h"
#include "Serialization/ArchiveBuffer.h"
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>


using namespace Seele;

void to_json(nlohmann::json& j, const Vector& vec) { j = nlohmann::json{fmt::format("({}, {}, {})", vec.x, vec.y, vec.z)}; }

void from_json(const nlohmann::json& j, Vector& vec) {
    std::string str;
    j.get_to(str);
    auto newEnd = std::remove(str.begin(), str.end(), ' ');
    str.erase(newEnd);
    std::stringstream stream(str);
    std::string temp;
    std::getline(stream, temp, ',');
    vec.x = std::stof(temp);
    std::getline(stream, temp, ',');
    vec.y = std::stof(temp);
    std::getline(stream, temp, ',');
    vec.z = std::stof(temp);
}

std::ostream& operator<<(std::ostream& stream, const Vector2& vector) {
    stream << "Vec2(" << vector.x << ", " << vector.y << ")";
    return stream;
}
std::ostream& operator<<(std::ostream& stream, const Vector& vector) {
    stream << "Vec3(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
    return stream;
}
std::ostream& operator<<(std::ostream& stream, const Vector4& vector) {
    stream << "Vec4(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
    return stream;
}


std::ostream& operator<<(std::ostream& stream, const UVector2& vector) {
    stream << "UVec2(" << vector.x << ", " << vector.y << ")";
    return stream;
}
std::ostream& operator<<(std::ostream& stream, const UVector& vector) {
    stream << "UVec3(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
    return stream;
}
std::ostream& operator<<(std::ostream& stream, const UVector4& vector) {
    stream << "UVec4(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Quaternion& vector) {
    stream << "Quat(" << vector.w << ", " << vector.x << ", " << vector.y << ", " << vector.z << ")";
    return stream;
}

Vector Seele::parseVector(const char* str) {
    // regex pattern consisting of 'float3(xComp, yComp, zComp)', more also matches for invalid floats, but that will throw later
    std::regex pattern("float3\\(\\s*([0-9.]+f?)\\s*,\\s*([0-9.]+f?)\\s*,\\s*([0-9.]+f?)\\s*\\)");
    std::cmatch base_match;
    std::regex_match(str, base_match, pattern);
    // match 0 is the whole expression
    float x = std::stof(base_match[1].str());
    float y = std::stof(base_match[2].str());
    float z = std::stof(base_match[3].str());
    return Vector(x, y, z);
}
