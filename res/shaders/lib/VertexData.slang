import MaterialParameter;

interface IVertexAttributes
{
	MaterialParameter create();
};

interface IVertexData
{
	associatedtype VertexAttributes : IVertexAttributes;
	VertexAttributes getAttributes(uint index, float4x4 transform);
};
ParameterBlock<IVertexData> pVertexData;