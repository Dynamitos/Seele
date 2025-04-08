#include "BVH.h"

using namespace Seele;

void BVH::findOverlaps(Array<Pair<entt::entity, entt::entity>>& overlaps) {
    overlaps.clear();
    for (const auto& node : dynamicNodes) {
        if (!node.isLeaf) {
            continue;
        }
        traverseStaticTree(node.box, node.owner, staticRoot, overlaps);
        traverseDynamicTree(node.box, node.owner, dynamicRoot, overlaps);
    }
}

void BVH::updateDynamicCollider(entt::entity entity, AABB aabb) {

    for (auto& node : dynamicNodes) {
        if (node.owner == entity) {
            if (!node.box.contains(aabb)) {
                // moved out of extended bounds
                reinsertCollider(entity, aabb);
            }
            return;
        }
    }
    // new collider
    addDynamicCollider(entity, aabb);
}

void BVH::colliderCallback(entt::registry& registry, entt::entity entity) {
    Component::Collider& collider = registry.get<Component::Collider>(entity);
    Component::Transform& transform = registry.get<Component::Transform>(entity);
    if (collider.type == Component::ColliderType::STATIC) {
        addStaticCollider(entity, collider.boundingbox.getTransformedBox(transform.toMatrix()));
    }
}

void BVH::visualize() {
    Array<DebugVertex> verts;
    for (const auto& node : staticNodes) {
        node.box.visualize(verts);
    }
    for (const auto& node : dynamicNodes) {
        node.box.visualize(verts);
    }
    addDebugVertices(verts);
}

void BVH::traverseStaticTree(const AABB& aabb, entt::entity source, int32 nodeIndex, Array<Pair<entt::entity, entt::entity>>& overlaps) {
    const Node& node = staticNodes[nodeIndex];
    if (!aabb.intersects(node.box)) {
        return;
    }
    if (node.isLeaf && node.owner != source) {
        overlaps.add(Pair<entt::entity, entt::entity>(source, node.owner));
        return;
    }
    traverseStaticTree(aabb, source, node.left, overlaps);
    traverseStaticTree(aabb, source, node.right, overlaps);
}

void BVH::traverseDynamicTree(const AABB& aabb, entt::entity source, int32 nodeIndex, Array<Pair<entt::entity, entt::entity>>& overlaps) {
    if (nodeIndex == -1) {
        return;
    }
    const Node& node = dynamicNodes[nodeIndex];
    if (!aabb.intersects(node.box)) {
        return;
    }
    if (node.isLeaf && node.owner != source) {
        overlaps.add(Pair<entt::entity, entt::entity>(source, node.owner));
        return;
    }
    traverseDynamicTree(aabb, source, node.left, overlaps);
    traverseDynamicTree(aabb, source, node.right, overlaps);
}

void BVH::reinsertCollider(entt::entity entity, AABB aabb) {

    removeCollider(entity);

    addDynamicCollider(entity, aabb);
}

void BVH::removeCollider(entt::entity entity) {
    int32 nodeIndex = -1;
    for (uint32_t i = 0; i < dynamicNodes.size(); i++) {
        if (dynamicNodes[i].isLeaf && dynamicNodes[i].owner == entity) {
            nodeIndex = i;
            break;
        }
    }
    if (nodeIndex == -1) {
        return;
    }
    int32 parentIndex = dynamicNodes[nodeIndex].parentIndex;
    if (parentIndex == -1) {
        // its the root node
        dynamicRoot = -1;
        freeNode(nodeIndex);

        return;
    }
    const Node& parent = dynamicNodes[parentIndex];
    int32 siblingIndex;

    if (parent.left == nodeIndex) {
        siblingIndex = parent.right;
    } else {
        siblingIndex = parent.left;
    }

    // the node to remove and their sibling share a parent
    // now that the node is removed, the parent is also useless
    // so the grandparent should point to the sibling directly instead of the parent
    if (parent.parentIndex != -1) {
        Node& grandParent = dynamicNodes[parent.parentIndex];
        if (grandParent.left == parentIndex) {
            grandParent.left = siblingIndex;
        } else {
            grandParent.right = siblingIndex;
        }
        dynamicNodes[siblingIndex].parentIndex = parent.parentIndex;
    } else {
        // if the shared parent was the root, we need a new root, which is the remaining sibling
        dynamicRoot = siblingIndex;
        dynamicNodes[siblingIndex].parentIndex = -1;
    }

    freeNode(nodeIndex);

    freeNode(parentIndex);
}

void BVH::addDynamicCollider(entt::entity entity, AABB aabb) {
    auto center = (aabb.max + aabb.min) / 2.0f;
    // enlarge box slightly to buffer movement
    aabb = AABB{
        .min = center + (aabb.min - center) * 1.1f,
        .max = center + (aabb.max - center) * 1.1f,
    };
    int32 leafIndex = allocateNode();
    Node newNode = Node{
        .box = aabb,
        .isLeaf = true,
        .isValid = true,
        .owner = entity,
    };
    dynamicNodes[leafIndex] = newNode;

    if (dynamicRoot == -1) {
        dynamicRoot = leafIndex;
        return;
    }

    int32 bestSibling;
    float bestCost = std::numeric_limits<float>::max();
    findSibling(newNode, dynamicRoot, bestCost, bestSibling);

    int32 oldParent = dynamicNodes[bestSibling].parentIndex;
    int32 newParent = allocateNode();
    dynamicNodes[newParent] = Node{
        .box = aabb.combine(dynamicNodes[bestSibling].box),
        .parentIndex = oldParent,
        .isLeaf = false,
        .isValid = true,
    };

    if (oldParent != -1) {
        if (dynamicNodes[oldParent].left == bestSibling) {
            dynamicNodes[oldParent].left = newParent;
        } else {
            dynamicNodes[oldParent].right = newParent;
        }
        dynamicNodes[newParent].left = bestSibling;
        dynamicNodes[newParent].right = leafIndex;
        dynamicNodes[bestSibling].parentIndex = newParent;
        dynamicNodes[leafIndex].parentIndex = newParent;

    } else {
        dynamicNodes[newParent].left = bestSibling;
        dynamicNodes[newParent].right = leafIndex;
        dynamicNodes[bestSibling].parentIndex = newParent;
        dynamicNodes[leafIndex].parentIndex = newParent;
        dynamicRoot = newParent;
    }
    int32 index = dynamicNodes[leafIndex].parentIndex;
    while (index != -1) {
        int32 left = dynamicNodes[index].left;
        int32 right = dynamicNodes[index].right;

        dynamicNodes[index].box = dynamicNodes[left].box.combine(dynamicNodes[right].box);
        index = dynamicNodes[index].parentIndex;
    }
}

void BVH::addStaticCollider(entt::entity entity, AABB boundingBox) {
    staticCollider.add(AABBCenter{
        .bb = boundingBox,
        .center = (boundingBox.min + boundingBox.max) / 2.0f,
        .id = entity,
    });
    staticNodes.clear();
    staticRoot = splitNode(staticCollider);
}

void BVH::findSibling(Node newNode, int32 nodeIndex, float& bestCost, int32& result) {
    if (nodeIndex == -1) {
        return;
    }
    float lowerBound = lowerBoundCost(newNode, nodeIndex);
    if (lowerBound < bestCost) {
        float cost = siblingCost(newNode, nodeIndex);
        if (cost < bestCost) {
            bestCost = cost;
            result = nodeIndex;
        }
        findSibling(newNode, dynamicNodes[nodeIndex].left, bestCost, result);
        findSibling(newNode, dynamicNodes[nodeIndex].right, bestCost, result);
    }
}

float BVH::siblingCost(Node newNode, int32 siblingIndex) {
    const Node& sibling = dynamicNodes[siblingIndex];
    AABB newBox = sibling.box.combine(newNode.box);
    float cost = newBox.surfaceArea();
    int32 parentIndex = sibling.parentIndex;
    while (parentIndex != -1) {
        AABB parentBox = dynamicNodes[parentIndex].box;
        cost += parentBox.combine(newBox).surfaceArea() - parentBox.surfaceArea();
        parentIndex = dynamicNodes[parentIndex].parentIndex;
    }
    return cost;
}

float BVH::lowerBoundCost(Node newNode, int32 branchIndex) {
    float cost = newNode.box.surfaceArea();
    while (branchIndex != -1) {
        AABB box = dynamicNodes[branchIndex].box;
        cost += box.combine(newNode.box).surfaceArea() - box.surfaceArea();
        branchIndex = dynamicNodes[branchIndex].parentIndex;
    }
    return cost;
}

int32 BVH::splitNode(Array<AABBCenter> aabbs) {
    if (aabbs.size() == 1) {
        int32 leafIndex = static_cast<int32>(staticNodes.size());
        Node& leaf = staticNodes.add();
        leaf.isLeaf = true;
        leaf.box = aabbs[0].bb;
        leaf.left = -1;
        leaf.right = -1;
        leaf.owner = aabbs[0].id;
        return leafIndex;
    }
    AABB rootBox;
    for (size_t i = 0; i < aabbs.size(); ++i) {
        rootBox.adjust(aabbs[i].bb.min);
        rootBox.adjust(aabbs[i].bb.max);
    }
    float xlen = rootBox.max.x - rootBox.min.x;
    float ylen = rootBox.max.y - rootBox.min.y;
    float zlen = rootBox.max.z - rootBox.min.z;
    int32 longestAxis;
    if (xlen >= ylen && xlen >= zlen) {
        longestAxis = 0;
    }
    if (ylen >= xlen && ylen >= zlen) {
        longestAxis = 1;
    }
    if (zlen >= xlen && zlen >= ylen) {
        longestAxis = 2;
    }
    struct {
        bool operator()(const AABBCenter& lhs, const AABBCenter& rhs) { return lhs.center[longestAxis] < rhs.center[longestAxis]; }
        int32 longestAxis = 0;
    } compare = {longestAxis};

    std::sort(aabbs.begin(), aabbs.end(), compare);
    Array<AABBCenter> left((aabbs.size() + 1) / 2);
    Array<AABBCenter> right(aabbs.size() / 2);
    std::copy(aabbs.begin(), aabbs.begin() + left.size(), left.begin());
    std::copy(aabbs.begin() + left.size(), aabbs.end(), right.begin());
    int32 rootIndex = static_cast<int32>(staticNodes.size());
    Node& rootNode = staticNodes.add();
    rootNode.box = rootBox;
    rootNode.left = splitNode(std::move(left));
    rootNode.right = splitNode(std::move(right));
    return rootIndex;
}
int32 BVH::allocateNode() {
    for (uint32 i = 0; i < dynamicNodes.size(); i++) {
        if (!dynamicNodes[i].isValid) {
            return i;
        }
    }
    int32 newLeaf = static_cast<int32>(dynamicNodes.size());
    dynamicNodes.add();
    return newLeaf;
}
void BVH::freeNode(int32 nodeIndex) {
    dynamicNodes[nodeIndex].isValid = false;
    dynamicNodes[nodeIndex].parentIndex = -1;
}

void BVH::validateBVH() const {
    if (dynamicRoot != -1) {
        assert(dynamicNodes[dynamicRoot].parentIndex == -1);
    }
    for (uint32 i = 0; i < dynamicNodes.size(); ++i) {
        int32 nodeIdx = i;
        if (!dynamicNodes[nodeIdx].isValid)
            continue;
        while (dynamicNodes[nodeIdx].parentIndex != -1) {
            nodeIdx = dynamicNodes[nodeIdx].parentIndex;
        }
    }
}
