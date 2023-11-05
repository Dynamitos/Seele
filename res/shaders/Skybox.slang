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
struct SkyboxData
{
    float4x4 transformMatrix;
    float4 fogBlend;
};
ParameterBuffer<SkyboxData> gSkyboxData;

[shader("vertex")]
VertexShaderOutput vertexMain(
    VertexShaderInput input)
{
    VertexShaderOutput output;
    float3x3 cameraRotation = float3x3(gViewParams.viewMatrix);
    float4 worldPos = float4(mul(cameraRotation, input.position), 1.0f);
    //clip(dot(worldPos, clipPlane));
    output.clipPos = mul(gViewParams.projectionMatrix, worldPos);
    output.texCoords = normalize(input.position);
    return output;
}

struct TextureData
{
    TextureCube cubeMap;
    TextureCube cubeMap2;
    SamplerState sampler;
};
ParameterBuffer<TextureData> textures;
static const float lowerLimit = 0.0;
static const float upperLimit = 0.1;

[shader("fragment")]
float4 fragmentMain(
    VertexShaderOutput output) : SV_Target
{
    float4 texture1 = textures.cubeMap.Sample(textures.sampler, output.texCoords);
    float4 texture2 = textures.cubeMap2.Sample(textures.sampler, output.texCoords);
    float4 finalColor = lerp(texture1, texture2, gSkyboxData.fogBlend.w);

    float factor = (output.texCoords.y - lowerLimit) / (upperLimit - lowerLimit);
    factor = clamp(factor, 0.0, 1.0);
    return lerp(float4(gSkyboxData.fogBlend.xyz, 1), finalColor, factor);
}