#include "PhysicsSystem.h"
#include <boost/numeric/odeint.hpp>

using namespace Seele;
using namespace Seele::Component;

PhysicsSystem::PhysicsSystem()
{

}

PhysicsSystem::~PhysicsSystem()
{
    
}

void PhysicsSystem::update(entt::registry& registry, float deltaTime)
{
    Array<Body> initialBodies;
    readRigidBodies(initialBodies, registry);

    Array<Body> bodies = integratePhysics(initialBodies, 0, deltaTime);
    writeRigidBodies(bodies, registry);

    Array<Collision> collisions;
    collisionSystem.detectCollisions(registry, collisions);
    
    if(!collisions.empty())
    {
        constexpr size_t numSteps = 2;
        for (float t = 0; t < deltaTime; t += deltaTime / numSteps)
        {
            rewindCollisions(initialBodies, registry, t, t + (deltaTime / numSteps), 10);
            readRigidBodies(initialBodies, registry);
        }
    }
}

void PhysicsSystem::serializeRB(const Body& rb, float* y) const
{
    *y++ = rb.x.x;
    *y++ = rb.x.y;
    *y++ = rb.x.z;

    *y++ = rb.q.w;
    *y++ = rb.q.x;
    *y++ = rb.q.y;
    *y++ = rb.q.z;

    *y++ = rb.P.x;
    *y++ = rb.P.y;
    *y++ = rb.P.z;

    *y++ = rb.L.x;
    *y++ = rb.L.y;
    *y++ = rb.L.z;
}

void PhysicsSystem::deserializeRB(Body& rb, const float* y) const
{
    rb.x.x = *y++;
    rb.x.y = *y++;
    rb.x.z = *y++;

    rb.q.w = *y++;
    rb.q.x = *y++;
    rb.q.y = *y++;
    rb.q.z = *y++;

    rb.P.x = *y++;
    rb.P.y = *y++;
    rb.P.z = *y++;

    rb.L.x = *y++;
    rb.L.y = *y++;
    rb.L.z = *y++;

    rb.v = rb.P * rb.inverseMass;
    rb.R = glm::mat3_cast(glm::normalize(rb.q));
    rb.iInv = rb.R * rb.iBodyInv * glm::transpose(rb.R);
    rb.omega = rb.iInv * rb.L;
}

void PhysicsSystem::serializeArray(const Array<Body>& bodies, Array<float>& x) const
{
    x.resize(bodies.size() * FLOATS_PER_RB);
    for(uint32_t i = 0; i < bodies.size(); ++i)
    {
        serializeRB(bodies[i], x.data()+(i*FLOATS_PER_RB));
    }
}

void PhysicsSystem::deserializeArray(Array<Body>& bodies, const Array<float>& x) const
{
    bodies.resize(x.size() / FLOATS_PER_RB);
    for(uint32_t i = 0; i < bodies.size(); ++i)
    {
        deserializeRB(bodies[i], x.data()+(i*FLOATS_PER_RB));
    }
}

void PhysicsSystem::readRigidBodies(Array<Body>& bodies, entt::registry& registry) const
{
    auto view = registry.view<RigidBody, Transform, Collider>();
    bodies.clear();
    bodies.reserve(view.size_hint());
    view
        .each([&bodies](entt::entity id, RigidBody& rb, Transform& transform, Collider& collider)
    {
        Body& rigidBody = bodies.add(Body(id, rb, collider, transform));
        rigidBody.updateMatrix();
    });
    registry.view<Collider, Transform>(entt::exclude<RigidBody>)
        .each([&bodies](entt::entity id, Collider& collider, Transform& transform)
    {
        Body& rigidBody = bodies.add(Body(id, collider, transform));
        rigidBody.updateMatrix();
    });
}

void PhysicsSystem::writeRigidBodies(const Array<Body>& bodies, entt::registry& registry) const
{
    for(auto& body : bodies)
    {
        if(registry.all_of<RigidBody, Transform>(body.id))
        {
            auto [physics, transform] = registry.get<RigidBody, Transform>(body.id);
            transform.setPosition(body.x);
            transform.setRotation(body.q);
            physics.linearMomentum = body.P;
            physics.angularMomentum = body.L;
            physics.force = body.force;
            physics.torque = body.torque;
        }
    }
}

PhysicsSystem::Body PhysicsSystem::readRigidBody(entt::entity entity, entt::registry& registry) const
{
    Body rigidBody;
    if(registry.all_of<RigidBody, Collider, Transform>(entity))
    {
        const auto& [physics, collider, transform] = registry.get<RigidBody, Collider, Transform>(entity);
        rigidBody = Body(entity, physics, collider, transform);
    }
    else
    {
        const auto& [collider, transform] = registry.get<Collider, Transform>(entity);
        rigidBody = Body(entity, collider, transform);
    }
    rigidBody.updateMatrix();
    return rigidBody;
}


void PhysicsSystem::writeRigidBody(const Body& body, entt::registry& registry) const
{
    if(registry.all_of<RigidBody, Transform>(body.id))
    {
        const auto& [physics, transform] = registry.get<RigidBody, Transform>(body.id);
        transform.setPosition(body.x);
        transform.setRotation(body.q);
        physics.linearMomentum = body.P;
        physics.angularMomentum = body.L;
        physics.force = body.force;
        physics.torque = body.torque;
    }
    else
    {
        auto& transform = registry.get<Transform>(body.id);
        assert(transform.getPosition() == body.x);
        assert(transform.getRotation() == body.q);
    }
}

Array<PhysicsSystem::Body> PhysicsSystem::integratePhysics(const Array<Body>& bodies, const float t0, const float tdelta) const
{
    Array<Body> result;
    Array<float> buffer;
    result.resize(bodies.size());
    buffer.resize(bodies.size() * FLOATS_PER_RB);
    std::memcpy(result.data(), bodies.data(), result.size() * sizeof(Body));
    serializeArray(bodies, buffer);
    auto dxdt = [this, &result](const Array<float>& x, Array<float>& x2, const float)
    {
        deserializeArray(result, x);
        float* xdot = x2.data();
        for (size_t i = 0; i < result.size(); i++)
        {
            // x(t)' = v(t)
            *xdot++ = result[i].v.x;
            *xdot++ = result[i].v.y;
            *xdot++ = result[i].v.z;
            
            // R(t)' = omega(t)*R(t)
            Math::Quaternion qdot = 0.5f * (Math::Quaternion(0, result[i].omega) * result[i].q);
            *xdot++ = qdot.w;
            *xdot++ = qdot.x;
            *xdot++ = qdot.y;
            *xdot++ = qdot.z;

            // P(t)' = F(t)
            *xdot++ = result[i].force.x;
            *xdot++ = result[i].force.y;
            *xdot++ = result[i].force.z;

            // L(t)' = tau(t)
            *xdot++ = result[i].torque.x;
            *xdot++ = result[i].torque.y;
            *xdot++ = result[i].torque.z;
        }
    };
    boost::numeric::odeint::stepper_rk4<Array<float>, float> stepper;
    boost::numeric::odeint::integrate_const(stepper, dxdt, buffer, t0, tdelta, tdelta);
    deserializeArray(result, buffer);
    return result;
}



void PhysicsSystem::rewindCollisions(const Array<Body>& t0Bodies, entt::registry& registry, const float t0, const float t1, size_t remainingRecursionDepth)
{
    if(remainingRecursionDepth == 0)
    {
        //std::cout << "reached max recursion depth" << std::endl;
    }
    // there are collisions happening between t0 and t1
    // we integrate until tc and see if they have already occured then
    Array<Collision> collisions;
    writeRigidBodies(integratePhysics(t0Bodies, t0, t1), registry);
    collisionSystem.detectCollisions(registry, collisions);
    
    //std::cout << "detected " << collisions.size() << " at " << tc << std::endl;
    // now we check if there has been a contact at tc
    Array<Contact> contacts;
    // collision occured at [tc; t1]
    for (auto &&collision : collisions)
    {
        const auto&[collider1, transform1] = registry.get<Collider, Transform>(collision.a);
        const auto&[collider2, transform2] = registry.get<Collider, Transform>(collision.b);
        calculateContacts(collision.a, collider1.physicsMesh.transform(transform1), collision.b, collider2.physicsMesh.transform(transform2), contacts);
    }

    // we then apply forces in order to counteract interpenetration
    Array<Contact> restingContacts;
    for(const auto& contact : contacts)
    {
        Body a = readRigidBody(contact.a, registry);
        Body b = readRigidBody(contact.b, registry);
        Math::Vector paDot = a.ptVelocity(contact.p);
        Math::Vector pbDot = b.ptVelocity(contact.p);
        float vrel = glm::dot(contact.n, paDot - pbDot);
        if(vrel > 0.001f)
        {
            continue;
        }
        if(vrel > -0.001f)
        {
            restingContacts.add(contact);
            continue;
        }
        resolvePenetratingContact(contact, a, b);
        a.updateMatrix();
        b.updateMatrix();
        writeRigidBody(a, registry);
        writeRigidBody(b, registry);
    }
    resolveRestingContacts(restingContacts, registry);
}

void PhysicsSystem::calculateContacts(entt::entity id1, const ShapeBase& shape1, entt::entity id2, const ShapeBase& shape2, Array<Contact>& contacts) const
{
    for(size_t i = 0; i < shape1.indices.size(); i += 3)
    {
        // face - vertex contacts
        const Math::Vector point1 = shape1.vertices[shape1.indices[i + 0]];
        const Math::Vector point2 = shape1.vertices[shape1.indices[i + 1]];
        const Math::Vector point3 = shape1.vertices[shape1.indices[i + 2]];
        const Math::Vector v1 = point2 - point1;
        const Math::Vector v2 = point3 - point1;
        const Math::Vector faceNormal = glm::normalize(glm::cross(v1, v2));
        auto area = [](Math::Vector ab, Math::Vector ac){
            return glm::length(glm::cross(ab, ac)) / 2.0f;
        };
        float faceArea = area(v1, v2);
        for(size_t j = 0; j < shape2.vertices.size(); j++)
        {
            Math::Vector worldPos = shape2.vertices[j];
            float dot = glm::dot(faceNormal, worldPos - point1);
            if(dot < 0.2f)
            {
                Math::Vector pa = point1 - worldPos;
                Math::Vector pb = point2 - worldPos;
                Math::Vector pc = point3 - worldPos;
                float a1 = area(pa, pb);
                float a2 = area(pb, pc);
                float a3 = area(pc, pa);

                if(std::abs(a1 + a2 + a3 - faceArea) > 0.2f)
                {
                    continue;
                }

                Contact c = {
                    .a = id2,
                    .b = id1,
                    .p = worldPos,
                    .n = faceNormal,
                    .vf = true
                };
                contacts.add(c);
            }
        }
        
        // edge - edge contacts
        auto lineLineContact = [=, &contacts](Math::Vector p1, Math::Vector p2, Math::Vector p3, Math::Vector p4)
        {
            //L1 = p1 + t * p2 - p1;
            //L2 = p3 + u * p4 - p3;
            Math::Vector a = p1;
            Math::Vector c = p3;
            Math::Vector ab = p2 - p1;
            Math::Vector cd = p4 - p3;
            float tx_den = cd.z * ab.y - cd.y * ab.z;
            float tx = (c.y * ab.z - a.y * ab.z - c.z * ab.y + a.z * ab.y) / tx_den;
            float ty_den = cd.z * ab.x - cd.z * ab.z;
            float ty = (c.x * ab.z - a.x * ab.z - c.z * ab.x + a.z * ab.x) / ty_den;
            float tz_den = cd.y * ab.x - cd.x * ab.y;
            float tz = (c.x * ab.y - a.x * ab.y - c.y * ab.x + a.y * ab.x) / tz_den;
            if (std::abs(tx - ty) < 0.1f 
             && std::abs(ty - tz) < 0.1f 
             && std::abs(tz - tx) < 0.1f
             && tx >= 0.f
             && tx <= 1.f)
            {
                Math::Vector p = p1 + tx * (p2 - p1);
                Math::Vector ea = p2 - p1;
                Math::Vector eb = p4 - p3;
                Math::Vector n = glm::normalize(glm::cross(eb, ea));
                Contact contact = {
                    .a = id1,
                    .b = id2,
                    .p = p,
                    .n = n,
                    .ea = ea,
                    .eb = eb,
                    .vf = false
                };
            }
        };
        for(size_t j = 0; j < shape2.indices.size(); j+=3)
        {
            const Math::Vector point4 = shape2.vertices[shape2.indices[j + 0]];
            const Math::Vector point5 = shape2.vertices[shape2.indices[j + 1]];
            const Math::Vector point6 = shape2.vertices[shape2.indices[j + 2]];

            lineLineContact(point1, point2, point4, point5);
            lineLineContact(point1, point2, point5, point6);
            lineLineContact(point1, point2, point6, point4);

            lineLineContact(point2, point3, point4, point5);
            lineLineContact(point2, point3, point5, point6);
            lineLineContact(point2, point3, point6, point4);

            lineLineContact(point3, point1, point4, point5);
            lineLineContact(point3, point1, point5, point6);
            lineLineContact(point3, point1, point6, point4);
        }
    }
}

void PhysicsSystem::resolveRestingContacts(const Array<Contact>& contacts, entt::registry& registry) const
{
    Array<Array<float>> amat(contacts.size(), Array<float>(contacts.size()));
    Array<float> bvec(contacts.size());

    computeAMatrix(contacts, amat, registry);
    computeBVector(contacts, bvec, registry);

    /*CGAL::Quadratic_program<float> qp(CGAL::SMALLER, true, 0, false, 0);
    for(size_t x = 0; x < contacts.size(); ++x)
    {
        for(size_t y = 0; y < contacts.size(); ++y)
        {
            qp.set_a(x, y, amat[x][y]);
        }
    }
    for(size_t y = 0; y < contacts.size(); ++y)
    {
        qp.set_b(y, bvec[y]);
    }
    CGAL::Quadratic_program_solution<CGAL::MP_Float> s = CGAL::solve_quadratic_program(qp, CGAL::MP_Float());
    assert(s.solves_quadratic_program(qp));

    auto solutionBegin = s.variable_values_begin();
    for(size_t y = 0; y < contacts.size(); ++y)
    {
        float f = CGAL::to_double(*solutionBegin++);
        Math::Vector n = contacts[y].n;
        RigidBody a = readRigidBody(contacts[y].a, registry);
        RigidBody b = readRigidBody(contacts[y].b, registry);
        a.force += f * n;
        a.torque += (contacts[y].p - a.x + a.centerOfMass) * (f * n);

        b.force -= f * n;
        b.torque -= (contacts[y].p - b.x + b.centerOfMass) * (f * n);
        writeRigidBody(a, registry);
        writeRigidBody(b, registry);
    }*/
}

void PhysicsSystem::resolvePenetratingContact(const Contact& contact, Body& a, Body& b) const
{
    Math::Vector paDot = a.ptVelocity(contact.p);
    Math::Vector pbDot = b.ptVelocity(contact.p);
    float vrel = glm::dot(contact.n, paDot - pbDot);
    Math::Vector n = contact.n;
    Math::Vector ra = contact.p - a.x + a.centerOfMass;
    Math::Vector rb = contact.p - b.x + b.centerOfMass;
    float numerator = -(1 + 0.5f) * vrel;

    float term1 = a.inverseMass;
    float term2 = b.inverseMass;
    float term3 = glm::dot(n, glm::cross(a.iInv * glm::cross(ra, n), ra));
    float term4 = glm::dot(n, glm::cross(b.iInv * glm::cross(rb, n), rb));

    float j = numerator / (term1 + term2 + term3 + term4);
    Math::Vector force = j * n;

    a.P += force;
    b.P -= force;
    a.L += glm::cross(ra, force);
    b.L -= glm::cross(rb, force);

    a.v = a.P * a.inverseMass;
    b.v = b.P * b.inverseMass;

    a.omega = a.iInv * a.L;
    b.omega = b.iInv * b.L;
}

Math::Vector PhysicsSystem::computeNdot(const Contact& c, const Body& a, const Body& b) const
{
    if(c.vf)
    {
        return glm::cross(b.omega, c.n);
    }
    else
    {
        Math::Vector eadot = glm::cross(a.omega, c.ea);
        Math::Vector ebdot = glm::cross(b.omega, c.eb);
        Math::Vector n1 = glm::cross(c.ea, c.eb);
        Math::Vector z = glm::cross(eadot, c.eb) + glm::cross(c.ea, ebdot);
        float l = glm::length(n1);
        n1 = glm::normalize(n1);

        return (z - glm::cross(glm::cross(z, n1), n1)) / l;
    }
}

float PhysicsSystem::computeAij(const Contact& ci, const Contact& cj, entt::registry& registry) const
{
    if((ci.a != cj.a) && (ci.b != cj.b) &&
       (ci.a != cj.b) && (ci.b != cj.a))
        return 0.0f;

    Body a = readRigidBody(ci.a, registry);
    Body b = readRigidBody(ci.b, registry);
    Math::Vector ni = ci.n;
    Math::Vector nj = cj.n;
    Math::Vector pi = ci.p;
    Math::Vector pj = cj.p;
    Math::Vector ra = pi - a.x + a.centerOfMass;
    Math::Vector rb = pi - b.x + b.centerOfMass;

    Math::Vector forceOnA = Math::Vector(0);
    Math::Vector torqueOnA = Math::Vector(0);
    if(cj.a == ci.a)
    {
        forceOnA = nj;
        torqueOnA = glm::cross((pj - a.x + a.centerOfMass), nj);
    }
    else if(cj.b == ci.a)
    {
        forceOnA = -nj;
        torqueOnA = glm::cross((pj - a.x + a.centerOfMass), nj);
    }
    Math::Vector forceOnB = Math::Vector(0);
    Math::Vector torqueOnB = Math::Vector(0);
    if(cj.a == ci.b)
    {
        forceOnB = nj;
        torqueOnB = glm::cross((pj - b.x + b.centerOfMass), nj);
    }
    else if(cj.b == ci.b)
    {
        forceOnB = -nj;
        torqueOnB = glm::cross((pj - b.x + b.centerOfMass), nj);
    }

    Math::Vector aLinear = forceOnA * a.inverseMass;
    Math::Vector aAngular = glm::cross((a.iInv * torqueOnA), ra);

    Math::Vector bLinear = forceOnB * b.inverseMass;
    Math::Vector bAngular = glm::cross((b.iInv * torqueOnB), rb);

    return glm::dot(ni, ((aLinear + aAngular) - (bLinear + bAngular)));
}

void PhysicsSystem::computeAMatrix(const Array<Contact>& contacts, Array<Array<float>>& amat, entt::registry& registry) const
{
    for(size_t x = 0; x < contacts.size(); ++x)
    {
        for(size_t y = 0; y < contacts.size(); ++y)
        {
            amat[x][y] = computeAij(contacts[x], contacts[y], registry);
        }
    }
}

void PhysicsSystem::computeBVector(const Array<Contact>& contacts, Array<float>& bvec, entt::registry& registry) const
{
    for(size_t y = 0; y < contacts.size(); ++y)
    {
        Contact c = contacts[y];
        Body a = readRigidBody(c.a, registry);
        Body b = readRigidBody(c.b, registry);
        Math::Vector n = c.n;
        Math::Vector ra = c.p - a.x + a.centerOfMass;
        Math::Vector rb = c.p - b.x + b.centerOfMass;

        Math::Vector fExtA = a.force;
        Math::Vector fExtB = b.force;
        Math::Vector tExtA = a.torque;
        Math::Vector tExtB = b.torque;
        
        Math::Vector aExtPart = fExtA * a.inverseMass + glm::cross(a.iInv * tExtA, ra);
        Math::Vector bExtPart = fExtB * b.inverseMass + glm::cross(b.iInv * tExtB, rb);

        Math::Vector aVelPart = glm::cross(a.omega, glm::cross(a.omega, ra)) + glm::cross(a.iInv * glm::cross(a.L, a.omega), ra);
        Math::Vector bVelPart = glm::cross(b.omega, glm::cross(b.omega, rb)) + glm::cross(b.iInv * glm::cross(b.L, b.omega), rb);

        float k1 = glm::dot(n, (aExtPart + aVelPart) - (bExtPart + bVelPart));
        Math::Vector ndot = computeNdot(c, a, b);

        float k2 = 2.0f * glm::dot(ndot, (a.ptVelocity(c.p) - b.ptVelocity(c.p)));
        bvec[y] = k1 + k2;
    }
}