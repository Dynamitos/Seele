import Common;
import VertexData;
import MaterialParameter;

struct StaticMeshVertexData : IVertexData
{
	VertexAttributes getAttributes(uint index)
	{
		VertexAttributes attributes;
		attributes.position_MS = float3(positions[3 * index + 0], positions[3 * index + 1], positions[3 * index + 2]);
		attributes.normal_MS = float3(normals[3 * index + 0], normals[3 * index + 1], normals[3 * index + 2]);
		attributes.tangent_MS = float3(tangents[3 * index + 0], tangents[3 * index + 1], tangents[3 * index + 2]);
		attributes.biTangent_MS = float3(biTangents[3 * index + 0], biTangents[3 * index + 1], biTangents[3 * index + 2]);
		for(uint i = 0; i < MAX_TEXCOORDS; ++i)
		{
			attributes.texCoords[i] = float2(texCoords[i][2 * index + 0], texCoords[i][2 * index + 1]);
		}
		attributes.vertexColor = float3(color[3 * index + 0], color[3 * index + 1], color[3 * index + 2]);
		return attributes;
	}
    VertexAttributes getPosition(uint index)
    {
        VertexAttributes attributes;
        attributes.position_MS = float3(positions[3 * index + 0], positions[3 * index + 1], positions[3 * index + 2]);
        return attributes;
	}
	StructuredBuffer<float> positions;
	StructuredBuffer<float> normals;
	StructuredBuffer<float> tangents;
	StructuredBuffer<float> biTangents;
	StructuredBuffer<float> color;
	StructuredBuffer<float> texCoords[MAX_TEXCOORDS];
};
