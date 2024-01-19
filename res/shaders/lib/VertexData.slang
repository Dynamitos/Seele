import MaterialParameter;

struct VertexInput
{
	uint vertexId;
	uint instanceId: SV_InstanceID;
}

interface IVertexData
{
	VertexAttributes getAttributes(uint index);
};
layout(set = 1)
ParameterBlock<IVertexData> pVertexData;