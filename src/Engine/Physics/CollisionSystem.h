#pragma once
#include "BVH.h"
#include "Component/Collider.h"
#include "Component/Transform.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include <entt/entt.hpp>


namespace Seele {
struct Collision {
    entt::entity a, b;
};
class CollisionSystem {
  public:
    CollisionSystem(entt::registry& registry);
    virtual ~CollisionSystem();
    void detectCollisions(Array<Collision>& collisions);

  private:
    struct Witness {
        Vector p;
        Vector n;
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
    entt::registry& registry;
    Map<Pair<entt::entity, entt::entity>, Witness> cachedWitness;

    bool checkCollision(Pair<entt::entity, entt::entity> pair);

    void updateWitness(Witness& witness, const Vector& point, const Vector& v1, const Vector& v2);

    // returns true if a collision was found
    bool createWitness(Witness& witness, const Component::ShapeBase& source, const Component::ShapeBase& other);

    // returns true if a collision was found
    bool witnessValid(const Witness& witness, const Component::ShapeBase& shape1, const Component::ShapeBase& shape2);
};
} // namespace Seele
