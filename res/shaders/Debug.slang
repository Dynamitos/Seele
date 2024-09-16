import Common;

struct DebugVertex
{
	float3 position : POSITION;
	float3 color : COLOR0;
};

struct Params
{
	float4 pos: SV_Position;
	float3 color: COLOR0;
}

[shader("vertex")]
Params vertexMain(
	DebugVertex vert
){
	Params result;
	result.pos = mul(pViewParams.c.projectionMatrix, mul(pViewParams.c.viewMatrix, float4(vert.position, 1)));
	result.color = vert.color;
	return result;
}

[shader("pixel")]
float4 fragmentMain(in Params params) : SV_Target
{
	return float4(params.color, 1);
}
