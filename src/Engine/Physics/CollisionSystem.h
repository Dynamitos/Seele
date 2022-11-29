#pragma once
#include <entt/entt.hpp>
#include "BVH.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Component/Collider.h"
#include "Component/Transform.h"

namespace Seele
{
struct Collision
{
    entt::entity a, b;
};
class CollisionSystem
{
public:
    CollisionSystem();
    virtual ~CollisionSystem();
    void detectCollisions(const entt::registry& registry, Array<Collision>& collisions);
private:
    struct Witness
    {
        Math::Vector p;
        Math::Vector n;
        // for finding p
        uint32_t point1Index;
        // and the face where the plane lies on
        uint32_t point2Index;
        uint32_t point3Index;
        uint32_t point4Index = UINT32_MAX;
        entt::entity source;
        entt::entity other;
    };
    BVH bvh;
    Map<Pair<entt::entity, entt::entity>, Witness> cachedWitness;

    bool checkCollision(const entt::registry& registry, Pair<entt::entity, entt::entity> pair);

    void updateWitness(Witness& witness, const Math::Vector& point, const Math::Vector& v1, const Math::Vector& v2);

    // returns true if a collision was found
    bool createWitness(Witness& witness, const Component::ShapeBase& source, const Component::ShapeBase& other);

    // returns true if a collision was found
    bool witnessValid(const Witness& witness, const Component::ShapeBase& shape1, const Component::ShapeBase& shape2);
};
} // namespace Seele
