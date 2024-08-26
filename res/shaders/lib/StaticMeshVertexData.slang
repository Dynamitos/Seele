import Common;
import VertexData;
import MaterialParameter;

struct StaticMeshVertexData
{
	VertexAttributes getAttributes(uint index)
	{
		VertexAttributes attributes;
		attributes.position_MS = positions[index].xyz;
#ifndef POS_ONLY
		attributes.normal_MS = normals[index].xyz;
		attributes.tangent_MS = tangents[index].xyz;
		attributes.biTangent_MS = biTangents[index].xyz;
		for(uint i = 0; i < MAX_TEXCOORDS; ++i)
		{
			attributes.texCoords[i] = texCoords[i][index];
		}
		attributes.vertexColor = color[index].xyz;
#endif
		return attributes;
	}
	StructuredBuffer<float4> positions;
	StructuredBuffer<float4> normals;
	StructuredBuffer<float4> tangents;
	StructuredBuffer<float4> biTangents;
	StructuredBuffer<float4> color;
	StructuredBuffer<float2> texCoords[MAX_TEXCOORDS];
};
layout(set=1)
ParameterBlock<StaticMeshVertexData> pVertexData;
