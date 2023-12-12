import AABB;

struct MeshletDescription
{
	uint32_t vertexCount;
	uint32_t primitiveCount;
	uint32_t vertexOffset;
	uint32_t primitiveOffset;
};

struct MeshData
{
    uint32_t meshletOffset = 0;
    uint16_t numMeshlets = 0;
    uint16_t indicesOffset = 0;
    uint32_t firstIndex = 0;
    uint32_t numIndices = 0;
};

static const uint MAX_VERTICES = 64;
static const uint MAX_PRIMITIVES = 126;
static const uint TASK_GROUP_SIZE = 128;
static const uint MESH_GROUP_SIZE = 32;
static const uint MAX_MESHLETS_PER_MESH = 512;

struct InstanceData
{
	float4x4 transformMatrix;
};

struct Scene
{
	StructuredBuffer<InstanceData> instances;
	StructuredBuffer<MeshData> meshData;
	StructuredBuffer<MeshletDescription> meshletInfos;
	StructuredBuffer<uint8_t> primitiveIndices;
	StructuredBuffer<uint32_t> vertexIndices;
};

ParameterBlock<Scene> pScene;

