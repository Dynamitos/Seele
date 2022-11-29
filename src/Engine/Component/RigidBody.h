#pragma once
#include "AABB.h"

namespace Seele
{
namespace Component
{
struct RigidBody
{
    float mass = 1.0f;
    Math::Vector force;
    Math::Vector torque;
    Math::Vector linearMomentum;
    Math::Vector angularMomentum;
};
} // namespace Component
} // namespace Seele
