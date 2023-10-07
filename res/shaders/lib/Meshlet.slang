struct MeshletDescription
{
	uint32_t vertexCount;
	uint32_t primitiveCount;
	uint32_t vertexOffset;
	uint32_t primitiveOffset;
};

struct MeshletData
{
	StructuredBuffer<MeshletDescription> meshletInfos;
	StructuredBuffer<uint8_t> primitiveIndices;

	StructuredBuffer<uint32_t> vertexIndices;
};

static const uint MAX_VERTICES = 64;
static const uint MAX_PRIMITIVES = 126;
static const uint GROUP_SIZE = 32;

ParameterBlock<MeshletData> meshlets;
