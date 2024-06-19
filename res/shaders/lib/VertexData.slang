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

layout(set=1)
ParameterBlock<IVertexData> pVertexData;