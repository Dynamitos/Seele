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

static const uint32_t MAX_VERTICES = 64;
static const uint32_t MAX_PRIMITIVES = 126;
static const uint32_t MAX_MESHLETS_PER_INSTANCE = 2048;

struct InstanceData
{
    float4x4 transformMatrix;
    float4x4 inverseTransformMatrix;
};

struct MeshletCullingInfo
{
    uint32_t visible;
    bool wasVisible()
    {
        return bool(visible);
    }
};

cbuffer DrawCallOffsets
{
    uint instanceOffset;
	uint textureOffset;
	uint samplerOffset;
	uint floatOffset;
} pOffsets;
//#ifdef RAY_TRACING
//layout(shaderRecordEXT)
//#else
//layout(push_constant)
//#endif
//ConstantBuffer<DrawCallOffsets> pOffsets;

struct Scene
{
    StructuredBuffer<InstanceData> instances;
    StructuredBuffer<MeshData> meshData;
    StructuredBuffer<MeshletDescription> meshletInfos;
    StructuredBuffer<uint8_t> primitiveIndices;
    StructuredBuffer<uint32_t> vertexIndices;
    StructuredBuffer<uint32_t> cullingOffsets;
    StructuredBuffer<MeshletCullingInfo> cullingInfos;
};
layout(set=2)
ParameterBlock<Scene> pScene;

uint32_t encodePrimitive(uint32_t meshletId)
{
    return meshletId;
}

uint decodePrimitive(uint32_t encoded)
{
    return encoded;
}

struct MeshPayload
{
    uint culledMeshlets[MAX_MESHLETS_PER_INSTANCE];
    uint instanceId;
    uint meshletOffset;
    uint cullingOffset;
};

