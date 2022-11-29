#pragma once
#include <entt/entt.hpp>
#include "MinimalEngine.h"
#include "Component/Transform.h"
#include "Component/RigidBody.h"
#include "Component/Collider.h"
#include "CollisionSystem.h"

namespace Seele
{
class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();
    void update(entt::registry& registry, float deltaTime);
private:
    struct Body
    {
        entt::entity id;
        float inverseMass;
        Math::Vector centerOfMass;
        Math::Matrix3 iBody, iBodyInv;
        Math::Vector scale;

        Math::Vector x;
        Math::Quaternion q;
        Math::Vector P;
        Math::Vector L;

        Math::Matrix3 iInv;
        Math::Matrix3 R;
        Math::Vector v;
        Math::Vector omega;
        Math::Vector force;
        Math::Vector torque;
        Math::Matrix4 matrix;
        Math::Vector ptVelocity(Math::Vector p) const {return v + glm::cross(omega, p - x + centerOfMass);}
        void updateMatrix() {
            Math::Matrix4 scaleMatrix = glm::scale(Math::Matrix4(1), scale);
            Math::Matrix4 rotationMatrix = glm::mat4_cast(q);
            Math::Matrix4 translationMatrix = glm::translate(Math::Matrix4(1), x);
            matrix = translationMatrix * rotationMatrix * scaleMatrix;
        }
        Body()
        {}
        Body(entt::entity id, const Component::RigidBody& physics, const Component::Collider& collider, const Component::Transform& transform)
            : id(id)
            , inverseMass(1 / (physics.mass * glm::length(transform.getScale())))
            , centerOfMass(collider.physicsMesh.centerOfMass)
            , iBody(collider.physicsMesh.bodyInertia)
            , iBodyInv(glm::inverse(collider.physicsMesh.bodyInertia))
            , scale(transform.getScale())
            , x(transform.getPosition())
            , q(transform.getRotation())
            , P(physics.linearMomentum)
            , L(physics.angularMomentum)
            , iInv(glm::mat3())
            , R(glm::mat3())
            , v(Math::Vector())
            , omega(Math::Vector())
            , force(physics.force)
            , torque(physics.torque)
        {
            v = P * inverseMass;
            R = glm::mat3_cast(glm::normalize(q));
            iInv = R * iBodyInv * glm::transpose(R);
            omega = iInv * L;
        }
        Body(entt::entity id, const Component::Collider& collider, const Component::Transform& transform)
            : id(id)
            , inverseMass(0)
            , centerOfMass(collider.physicsMesh.centerOfMass)
            , iBody(glm::mat3(0))
            , iBodyInv(glm::mat3(0))
            , scale(transform.getScale())
            , x(transform.getPosition())
            , q(transform.getRotation())
            , P(Math::Vector(0))
            , L(Math::Vector(0))
            , iInv(glm::mat3())
            , R(glm::mat3())
            , v(Math::Vector())
            , omega(Math::Vector())
            , force(Math::Vector(0))
            , torque(Math::Vector(0))
        {
            R = glm::mat3_cast(glm::normalize(q));
            iInv = R * iBodyInv * glm::transpose(R);
            omega = iInv * L;
        }
    };
    struct Contact
    {
        entt::entity a, b;
        glm::vec3 p;
        glm::vec3 n;
        glm::vec3 ea;
        glm::vec3 eb;
        bool vf;
    };
    static constexpr size_t FLOATS_PER_RB = sizeof(Body) / sizeof(float);
    
    CollisionSystem collisionSystem;

    void serializeRB(const Body& rb, float* y) const;
    
    void deserializeRB(Body& rb, const float* y) const;
    
    void serializeArray(const Array<Body>& bodies, Array<float>& x) const;
    
    void deserializeArray(Array<Body>& bodies, const Array<float>& x) const;
    
    void readRigidBodies(Array<Body>& bodies, entt::registry& registry) const;
    
    void writeRigidBodies(const Array<Body>& bodies, entt::registry& registry) const;
    
    Body readRigidBody(entt::entity entity, entt::registry& regsitry) const;
    
    void writeRigidBody(const Body& body, entt::registry& registry) const;
    
    Array<Body> integratePhysics(const Array<Body>& bodies, const float t0, const float tdelta) const;
    
    void rewindCollisions(const Array<Body>& t0Bodies, entt::registry& registry, const float t0, const float t1, size_t remainingDepth);
    
    void calculateContacts(entt::entity id1, const Component::ShapeBase& shape1, entt::entity id2, const Component::ShapeBase& shape2, Array<Contact>& contacts) const;

    void resolveRestingContacts(const Array<Contact>& contacts, entt::registry& registry) const;

    void resolvePenetratingContact(const Contact& c, Body& a, Body& b) const;

    Math::Vector computeNdot(const Contact& c, const Body& a, const Body& b) const;

    float computeAij(const Contact& ci, const Contact& cj, entt::registry& registry) const;

    void computeAMatrix(const Array<Contact>& contacts, Array<Array<float>>& amat, entt::registry& registry) const;

    void computeBVector(const Array<Contact>& contacts, Array<float>& avec, entt::registry& registry) const;
};
} // namespace Seele
