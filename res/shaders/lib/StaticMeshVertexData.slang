import Common;
import VertexData;
import MaterialParameter;

struct StaticMeshVertexData : IVertexData
{
	VertexAttributes getAttributes(uint index, float4x4 transform)
	{
		VertexAttributes attributes;
		MaterialParameter params;
		float4 localPos = float4(positions[index], 1);
		float4 worldPos = mul(transform, localPos);
		float4 viewPos = mul(pViewParams.viewMatrix, worldPos);
		float4 clipPos = mul(pViewParams.projectionMatrix, viewPos);
		params.worldPosition = worldPos.xyz;
		params.texCoords = texCoords[index];
		params.normal = normals[index];
		params.tangent = tangents[index];
		params.biTangent = biTangents[index];
		attributes.parameter = params;
		attributes.clipPosition = clipPos;
		return attributes;
	}
	StructuredBuffer<float3> positions;
	StructuredBuffer<float2> texCoords;
	StructuredBuffer<float3> normals;
	StructuredBuffer<float3> tangents;
	StructuredBuffer<float3> biTangents;
};
