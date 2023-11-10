import MaterialParameter;

interface IVertexData
{
	VertexAttributes getAttributes(uint index, float4x4 transform);
};
layout(set = 1)
ParameterBlock<IVertexData> pVertexData;