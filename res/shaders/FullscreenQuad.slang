struct QuadOutput
{
	float2 uv : UV;
	float4 position : SV_Position;
}

[shader("vertex")]
QuadOutput quadMain(uint vertexIndex : SV_VertexID)
{
	QuadOutput result;
	result.uv = float2((vertexIndex << 1) & 2, vertexIndex & 2);
	result.position = float4(result.uv * 2.0f + -1.0f, 0.0f, 1.0f);
	result.uv.y = 1 - result.uv.y;
	return result;
}