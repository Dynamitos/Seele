import LightEnv;

struct VertexInput
{
	float3 basePosition;
	float3 position;
	float3 velocity;
}

struct ViewParameter
{
	float4 cameraPos_WS;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	PointLight light;
}
layout(set = 0, std430)
ParameterBlock<ViewParameter> gViewParams;

struct VertexToPixel
{
	float4 position_CS : SV_Position;
}

VertexToPixel vertexMain(VertexInput input)
{
	VertexToPixel output = (VertexToPixel)0;
	float3 cameraRight_WS = float3(gViewParams.viewMatrix[0][0], gViewParams.viewMatrix[0][1], gViewParams.viewMatrix[0][2]);
    float3 cameraUp_WS = float3(gViewParams.viewMatrix[1][0], gViewParams.viewMatrix[1][1], gViewParams.viewMatrix[1][2]);
    float3 worldPosition = input.position + cameraRight_WS * input.basePosition.x + cameraUp_WS * input.basePosition.y;
    float4 viewPosition = mul(gViewParams.viewMatrix, float4(worldPosition, 1));
	output.position_CS = mul(gViewParams.projectionMatrix, viewPosition);
    return output;
}

float4 fragMain(VertexToPixel input) : SV_Target
{
	return float4(0, 1, 0, 0.001f);
}