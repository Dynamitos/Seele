import Bounding;

struct MeshletDescription
{
	BoundingSphere bounding;
	uint32_t vertexCount;
	uint32_t primitiveCount;
	uint32_t vertexOffset;
	uint32_t primitiveOffset;
	float3 color;
	float pad;
};

struct MeshData
{
	BoundingSphere bounding;
	uint32_t numMeshlets;
	uint32_t meshletOffset;
	uint32_t firstIndex;
	uint32_t numIndices;
	uint32_t indicesOffset;
	uint32_t pad0[3];
};

static const uint MAX_VERTICES = 256;
static const uint MAX_PRIMITIVES = 256;
static const uint TASK_GROUP_SIZE = 128;
static const uint MESH_GROUP_SIZE = 32;
static const uint MAX_MESHLETS_PER_MESH = 512;

struct InstanceData
{
	float4x4 transformMatrix;
	float4x4 inverseTransformMatrix;
};

struct Scene
{
	StructuredBuffer<InstanceData> instances;
	StructuredBuffer<MeshData> meshData;
	StructuredBuffer<MeshletDescription> meshletInfos;
	StructuredBuffer<uint8_t> primitiveIndices;
	StructuredBuffer<uint32_t> vertexIndices;
};
layout(set=2)
ParameterBlock<Scene> pScene;

