import Common;

struct DebugVertex
{
	float3 position;
	float3 color;
}

struct VertexOutput
{
	float4 clipPosition : SV_Position;
	float3 color : COLOR0;
};

[shader("vertex")]
VertexOutput vertexMain(DebugVertex input)
{
	VertexOutput output;
	output.clipPosition = mul(pViewParams.projectionMatrix, mul(pViewParams.viewMatrix, float4(input.position, 1.0f)));
	output.color = input.color;
	return output;
}

[shader("pixel")]
float4 fragmentMain(VertexOutput input)
{
	return float4(input.color, 1.0f);
}