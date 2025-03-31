import Common;

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
ParameterBlock<SkyboxData> pSkyboxData;

[shader("vertex")]
VertexShaderOutput vertexMain(
    uint vertexIndex : SV_VertexId)
{
    const float3 vertices[] = {
        // Back
        float3(-512, -512,  512),
        float3(-512,  512,  512),
        float3( 512, -512,  512),

        float3( 512, -512,  512),
        float3(-512,  512,  512),
        float3( 512,  512,  512),

        // Front
        float3( 512, -512, -512),
        float3( 512,  512, -512),
        float3(-512, -512, -512),

        float3(-512, -512, -512),
        float3( 512,  512, -512),
        float3(-512,  512, -512),

        // Top
        float3(-512, -512, -512),
        float3(-512, -512,  512),
        float3( 512, -512, -512),

        float3( 512, -512, -512),
        float3(-512, -512,  512),
        float3( 512, -512,  512),

        // Bottom
        float3(-512,  512,  512),
        float3(-512,  512, -512),
        float3( 512,  512,  512),

        float3( 512,  512,  512),
        float3(-512,  512, -512),
        float3( 512,  512, -512),

        // Left
        float3(-512, -512, -512),
        float3(-512,  512, -512),
        float3(-512, -512,  512),

        float3(-512, -512,  512),
        float3(-512,  512, -512),
        float3(-512,  512,  512),

        // Right
        float3( 512, -512,  512),
        float3( 512,  512,  512),
        float3( 512, -512, -512),

        float3( 512, -512, -512),
        float3( 512,  512,  512),
        float3( 512,  512, -512),
    };

    VertexShaderOutput output;
    float3x3 cameraRotation = float3x3(pViewParams.viewMatrix);
    float4 worldPos = float4(mul(cameraRotation, vertices[vertexIndex]), 1.0f);
    //clip(dot(worldPos, clipPlane));
    output.clipPos = mul(pViewParams.projectionMatrix, worldPos);
    output.texCoords = normalize(vertices[vertexIndex]);
    return output;
}

struct TextureData
{
    TextureCube cubeMap;
    TextureCube cubeMap2;
    SamplerState sampler;
};
ParameterBlock<TextureData> pSkyboxTextures;
static const float lowerLimit = 0.0;
static const float upperLimit = 0.1;

[shader("fragment")]
float4 fragmentMain(
    VertexShaderOutput output) : SV_Target
{
    float4 texture1 = pSkyboxTextures.cubeMap.Sample(pSkyboxTextures.sampler, output.texCoords);
    float4 texture2 = pSkyboxTextures.cubeMap2.Sample(pSkyboxTextures.sampler, output.texCoords);
    float4 finalColor = lerp(texture1, texture2, pSkyboxData.fogBlend.w);

    float factor = (output.texCoords.y - lowerLimit) / (upperLimit - lowerLimit);
    factor = clamp(factor, 0.0, 1.0);
    return lerp(float4(pSkyboxData.fogBlend.xyz, 1), finalColor, factor);
}