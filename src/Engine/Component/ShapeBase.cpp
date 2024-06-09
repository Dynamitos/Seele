#include "ShapeBase.h"
#include "Math/AABB.h"

using namespace Seele;
using namespace Seele::Component;

// https://people.eecs.berkeley.edu/~jfc/mirtich/massProps.html
struct ComputationState {
    // compute physics properties
    int A; /* alpha */
    int B; /* beta */
    int C; /* gamma */

    /* projection integrals */
    float P1, Pa, Pb, Paa, Pab, Pbb, Paaa, Paab, Pabb, Pbbb;

    /* face integrals */
    float Fa, Fb, Fc, Faa, Fbb, Fcc, Faaa, Fbbb, Fccc, Faab, Fbbc, Fcca;

    /* volume integrals */
    float T0;
    Vector T1, T2, TP;
};

struct Face {
    StaticArray<Vector, 3> vertices;
    Vector normal;
    float w;
};

void computeProjectionIntegrals(Face& f, ComputationState& state) {
    state.P1 = state.Pa = state.Pb = state.Paa = state.Pab = state.Pbb = state.Paaa = state.Paab = state.Pabb = state.Pbbb = 0.0;
    for (uint32_t i = 0; i < 3; ++i) {
        float a0 = f.vertices[i][state.A];
        float b0 = f.vertices[i][state.B];
        float a1 = f.vertices[(i + 1) % 3][state.A];
        float b1 = f.vertices[(i + 1) % 3][state.B];

        float da = a1 - a0;
        float db = b1 - b0;

        float a0_2 = a0 * a0, a0_3 = a0_2 * a0, a0_4 = a0_3 * a0;
        float b0_2 = b0 * b0, b0_3 = b0_2 * b0, b0_4 = b0_3 * b0;
        float a1_2 = a1 * a1, a1_3 = a1_2 * a1;
        float b1_2 = b1 * b1, b1_3 = b1_2 * b1;

        float C1 = a1 + a0;
        float Ca = a1 * C1 + a0_2;
        float Caa = a1 * Ca + a0_3;
        float Caaa = a1 * Caa + a0_4;
        float Cb = b1 * (b1 + b0) + b0_2;
        float Cbb = b1 * Cb + b0_3;
        float Cbbb = b1 * Cbb + b0_4;
        float Cab = 3 * a1_2 + 2 * a1 * a0 + a0_2;
        float Kab = a1_2 + 2 * a1 * a0 + 3 * a0_2;
        float Caab = a0 * Cab + 4 * a1_3;
        float Kaab = a1 * Kab + 4 * a0_3;
        float Cabb = 4 * b1_3 + 3 * b1_2 * b0 + 2 * b1 * b0_2 + b0_3;
        float Kabb = b1_3 + 2 * b1_2 * b0 + 3 * b1 * b0_2 + 4 * b0_3;

        state.P1 += db * C1;
        state.Pa += db * Ca;
        state.Paa += db * Caa;
        state.Paaa += db * Caaa;
        state.Pb += da * Cb;
        state.Pbb += da * Cbb;
        state.Pbbb += da * Cbbb;
        state.Pab += db * (b1 * Cab + b0 * Kab);
        state.Paab += db * (b1 * Caab + b0 * Kaab);
        state.Pabb += da * (a1 * Cabb + a0 * Kabb);
    }

    state.P1 /= 2.0;
    state.Pa /= 6.0;
    state.Paa /= 12.0;
    state.Paaa /= 20.0;
    state.Pb /= -6.0;
    state.Pbb /= -12.0;
    state.Pbbb /= -20.0;
    state.Pab /= 24.0;
    state.Paab /= 60.0;
    state.Pabb /= -60.0;
}

void computeFaceIntegrals(Face& f, ComputationState& state) {
    computeProjectionIntegrals(f, state);
    float k1 = 1.0f / f.normal[state.C];
    float k2 = k1 * k1;
    float k3 = k2 * k1;
    float k4 = k3 * k1;

    glm::vec3 n = f.normal;
    float w = f.w;

    state.Fa = k1 * state.Pa;
    state.Fb = k1 * state.Pb;
    state.Fc = -k2 * (n[state.A] * state.Pa + n[state.B] * state.Pb + f.w * state.P1);

    state.Faa = k1 * state.Paa;
    state.Fbb = k1 * state.Pbb;
    state.Fcc = k3 * (n[state.A] * n[state.A] * state.Paa + 2 * n[state.A] * n[state.B] * state.Pab + n[state.B] * n[state.B] * state.Pbb +
                      w * (2 * (n[state.A] * state.Pa + n[state.B] * state.Pb) + w * state.P1));

    state.Faaa = k1 * state.Paaa;
    state.Fbbb = k1 * state.Pbbb;
    state.Fccc =
        -k4 *
        (n[state.A] * n[state.A] * n[state.A] * state.Paaa + 3 * n[state.A] * n[state.A] * n[state.B] * state.Paab +
         3 * n[state.A] * n[state.B] * n[state.B] * state.Pabb + 3 * n[state.B] * n[state.B] * n[state.B] * state.Pbbb +
         3 * w * (n[state.A] * n[state.A] * state.Paa + 2 * n[state.A] * n[state.B] * state.Pa + n[state.B] * n[state.B] * state.Pbb) +
         w * w * (3 * (n[state.A] * state.Pa + n[state.B] * state.Pb) + w * state.P1));

    state.Faab = k1 * state.Paab;
    state.Fbbc = -k2 * (n[state.A] * state.Paab + n[state.B] * state.Pbbb + w * state.Pbb);
    state.Fcca = k3 * (n[state.A] * n[state.A] * state.Paa + 2 * n[state.A] * n[state.B] * state.Paab +
                       n[state.B] * n[state.B] * state.Pabb + w * (2 * (n[state.A] * state.Paa + n[state.B] * state.Pab) + w * state.Pa));
}

void computeVolumeIntegrals(const Array<Vector> vertices, const Array<uint32>& indices, ComputationState& state) {
    std::memset(&state, 0, sizeof(ComputationState));
    for (size_t i = 0; i < indices.size(); i += 3) {
        Face f;
        f.vertices = {
            vertices[indices[i]],
            vertices[indices[i + 1]],
            vertices[indices[i + 2]],
        };

        Vector e1 = f.vertices[2] - f.vertices[0];
        Vector e2 = f.vertices[1] - f.vertices[0];
        f.normal = glm::normalize(glm::cross(e1, e2));
        f.w = -f.normal.x * f.vertices[0].x - f.normal.y * f.vertices[0].y - f.normal.z * f.vertices[0].z;

        float nx = std::abs(f.normal.x);
        float ny = std::abs(f.normal.y);
        float nz = std::abs(f.normal.z);
        if (nx > ny && nx > nz)
            state.C = 0;
        else
            state.C = (ny > nz) ? 1 : 2;
        state.A = (state.C + 1) % 3;
        state.B = (state.A + 1) % 3;

        computeFaceIntegrals(f, state);

        state.T0 += f.normal.x * ((state.A == 0) ? state.Fa : ((state.B == 0) ? state.Fb : state.Fc));
        state.T1[state.A] += f.normal[state.A] * state.Faa;
        state.T1[state.B] += f.normal[state.B] * state.Fbb;
        state.T1[state.C] += f.normal[state.C] * state.Fcc;
        state.T2[state.A] += f.normal[state.A] * state.Faaa;
        state.T2[state.B] += f.normal[state.B] * state.Fbbb;
        state.T2[state.C] += f.normal[state.C] * state.Fccc;
        state.TP[state.A] += f.normal[state.A] * state.Faab;
        state.TP[state.B] += f.normal[state.B] * state.Fbbc;
        state.TP[state.C] += f.normal[state.C] * state.Fcca;
    }
    state.T1 /= 2.0f;
    state.T2 /= 3.0f;
    state.TP /= 2.0f;
}

void computePhysicsParamsForMesh(Array<Vector>& vertices, const Array<uint32>& indices, Matrix3& bodyInertia, Vector& centerOfMass,
                                 float& mass) {
    ComputationState state;
    computeVolumeIntegrals(vertices, indices, state);
    float density = 1;
    mass = density * state.T0;
    Vector r = state.T1 / state.T0;
    centerOfMass = r;
    bodyInertia[0][0] = density * (state.T2.y + state.T2.z);
    bodyInertia[1][1] = density * (state.T2.z + state.T2.x);
    bodyInertia[2][2] = density * (state.T2.x + state.T2.y);
    bodyInertia[0][1] = bodyInertia[1][0] = -density * state.TP.x;
    bodyInertia[1][2] = bodyInertia[2][1] = -density * state.TP.y;
    bodyInertia[2][1] = bodyInertia[1][2] = -density * state.TP.z;

    bodyInertia[0][0] -= mass * (r.y * r.y + r.z * r.z);
    bodyInertia[1][1] -= mass * (r.z * r.z + r.x * r.x);
    bodyInertia[2][2] -= mass * (r.x * r.x + r.y * r.y);
    bodyInertia[0][1] = bodyInertia[1][0] += mass * r.x * r.y;
    bodyInertia[1][2] = bodyInertia[2][1] += mass * r.y * r.z;
    bodyInertia[2][1] = bodyInertia[1][2] += mass * r.z * r.x;
}

ShapeBase::ShapeBase() {}

ShapeBase::ShapeBase(Array<Vector> vertices, Array<uint32> indices) : vertices(vertices), indices(indices) {
    computePhysicsParamsForMesh(vertices, indices, bodyInertia, centerOfMass, mass);
}

ShapeBase ShapeBase::transform(const Component::Transform& transform) const {
    ShapeBase result = *this;
    for (auto& vert : result.vertices) {
        vert = transform.toMatrix() * Vector4(vert, 1.0f);
    }
    return result;
}

void ShapeBase::addCollider(Array<Vector> verts, Array<uint32> inds, Matrix4 matrix) {
    size_t indOffset = vertices.size();
    for (auto vert : verts) {
        vertices.add(Vector(matrix * Vector4(vert, 1.0f)));
    }
    for (auto ind : inds) {
        indices.add(ind + static_cast<uint32>(indOffset));
    }
    computePhysicsParamsForMesh(vertices, indices, bodyInertia, centerOfMass, mass);
}

void ShapeBase::visualize() const {
    Array<DebugVertex> verts;
    for (uint32 i = 0; i < indices.size(); i += 3) {
        verts.add(DebugVertex{
            .position = Vector(vertices[indices[i + 0]]),
            .color = Vector(1, 0, 0),
        });

        verts.add(DebugVertex{
            .position = Vector(vertices[indices[i + 1]]),
            .color = Vector(1, 0, 0),
        });

        verts.add(DebugVertex{
            .position = Vector(vertices[indices[i + 1]]),
            .color = Vector(1, 0, 0),
        });

        verts.add(DebugVertex{
            .position = Vector(vertices[indices[i + 2]]),
            .color = Vector(1, 0, 0),
        });

        verts.add(DebugVertex{
            .position = Vector(vertices[indices[i + 2]]),
            .color = Vector(1, 0, 0),
        });

        verts.add(DebugVertex{
            .position = Vector(vertices[indices[i + 0]]),
            .color = Vector(1, 0, 0),
        });
    }
    addDebugVertices(std::move(verts));
}
