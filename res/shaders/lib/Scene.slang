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

static const uint MAX_VERTICES = 256;
static const uint MAX_PRIMITIVES = 256;
static const uint TASK_GROUP_SIZE = 128;
static const uint MESH_GROUP_SIZE = 32;

struct InstanceData
{
	float4x4 transformMatrix;
	float4x4 inverseTransformMatrix;
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
//	StructuredBuffer<uint32_t> cullingOffsets;
//	RWStructuredBuffer<uint32_t> culledMeshlets;
};
layout(set=2)
ParameterBlock<Scene> pScene;

struct MeshPayload
{
	uint instanceId;
	uint culledMeshlets[2048];
};

