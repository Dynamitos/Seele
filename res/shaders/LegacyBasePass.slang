import VertexData;

struct InstanceData
{
	float4x4 transformMatrix;
};

struct Scene
{
	StructuredBuffer<InstanceData> instances;
}

ParameterBlock<Scene> pScene;

[shader("vertex")]
IVertexAttributes vertexMain(	
	uint vertexId: SV_VertexID,
	uint instanceId: SV_InstanceID,
){
	InstanceData inst = pScene.instances[instanceId];
	IVertexAttributes attr = pVertexData.getAttributes(vertexId, inst.transformMatrix);
	return attr;
}