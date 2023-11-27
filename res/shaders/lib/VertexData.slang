import MaterialParameter;

interface IVertexData
{
	VertexAttributes getAttributes(uint index);
};
layout(set = 1)
ParameterBlock<IVertexData> pVertexData;