import Common;
import VertexData;
import MaterialParameter;

struct StaticMeshVertexData : IVertexData
{
	VertexAttributes getAttributes(uint index, float4x4 transform)
	{
		VertexAttributes attributes;
		float4 modelPos = float4(positions[3 * index + 0], positions[3 * index + 1], positions[3 * index + 2], 1);
		float4 worldPos = mul(transform, modelPos);
		float4 viewPos = mul(pViewParams.viewMatrix, worldPos);
		float4 clipPos = mul(pViewParams.projectionMatrix, viewPos);
		attributes.normal_MS = float3(normals[3 * index + 0], normals[3 * index + 1], normals[3 * index + 2]);
		attributes.tangent_MS = float3(tangents[3 * index + 0], tangents[3 * index + 1], tangents[3 * index + 2]);
		attributes.biTangent_MS = float3(biTangents[3 * index + 0], biTangents[3 * index + 1], biTangents[3 * index + 2]);
		attributes.position_WS = worldPos.xyz;
		attributes.position_CS = clipPos;
		attributes.texCoords = float2(texCoords[2 * index + 0], texCoords[2 * index + 1]);
		attributes.vertexColor = float3(color[3 * index + 0], color[3 * index + 1], color[3 * index + 2]);
		return attributes;
	}
	StructuredBuffer<float> positions;
	StructuredBuffer<float> texCoords;
	StructuredBuffer<float> normals;
	StructuredBuffer<float> tangents;
	StructuredBuffer<float> biTangents;
	StructuredBuffer<float> color;
};
