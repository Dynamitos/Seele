#pragma once
#include "Math/Vector.h"
#include "Math/Matrix.h"

namespace Seele
{
namespace Math
{
class Transform
{
public:
    Transform();
    Transform(const Transform &other);
    Transform(Transform &&other);
    explicit Transform(Vector position);
    Transform(Vector position, Quaternion rotation);
    Transform(Vector position, Quaternion rotation, Vector scale);
    Transform(Quaternion rotation, Vector scale);
    ~Transform();
    Matrix4 toMatrix() const;
    Vector transformPosition(const Vector &v) const;

    Vector getPosition() const;
    Quaternion getRotation() const;
    Vector getScale() const;

    void setPosition(Vector pos);
    void setRotation(Quaternion quat);
    void setScale(Vector scale);

    Vector getForward() const;
    Vector getRight() const;
    Vector getUp() const;

    bool equals(const Transform &other, float tolerance = 0.000000001f);
    static void multiply(Transform *outTransform, const Transform *a, const Transform *b);
    static void add(Transform *outTransform, const Transform *a, const Transform *b);

    Transform &operator=(const Transform &other);
    Transform &operator=(Transform &&other);
    Transform operator+(const Transform& other) const;
    Transform operator*(const Transform &other) const;

private:
    Vector4 position;
    Quaternion rotation;
    Vector4 scale;
};
} // namespace Math
} // namespace Seele