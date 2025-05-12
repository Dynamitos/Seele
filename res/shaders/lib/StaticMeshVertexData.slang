import Common;
import VertexData;
import MaterialParameter;
import Scene;

struct StaticMeshVertexData : IVertexData
{
    float uint16ToFloat(uint16_t value)
    {
        return value / 65535.0f;
    }

	VertexAttributes getAttributes(uint index)
	{
		VertexAttributes attributes;
		attributes.position_MS = float3(pScene.positions[index * 3 + 0], pScene.positions[index * 3 + 1], pScene.positions[index * 3 + 2]);
#ifndef POS_ONLY
        //attributes.qTangent = qTangent[index];
        attributes.normal_MS = float3(normals[index * 3 + 0], normals[index * 3 + 1], normals[index * 3 + 2]);
        attributes.tangent_MS = float3(tangents[index * 3 + 0], tangents[index * 3 + 1], tangents[index * 3 + 2]);
        attributes.biTangent_MS = float3(biTangents[index * 3 + 0], biTangents[index * 3 + 1], biTangents[index * 3 + 2]);;
		for(uint i = 0; i < MAX_TEXCOORDS; ++i)
		{
			attributes.texCoords[i] = float2(uint16ToFloat(texCoords[i][index * 2 + 0]), uint16ToFloat(texCoords[i][index * 2 + 1]));
		}
		attributes.vertexColor = float3(uint16ToFloat(color[index * 3 + 0]), uint16ToFloat(color[index * 3 + 1]) , uint16ToFloat(color[index * 3 + 2]));
#endif
		return attributes;
	}
    //StructuredBuffer<float> qTangents;
	StructuredBuffer<float> normals;
    StructuredBuffer<float> tangents;
    StructuredBuffer<float> biTangents;
	StructuredBuffer<uint16_t> color;
	StructuredBuffer<uint16_t> texCoords[MAX_TEXCOORDS];
};
