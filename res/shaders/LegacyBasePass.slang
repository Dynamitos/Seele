import VertexData;
import StaticMeshVertexData;

struct InstanceData
{
	float4x4 transformMatrix;
};

struct Scene
{
	StructuredBuffer<InstanceData> instances;
}

ParameterBlock<Scene> scene;

[shader("vertex")]
StaticMeshVertexAttributes vertexMain(	
	uint vertexId: SV_VertexID,
	uint instanceId: SV_InstanceID,
){
	InstanceData inst = scene.instances[instanceId];
	StaticMeshVertexAttributes attr = vertexData.getAttributes(vertexId, inst.transformMatrix);
	return attr;
}