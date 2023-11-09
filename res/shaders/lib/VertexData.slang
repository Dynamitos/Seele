import MaterialParameter;

interface IVertexData
{
	VertexAttributes getAttributes(uint index, float4x4 transform);
};
ParameterBlock<IVertexData> pVertexData;