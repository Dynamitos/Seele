import Common;
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
layout(set=2)
ParameterBlock<Scene> pScene;

[shader("vertex")]
FragmentParameter vertexMain(	
	uint vertexId: SV_VertexID,
	uint instanceId: SV_InstanceID,
) : PARAMETER{
	InstanceData inst = pScene.instances[instanceId];
	VertexAttributes attr = pVertexData.getAttributes(vertexId, inst.transformMatrix);
	return attr.getParameter(inst.transformMatrix);
}