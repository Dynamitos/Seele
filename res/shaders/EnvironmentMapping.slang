struct VertexOutput
{
    float4 svPos : SV_Position;
    float3 localPos : LOCALPOS;
};
const static float3 vertices[] = {
    // Back
    float3(-1, -1,  1),
    float3(-1,  1,  1),
    float3( 1, -1,  1),

    float3( 1, -1,  1),
    float3(-1,  1,  1),
    float3( 1,  1,  1),

    // Front
    float3( 1, -1, -1),
    float3( 1,  1, -1),
    float3(-1, -1, -1),

    float3(-1, -1, -1),
    float3( 1,  1, -1),
    float3(-1,  1, -1),

    // Top
    float3(-1, -1, -1),
    float3(-1, -1,  1),
    float3( 1, -1, -1),

    float3( 1, -1, -1),
    float3(-1, -1,  1),
    float3( 1, -1,  1),

    // Bottom
    float3(-1,  1,  1),
    float3(-1,  1, -1),
    float3( 1,  1,  1),

    float3( 1,  1,  1),
    float3(-1,  1, -1),
    float3( 1,  1, -1),

    // Left
    float3(-1, -1, -1),
    float3(-1,  1, -1),
    float3(-1, -1,  1),

    float3(-1, -1,  1),
    float3(-1,  1, -1),
    float3(-1,  1,  1),

    // Right
    float3( 1, -1,  1),
    float3( 1,  1,  1),
    float3( 1, -1, -1),

    float3( 1, -1, -1),
    float3( 1,  1,  1),
    float3( 1,  1, -1),
};

struct ViewParams
{
    float4x4 view[6];
    float4x4 projection;
    Texture2D equirectangularMap;
    SamplerState sampler;
};
ParameterBlock<ViewParams> pViewParams;

[shader("vertex")]
VertexOutput vertMain(uint vertexIndex : SV_VertexID, uint viewIndex : SV_ViewID)
{
    VertexOutput output;
    output.localPos = vertices[vertexIndex + 6 * viewIndex];
    output.svPos = mul(pViewParams.projection, mul(pViewParams.view[viewIndex], float4(vertices[vertexIndex], 1)));
    return output;
}

const static float2 invAtan = float2(0.1591, 0.3183);
float2 sampleSphericalMap(float3 v)
{
    float2 uv = float2(atan(v.z / v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

[shader("fragment")]
float4 fragMain(float3 localPos : LOCALPOS) : SV_Target
{
    float2 uv = sampleSphericalMap(normalize(localPos));
    float3 color = pViewParams.equirectangularMap.Sample(pViewParams.sampler, uv).rgb;
    return float4(color, 1);
}