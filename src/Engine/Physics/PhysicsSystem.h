#pragma once
#include "CollisionSystem.h"
#include "Component/Collider.h"
#include "Component/RigidBody.h"
#include "Component/Transform.h"
#include "MinimalEngine.h"
#include <entt/entt.hpp>


namespace Seele {
class PhysicsSystem {
  public:
    PhysicsSystem(entt::registry& registry);
    ~PhysicsSystem();
    void update(float deltaTime);

  private:
    struct Body {
        entt::entity id = entt::entity();
        float inverseMass = 0.0f;
        Vector centerOfMass = Vector();
        Matrix3 iBody = Matrix3(), iBodyInv = Matrix3();
        Vector scale = Vector();

        Vector x = Vector();
        Quaternion q = Quaternion();
        Vector P = Vector();
        Vector L = Vector();

        Matrix3 iInv = Matrix3();
        Matrix3 R = Matrix3();
        Vector v = Vector();
        Vector omega = Vector();
        Vector force = Vector();
        Vector torque = Vector();
        Matrix4 matrix = Matrix4();
        Vector ptVelocity(Vector p) const { return v + glm::cross(omega, p - x + centerOfMass); }
        void updateMatrix() {
            Matrix4 scaleMatrix = glm::scale(Matrix4(1), scale);
            Matrix4 rotationMatrix = glm::mat4_cast(q);
            Matrix4 translationMatrix = glm::translate(Matrix4(1), x);
            matrix = translationMatrix * rotationMatrix * scaleMatrix;
        }
        Body() {}
        Body(entt::entity id, const Component::RigidBody& physics, const Component::Collider& collider,
             const Component::Transform& transform)
            : id(id), inverseMass(1 / (physics.mass * glm::length(transform.getScale()))), centerOfMass(collider.physicsMesh.centerOfMass),
              iBody(collider.physicsMesh.bodyInertia), iBodyInv(glm::inverse(collider.physicsMesh.bodyInertia)),
              scale(transform.getScale()), x(transform.getPosition()), q(transform.getRotation()), P(physics.linearMomentum),
              L(physics.angularMomentum), iInv(glm::mat3()), R(glm::mat3()), v(Vector()), omega(Vector()), force(physics.force),
              torque(physics.torque) {
            v = P * inverseMass;
            R = glm::mat3_cast(glm::normalize(q));
            iInv = R * iBodyInv * glm::transpose(R);
            omega = iInv * L;
        }
        Body(entt::entity id, const Component::Collider& collider, const Component::Transform& transform)
            : id(id), inverseMass(0), centerOfMass(collider.physicsMesh.centerOfMass), iBody(glm::mat3(0)), iBodyInv(glm::mat3(0)),
              scale(transform.getScale()), x(transform.getPosition()), q(transform.getRotation()), P(Vector(0)), L(Vector(0)),
              iInv(glm::mat3()), R(glm::mat3()), v(Vector()), omega(Vector()), force(Vector(0)), torque(Vector(0)) {
            R = glm::mat3_cast(glm::normalize(q));
            iInv = R * iBodyInv * glm::transpose(R);
            omega = iInv * L;
        }
    };
    struct Contact {
        entt::entity a, b;
        glm::vec3 p;
        glm::vec3 n;
        glm::vec3 ea;
        glm::vec3 eb;
        bool vf;
    };
    static constexpr size_t FLOATS_PER_RB = 13;
    entt::registry& registry;
    CollisionSystem collisionSystem;
    bool pause = false;

    void serializeRB(const Body& rb, float* y) const;

    void deserializeRB(Body& rb, const float* y) const;

    void serializeArray(const Array<Body>& bodies, Array<float>& x) const;

    void deserializeArray(Array<Body>& bodies, const Array<float>& x) const;

    void readRigidBodies(Array<Body>& bodies) const;

    void writeRigidBodies(const Array<Body>& bodies) const;

    Body readRigidBody(entt::entity entity) const;

    void writeRigidBody(const Body& body) const;

    Array<Body> integratePhysics(const Array<Body>& bodies, const float t0, const float tdelta) const;

    void rewindCollisions(const Array<Body>& t0Bodies, const float t0, const float t1, size_t remainingDepth);

    void calculateContacts(entt::entity id1, const Component::ShapeBase& shape1, entt::entity id2, const Component::ShapeBase& shape2,
                           Array<Contact>& contacts) const;

    void resolveRestingContacts(const Array<Contact>& contacts) const;

    void resolvePenetratingContact(const Contact& c, Body& a, Body& b) const;

    Vector computeNdot(const Contact& c, const Body& a, const Body& b) const;

    float computeAij(const Contact& ci, const Contact& cj) const;

    void computeAMatrix(const Array<Contact>& contacts, Array<Array<float>>& amat) const;

    void computeBVector(const Array<Contact>& contacts, Array<float>& avec) const;

    void solveQP(const Array<Array<float>>& A, const Array<float>& b, Array<float>& f) const;
};
} // namespace Seele
