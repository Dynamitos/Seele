import Common;
import VertexData;
import MaterialParameter;

struct StaticMeshVertexAttributes : IVertexAttributes
{
	float4 clipPosition: SV_Position;
	float3 worldPosition: POSITION0;
	float2 texCoords: TEXCOORD0;
	float3 normal: NORMAL0;
	float3 tangent: TANGENT0;
	float3 biTangent: BITANGENT0;
	
	MaterialParameter create()
	{
		MaterialParameter result;
		result.worldPosition = worldPosition;
		result.texCoords = texCoords;
		result.normal = normal;
		result.tangent = tangent;
		result.biTangent = biTangent;
		return result;
	}

};

struct StaticMeshVertexData : IVertexData
{
	typedef StaticMeshVertexAttributes VertexAttributes;
	StaticMeshVertexAttributes getAttributes(uint index, float4x4 transform)
	{
		StaticMeshVertexAttributes attr;
		float4 localPos = float4(positions[index], 1);
		float4 worldPos = mul(transform, localPos);
		float4 viewPos = mul(pViewParams.viewMatrix, worldPos);
		float4 clipPos = mul(pViewParams.projectionMatrix, viewPos);
		attr.clipPosition = clipPos;
		attr.worldPosition = worldPos.xyz;
		attr.texCoords = texCoords[index];
		attr.normal = normals[index];
		attr.tangent = tangents[index];
		attr.biTangent = biTangents[index];
		return attr;
	}
	StructuredBuffer<float3> positions;
	StructuredBuffer<float2> texCoords;
	StructuredBuffer<float3> normals;
	StructuredBuffer<float3> tangents;
	StructuredBuffer<float3> biTangents;
};
