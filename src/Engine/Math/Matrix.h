#pragma once
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
namespace Seele {
typedef glm::mat2 Matrix2;
typedef glm::mat3 Matrix3;
typedef glm::mat4 Matrix4;

static Matrix4 perspectiveProjection(float fov, float aspect, float nearPlane, float farPlane) {
    const float e = 1.0f / std::tan(fov * 0.5f);
    return {
        {
            e / aspect,
            0.0f,
            0.0f,
            0.0f,
        },
        {
            0.0f,
            -e,
            0.0f,
            0.0f,
        },
        {
            0.0f,
            0.0f,
            (nearPlane + farPlane) / (nearPlane - farPlane),
            -1.0f,
        },
        {
            0.0f,
            0.0f,
            (farPlane * nearPlane) / (nearPlane - farPlane),
            0.0f,
        },
    };
}
static Matrix4 orthographicProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    return Matrix4{
        {
            2.0f / (right - left),
            0.0f,
            0.0f,
            0.0f,
        },
        {
            0.0f,
            2.0f / (top - bottom),
            0.0f,
            0.0f,
        },
        {
            0.0f,
            0.0f,
            -2.0f / (nearPlane - farPlane),
            0.0f,
        },
        {
            -(right + left) / (right - left),
            -(top + bottom) / (top - bottom),
            -(nearPlane + farPlane) / (nearPlane - farPlane),
            1.0f,
        },
    };
}
} // namespace Seele