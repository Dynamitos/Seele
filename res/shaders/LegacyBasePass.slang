import VertexData;
import MaterialParameter;

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
VertexAttributes vertexMain(	
	uint vertexId: SV_VertexID,
	uint instanceId: SV_InstanceID,
){
	InstanceData inst = pScene.instances[instanceId];
	VertexAttributes attr = pVertexData.getAttributes(vertexId, inst.transformMatrix);
	return attr;
}