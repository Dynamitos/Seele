import MaterialParameter;

struct VertexInput
{
	uint vertexId : SV_VertexID;
	uint instanceId: SV_InstanceID;
}

interface IVertexData
{
	VertexAttributes getAttributes(uint index);
};

layout(set=2)
ParameterBlock<IVertexData> pVertexData;