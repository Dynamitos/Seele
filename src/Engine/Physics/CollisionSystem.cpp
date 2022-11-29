#include "CollisionSystem.h"

using namespace Seele;
using namespace Seele::Component;

CollisionSystem::CollisionSystem()
{
    
}

CollisionSystem::~CollisionSystem()
{
    
}

void CollisionSystem::detectCollisions(const entt::registry& registry, Array<Collision>& collisions)
{
    collisions.clear();
    auto view = registry.view<Collider, Transform>();
    for(auto && [entity, collider, transform] : view.each())
    {
        if(collider.type == ColliderType::DYNAMIC && transform.isDirty())
        {
            bvh.updateDynamicCollider(entity, collider.boundingbox.getTransformedBox(transform.toMatrix()));
        }
    }
    Array<Pair<entt::entity, entt::entity>> overlaps;
    bvh.findOverlaps(overlaps);
    for(auto pair : overlaps)
    {
        if(checkCollision(registry, pair))
        {
            collisions.add(Collision {
                .a = pair.key,
                .b = pair.value,
            });
        }
    }
}

bool CollisionSystem::checkCollision(const entt::registry& registry, Pair<entt::entity, entt::entity> pair)
{
    const auto&[collider1, transform1] = registry.get<Collider, Transform>(pair.key);
    const auto&[collider2, transform2] = registry.get<Collider, Transform>(pair.value);
    ShapeBase shape1 = collider1.physicsMesh.transform(transform1);
    ShapeBase shape2 = collider2.physicsMesh.transform(transform2);
    Witness witness;
    if(cachedWitness.exists(pair))
    {
        witness = cachedWitness[pair];
    }
    if(witnessValid(witness, shape1, shape2))
    {
        return false;
    }
    return createWitness(witness, shape1, shape2);
}

void CollisionSystem::updateWitness(Witness& result, const glm::vec3& point, const glm::vec3& v1, const glm::vec3& v2)
{
    const glm::vec3 faceNormal = glm::normalize(glm::cross(v1, v2));

    result.n = faceNormal;
    result.p = point;
}


bool CollisionSystem::createWitness(Witness& result, const ShapeBase& source, const ShapeBase& other)
{
    for (size_t i = 0; i < source.indices.size(); i += 3)
    {
        const glm::vec3 point1 = source.vertices[source.indices[i + 0]];
        const glm::vec3 point2 = source.vertices[source.indices[i + 1]];
        const glm::vec3 point3 = source.vertices[source.indices[i + 2]];
        const glm::vec3 v1 = point2 - point1;
        const glm::vec3 v2 = point3 - point1;
        
        updateWitness(result, point1, v1, v2);
        result.point1Index = source.indices[i + 0];
        result.point2Index = source.indices[i + 1];
        result.point3Index = source.indices[i + 2];
        result.point4Index = UINT32_MAX;
        bool valid = witnessValid(result, source, other);
        // if not, it is a valid separating plane, so no collision
        if (valid)
        {
            return false;
        }
    }
    for (size_t i = 0; i < source.indices.size(); i += 3)
    {
        auto findEdgePlane = [=, &result](uint32_t point1Index, uint32_t point2Index)
        {
            const glm::vec3 point1 = source.vertices[point1Index];
            const glm::vec3 point2 = source.vertices[point2Index];
            result.point1Index = point1Index;
            result.point2Index = point2Index;
            const glm::vec3 d1 = point2 - point1;
            for (size_t j = 0; j < other.indices.size(); j += 3)
            {
                for (const auto& [point3Index, point4Index] : { std::pair(j+1, j), std::pair(j+2, j+1), std::pair(j, j+2) })
                {
                    result.point3Index = other.indices[point3Index];
                    result.point4Index = other.indices[point4Index];
                    const glm::vec3 point3 = other.vertices[result.point3Index];
                    const glm::vec3 point4 = other.vertices[result.point4Index];
                    const glm::vec3 d2 = point3 - point4;
                    updateWitness(result, point1, d2, d1);
                    if (witnessValid(result, source, other))
                    {
                        return true;
                    }
                }
            }
            return false;
        };
        if(findEdgePlane(source.indices[i], source.indices[i+1]))
        {
            return false;
        }
        if(findEdgePlane(source.indices[i+1], source.indices[i+2]))
        {
            return false;
        }
        if(findEdgePlane(source.indices[i+2], source.indices[i]))
        {
            return false;
        }
    }
    // no separating plane was found, collision
    return true;
}

bool CollisionSystem::witnessValid(const Witness& witness, const Component::ShapeBase& shape1, const Component::ShapeBase& shape2)
{
    const float e = 0.0001f;

    for (size_t i = 0; i < shape1.vertices.size(); i++)
    {
        if(glm::dot(witness.n, shape1.vertices[i] - witness.p) > e)
        {
            // something intersecting the separating plane
            // it is not valid anymore
            return false;
        }
    }
    for (size_t i = 0; i < shape2.vertices.size(); i++)
    {
        if(glm::dot(witness.n, shape2.vertices[i] - witness.p) < -e)
        {
            // something intersecting the separating plane
            // it is not valid anymore
            return false;
        }
    }
    return true;
}

