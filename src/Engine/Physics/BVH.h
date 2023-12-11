#pragma once
#include <entt/entt.hpp>
#include "Containers/Array.h"
#include "Math/AABB.h"
#include "Component/Collider.h"
#include "Containers/Pair.h"

namespace Seele
{
class BVH
{
public:
    void findOverlaps(Array<Pair<entt::entity, entt::entity>>& overlaps);
    void updateDynamicCollider(entt::entity entity, AABB aabb);
    void colliderCallback(entt::registry& registry, entt::entity entity);
    void visualize();
private:
    struct AABBCenter
    {
        AABB bb;
        Vector center;
        entt::entity id;
    };
    struct Node
    {
        AABB box;
        int32 parentIndex = -1;
        int32 left = -1;
        int32 right = -1;
        bool isLeaf;
        bool isValid = true;
        entt::entity owner;
    };
    void traverseStaticTree(const AABB& aabb, entt::entity source, int32 nodeIndex, Array<Pair<entt::entity, entt::entity>>& overlaps);
    void traverseDynamicTree(const AABB& aabb, entt::entity source, int32 nodeIndex, Array<Pair<entt::entity, entt::entity>>& overlaps);
    void reinsertCollider(entt::entity, AABB aabb);
    void removeCollider(entt::entity entity);
    void addDynamicCollider(entt::entity entity, AABB aabb);
    void addStaticCollider(entt::entity entity, AABB aabb);
    void findSibling(Node newNode, int32 nodeIndex, float& bestCost, int32& result);
    float siblingCost(Node newNode, int32 siblingIndex);
    float lowerBoundCost(Node newNode, int32 branchIndex);
    int32 splitNode(Array<AABBCenter> aabbs);
    int32 allocateNode();
    void freeNode(int32 nodeIndex);
    void validateBVH() const;
    Array<Node> dynamicNodes;
    Array<Node> staticNodes;
    Array<AABBCenter> staticCollider;
    int32 staticRoot = -1;
    int32 dynamicRoot = -1;
};
} // namespace Seele
