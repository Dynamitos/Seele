import Bounding;

struct MeshletDescription
{
    AABB bounding;
    uint32_t vertexCount;
    uint32_t primitiveCount;
    uint32_t vertexOffset;
    uint32_t primitiveOffset;
    float3 color;
    uint32_t indicesOffset;
};

struct MeshData
{
    AABB bounding;
    uint32_t numMeshlets;
    uint32_t meshletOffset;
    uint32_t firstIndex;
    uint32_t numIndices;
};

static const uint32_t MAX_VERTICES = 256;
static const uint32_t MAX_PRIMITIVES = 256;
static const uint32_t TASK_GROUP_SIZE = 128;
static const uint32_t MESH_GROUP_SIZE = 32;
static const uint32_t MAX_MESHLETS_PER_INSTANCE = 2048;

struct InstanceData
{
    float4x4 transformMatrix;
    float4x4 inverseTransformMatrix;
};

struct MeshletCullingInfo
{
    uint32_t visible[MAX_PRIMITIVES / 32];
    // lookup if a specific triangle is visible
    bool triangleVisible(uint32_t primIndex)
    {
        uint32_t arrIdx = primIndex / 32;
        uint32_t cullIdx = primIndex % 32;
        return (visible[arrIdx] & (1 << cullIdx)) != 0;
    }
    bool triangleCulled(uint32_t primIndex)
    {
        return !triangleVisible(primIndex);
    }
    bool anyVisible()
    {
        return (visible[0] + visible[1] + visible[2] + visible[3]) != 0;
    }
};

struct DrawCallOffsets
{
    uint32_t instanceOffset;
};
layout(push_constant)
ConstantBuffer<DrawCallOffsets> pOffsets;

struct Scene
{
    StructuredBuffer<InstanceData> instances;
    StructuredBuffer<MeshData> meshData;
    StructuredBuffer<MeshletDescription> meshletInfos;
    StructuredBuffer<uint8_t> primitiveIndices;
    StructuredBuffer<uint32_t> vertexIndices;
    StructuredBuffer<MeshletCullingInfo> culledMeshlets;
    StructuredBuffer<uint32_t> cullingOffsets;
};
layout(set = 2)
ParameterBlock<Scene> pScene;

uint32_t encodePrimitive(uint32_t primitiveId, uint32_t meshletId)
{
    return primitiveId + (meshletId * uint(MAX_PRIMITIVES));
}

uint2 decodePrimitive(uint32_t encoded)
{
    uint prim = uint(encoded % MAX_PRIMITIVES);
    encoded = encoded / MAX_PRIMITIVES;
    uint meshletId = uint(encoded);
    return uint2(prim, meshletId);
}

struct MeshPayload
{
    uint culledMeshlets[MAX_MESHLETS_PER_INSTANCE];
    uint instanceId;
    uint meshletOffset;
    uint cullingOffset;
};

