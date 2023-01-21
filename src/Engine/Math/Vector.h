#pragma once
#pragma warning(push)
#pragma warning(disable: 4201)
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/gtc/quaternion.hpp>
#pragma warning(pop)
#include <nlohmann/json_fwd.hpp>
namespace Seele
{
typedef glm::vec2 Vector2;
typedef glm::vec3 Vector;
typedef glm::vec4 Vector4;

typedef glm::uvec2 UVector2;
typedef glm::uvec3 UVector3;
typedef glm::uvec4 UVector4;

typedef glm::ivec2 IVector2;
typedef glm::ivec3 IVector3;
typedef glm::ivec4 IVector4;

typedef glm::quat Quaternion;

Vector parseVector(const char*);

static inline float square(float x)
{
	return x * x;
}

static inline Vector unrotateVector(Quaternion quaternion, Vector v)
{
	const Vector q(-quaternion.x, -quaternion.y, -quaternion.z);
	const Vector t = 2.f * glm::cross(q, v);
	const Vector result = v + (quaternion.w * t) + glm::cross(q, t);
	return result;
}

static inline bool equalsQuaternion(const Quaternion &right, const Quaternion &left, float tolerance)
{
	return (abs(right.x - left.x) <= tolerance && abs(right.y - left.y) <= tolerance && abs(right.z - left.z) <= tolerance && abs(right.w - left.w) <= tolerance) || (abs(right.x + left.x) <= tolerance && abs(right.y + left.y) <= tolerance && abs(right.z + left.z) <= tolerance && abs(right.w + left.w) <= tolerance);
}

static inline float clampRotatorAxis(float angle)
{
	angle = fmod(angle, 360.f);
	if (angle < 0.f)
	{
		angle += 360.f;
	}
	return angle;
}
static inline float normalizeRotatorAxis(float angle)
{
	angle = clampRotatorAxis(angle);

	if (angle > 180.f)
	{
		angle -= 360.f;
	}
	return angle;
}


static inline Vector toRotator(const Quaternion &other)
{
	const float singularityTest = other.z * other.x - other.w * other.y;
	const float yawY = 2.f * (other.w * other.z + other.x * other.y);
	const float yawX = (1.f - 2.f * (square(other.y) + square(other.z)));

	const float SINGULARITY_THRESHOLD = 0.4999995f;
	const float RAD_TO_DEG = (180.f) / glm::pi<float>();
	Vector rotatorFromQuat;

	if (singularityTest < -SINGULARITY_THRESHOLD)
	{
		rotatorFromQuat.x = -90.f;
		rotatorFromQuat.y = atan2(yawY, yawX) * RAD_TO_DEG;
		rotatorFromQuat.z = normalizeRotatorAxis(-rotatorFromQuat.y - (2.f * atan2(other.x, other.w) * RAD_TO_DEG));
	}
	else if (singularityTest > SINGULARITY_THRESHOLD)
	{
		rotatorFromQuat.x = 90.f;
		rotatorFromQuat.y = atan2(yawY, yawX) * RAD_TO_DEG;
		rotatorFromQuat.z = normalizeRotatorAxis(rotatorFromQuat.y - (2.f * atan2(other.x, other.w) * RAD_TO_DEG));
	}
	else
	{
		rotatorFromQuat.x = asin(2.f * (singularityTest)) * RAD_TO_DEG;
		rotatorFromQuat.y = atan2(yawY, yawX) * RAD_TO_DEG;
		rotatorFromQuat.z = atan2(-2.f * (other.w * other.x + other.y * other.z), (1.f - 2.f * (square(other.x) + square(other.y)))) * RAD_TO_DEG;
	}
	return rotatorFromQuat;
}
void to_json(nlohmann::json& j, const Vector& vec);
void from_json(nlohmann::json& j, Vector& vec);
} // namespace Seele

std::ostream& operator<<(std::ostream& stream, const Seele::Vector2& vector);
std::ostream& operator<<(std::ostream& stream, const Seele::Vector& vector);
std::ostream& operator<<(std::ostream& stream, const Seele::Vector4& vector);