#pragma once
#include "Math/AABB.h"

namespace Seele
{
namespace Component
{
struct RigidBody
{
    float mass = 1.0f;
    Vector force;
    Vector torque;
    Vector linearMomentum;
    Vector angularMomentum;
};
} // namespace Component
} // namespace Seele
