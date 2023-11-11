struct MaterialParameter
{
	float3 position_TS;
	float3 worldPosition;
	float2 texCoords;
	float3 normal;
	float3 tangent;
	float3 biTangent;
	float3 viewDir_TS;
	float3 vertexColor;
}

struct VertexAttributes
{
	MaterialParameter parameter : PARAMETER;
	float4 clipPosition: SV_POSITION;
};
