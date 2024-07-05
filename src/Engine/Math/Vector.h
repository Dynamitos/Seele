#pragma once
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/gtc/quaternion.hpp>
#ifdef WIN32
#pragma warning(pop)
#endif
#include <nlohmann/json_fwd.hpp>

namespace Seele {
typedef glm::vec2 Vector2;
typedef glm::vec3 Vector;
typedef glm::vec4 Vector4;

typedef glm::uvec2 UVector2;
typedef glm::uvec3 UVector;
typedef glm::uvec4 UVector4;

typedef glm::ivec2 IVector2;
typedef glm::ivec3 IVector;
typedef glm::ivec4 IVector4;

typedef glm::quat Quaternion;

Vector parseVector(const char*);

void to_json(nlohmann::json& j, const Vector& vec);
void from_json(nlohmann::json& j, Vector& vec);
} // namespace Seele

std::ostream& operator<<(std::ostream& stream, const Seele::Vector2& vector);
std::ostream& operator<<(std::ostream& stream, const Seele::Vector& vector);
std::ostream& operator<<(std::ostream& stream, const Seele::Vector4& vector);
std::ostream& operator<<(std::ostream& stream, const Seele::Quaternion& vector);
