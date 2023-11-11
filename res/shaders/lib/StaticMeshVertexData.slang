import Common;
import VertexData;
import MaterialParameter;

struct StaticMeshVertexData : IVertexData
{
	VertexAttributes getAttributes(uint index, float4x4 transform)
	{
		VertexAttributes attributes;
		MaterialParameter params;
		float4 localPos = float4(positions[3 * index + 0], positions[3 * index + 1], positions[3 * index + 2], 1);
		float4 worldPos = mul(transform, localPos);
		float4 viewPos = mul(pViewParams.viewMatrix, worldPos);
		float4 clipPos = mul(pViewParams.projectionMatrix, viewPos);
		params.worldPosition = worldPos.xyz;
		params.texCoords = float2(texCoords[2 * index + 0], texCoords[2 * index + 1]);
		params.normal = float3(normals[3 * index + 0], normals[3 * index + 1], normals[3 * index + 2]);
		params.tangent = float3(tangents[3 * index + 0], tangents[3 * index + 1], tangents[3 * index + 2]);
		params.biTangent = float3(biTangents[3 * index + 0], biTangents[3 * index + 1], biTangents[3 * index + 2]);
		params.vertexColor = float3(color[3 * index + 0], color[3 * index + 1], color[3 * index + 2]);
		attributes.parameter = params;
		attributes.clipPosition = clipPos;
		return attributes;
	}
	StructuredBuffer<float> positions;
	StructuredBuffer<float> texCoords;
	StructuredBuffer<float> normals;
	StructuredBuffer<float> tangents;
	StructuredBuffer<float> biTangents;
	StructuredBuffer<float> color;
};
