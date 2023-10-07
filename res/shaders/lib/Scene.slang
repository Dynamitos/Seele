struct InstanceData
{
	float4x4 transformMatrix;
};

struct MeshData
{
	uint numMeshlets;
	uint meshletOffset;
	uint numInstances;
	uint instanceOffset;
};

struct Scene
{
	StructuredBuffer<InstanceData> instances;
	StructuredBuffer<MeshData> meshData;
};

ParameterBlock<Scene> scene;
