#pragma once
#include "Vector.h"
#include "Matrix.h"

namespace Seele
{
class Transform
{
public:
    Transform();
    Transform(const Transform &other);
    Transform(Transform &&other);
    Transform(Vector position);
    Transform(Vector position, Quaternion rotation);
    Transform(Vector position, Quaternion rotation, Vector scale);
    Transform(Quaternion rotation, Vector scale);
    ~Transform();
    Vector inverseTransformPosition(const Vector &v) const;
    Matrix4 toMatrix();
    static Vector getSafeScaleReciprocal(const Vector4 &inScale, float tolerance = 0.000000001f);
    Vector transformPosition(const Vector &v) const;

    Vector getPosition() const;
    Quaternion getRotation() const;
    Vector getScale() const;

    bool equals(const Transform &other, float tolerance = 0.000000001f);
    static void multiply(Transform *outTransform, const Transform *a, const Transform *b);

    Transform &operator=(const Transform &other);
    Transform &operator=(Transform &&other);
    Transform operator*(const Transform &other) const;

private:
    Vector4 position;
    Quaternion rotation;
    Vector4 scale;
};
} // namespace Seele