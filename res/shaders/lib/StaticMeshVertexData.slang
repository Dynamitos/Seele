import Common;
import VertexData;
import MaterialParameter;

struct StaticMeshVertexData : IVertexData
{
    float uint16ToFloat(uint16_t value)
    {
        return value / 65535.0f;
    }

	VertexAttributes getAttributes(uint index)
	{
		VertexAttributes attributes;
		attributes.position_MS = float3(positions[index * 3 + 0], positions[index * 3 + 1], positions[index * 3 + 2]);
#ifndef POS_ONLY
        //attributes.qTangent = qTangent[index];
        attributes.normal_MS = float3(normals[index * 3 + 0], normals[index * 3 + 1], normals[index * 3 + 2]);
        float4 tangentSign = float4(tangents[index * 4 + 0], tangents[index * 4 + 1], tangents[index * 4 + 2], tangents[index * 4 + 3]);
        attributes.tangent_MS = tangentSign.xyz;
        attributes.biTangent_MS = cross(attributes.normal_MS, attributes.tangent_MS) * -tangentSign.w;
		for(uint i = 0; i < MAX_TEXCOORDS; ++i)
		{
			attributes.texCoords[i] = float2(uint16ToFloat(texCoords[i][index * 2 + 0]), uint16ToFloat(texCoords[i][index * 2 + 1]));
		}
		attributes.vertexColor = float3(uint16ToFloat(color[index * 3 + 0]), uint16ToFloat(color[index * 3 + 1]) , uint16ToFloat(color[index * 3 + 2]));
#endif
		return attributes;
	}
	StructuredBuffer<float> positions;
    //StructuredBuffer<float> qTangents;
	StructuredBuffer<float> normals;
	StructuredBuffer<float> tangents;
	StructuredBuffer<uint16_t> color;
	StructuredBuffer<uint16_t> texCoords[MAX_TEXCOORDS];
};
