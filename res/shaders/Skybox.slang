import Common;

struct VertexShaderInput
{
    float3 position;
};

struct VertexShaderOutput
{
    float4 clipPos : SV_Position;
    float3 texCoords;
};

[[vk::push_constant]]
ConstantBuffer<float4x4> transformMatrix;

[shader("vertex")]
VertexStageOutput vertexMain(
    VertexShaderInput input)
{
    VertexStageOutput output;
    vec4 worldPos = 512 * vec4(input.position, 1.0f);
    clip(dot(worldPos, clipPlane));
    output.clipPos = gViewParams.projectionMatrix * gViewParams.viewMatrix * worldPos;
    output.texCoords = position;
    return output;
}

[[vk::push_constant]]
ConstantBuffer<float3> fogColor;

[[vk::push_constant]]
ConstantBuffer<float> blendFactor;

Texture3D cubeMap;
Texture3D cubeMap2;
SamplerState sampler;

const float lowerLimit = 0.0;
const float upperLimit = 0.1;

float4 fragMain(
    VertexShaderOutput output)
{
    float4 texture1 = cubeMap.Sample(sampler, output.texCoords);
    float4 texture2 = cubeMap2.Sample(sampler, output.texCoords);
    float4 finalColor = mix(texture1, texture2, blendFactor);

    float factor = (input.texCoords.y - lowerLimit) / (upperLimit - lowerLimit);
    factor = clamp(factor, 0.0, 1.0);
    return = mix(vec4(fogColor, 1), finalColor, factor);
}