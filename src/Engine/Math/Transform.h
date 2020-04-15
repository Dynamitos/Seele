#pragma once
#include "Vector.h"

namespace Seele
{
class Transform
{
public:
    Transform();
    Transform(Vector position);
    Transform(Vector position, Quaternion rotation);
    Transform(Vector position, Quaternion rotation, Vector scale);
    Transform(Quaternion rotation, Vector scale);
    ~Transform();
    Vector inverseTransformPosition(const Vector &v) const;
    static Vector getSafeScaleReciprocal(const Vector4 &inScale, float tolerance = 0.00001f);

    inline Vector getPosition() const;
    inline Quaternion getRotation() const;
    inline Vector getScale() const;

    inline static void multiply(Transform* outTransform, const Transform* a, const Transform* b);

    Transform& operator*(const Transform& other) const
    {
        Transform outTransform;
        multiply(&outTransform, this, &other);
        return outTransform;
    }

private:
    Vector4 position;
    Quaternion rotation;
    Vector4 scale;
};
} // namespace Seele