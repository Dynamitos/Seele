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

struct VertexShaderOutput
{
	FragmentParameter parameter : PARAMETER;
	float4 clipPos: SV_Position;
}

[shader("vertex")]
VertexShaderOutput vertexMain(	
	uint vertexId: SV_VertexID,
	uint instanceId: SV_InstanceID,
){
	InstanceData inst = pScene.instances[instanceId];
	VertexAttributes attr = pVertexData.getAttributes(vertexId, inst.transformMatrix);
	VertexShaderOutput output;
	output.parameter = attr.getParameter(inst.transformMatrix);
	output.clipPos = output.parameter.position_CS;
	return output;
}