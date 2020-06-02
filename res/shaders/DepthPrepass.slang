import InputGeometry;

struct ViewParams
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
    float4 cameraPos_WS;
};

layout(set = 0, binding = 0)
ParameterBlock<ViewParams> gViewParams;

struct ModelParameter
{
	float4x4 modelMatrix;
}

[[vk::push_constant]]
ConstantBuffer<ModelParameter> gModelParams;

struct VertexShaderOutput
{
	float4 out_Position : SV_Position;
}
[shader("vertex")]
VertexShaderOutput depthPrepass(PositionOnlyVertexInput input)
{
	VertexShaderOutput out;
	float4 worldPos = mul(gModelParams.modelMatrix, float4(input.getVertexPosition(), 1));
	float4 viewPos = mul(gViewParams.viewMatrix, worldPos);
	out.out_Position = mul(gViewParams.projectionMatrix, viewPos);
	return out;
}