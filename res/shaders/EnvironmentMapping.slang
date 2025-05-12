const static float3 vertices[] = {
    // Right
    float3( 1, -1,  1),
    float3( 1,  1,  1),
    float3( 1, -1, -1),

    float3( 1, -1, -1),
    float3( 1,  1,  1),
    float3( 1,  1, -1),

    // Left
    float3(-1, -1, -1),
    float3(-1,  1, -1),
    float3(-1, -1,  1),

    float3(-1, -1,  1),
    float3(-1,  1, -1),
    float3(-1,  1,  1),

    // Bottom
    float3(-1,  1,  1),
    float3(-1,  1, -1),
    float3( 1,  1,  1),

    float3( 1,  1,  1),
    float3(-1,  1, -1),
    float3( 1,  1, -1),

    // Top
    float3(-1, -1, -1),
    float3(-1, -1,  1),
    float3( 1, -1, -1),

    float3( 1, -1, -1),
    float3(-1, -1,  1),
    float3( 1, -1,  1),

    // Front
    float3( 1, -1, -1),
    float3( 1,  1, -1),
    float3(-1, -1, -1),

    float3(-1, -1, -1),
    float3( 1,  1, -1),
    float3(-1,  1, -1),

    // Back
    float3(-1, -1,  1),
    float3(-1,  1,  1),
    float3( 1, -1,  1),

    float3( 1, -1,  1),
    float3(-1,  1,  1),
    float3( 1,  1,  1),

};

struct ViewParams
{
    float4x4 view[6];
    float4x4 projection;
    Texture2D equirectangularMap;
    SamplerState sampler;
    TextureCube cubeMap;
};
ParameterBlock<ViewParams> pViewParams;
struct VertexOutput
{
    float4 svPos : SV_Position;
    float3 localPos : LOCALPOS;
};
[shader("vertex")]
VertexOutput vertMain(uint vertexIndex : SV_VertexID, uint viewIndex : SV_ViewID)
{
    VertexOutput output;
    output.localPos = vertices[vertexIndex];
    output.svPos = mul(pViewParams.projection, mul(pViewParams.view[viewIndex], float4(output.localPos, 1)));
    return output;
}

const static float2 invAtan = float2(0.1591, 0.3183);
float2 sampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(-v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

[shader("pixel")]
float4 computeCubemap(float3 localPos : LOCALPOS) : SV_Target
{
    float2 uv = sampleSphericalMap(normalize(localPos));
    float3 color = pViewParams.equirectangularMap.Sample(pViewParams.sampler, uv).rgb;
    return float4(color, 1);
}

static const float PI = 3.14159265359;
[shader("pixel")]
float4 convolveCubemap(float3 localPos : LOCALPOS) : SV_Target
{
    float3 normal = normalize(localPos);

    float3 irradiance = float3(0);

    float3 up = float3(0, 1, 0);
    float3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0f;
    for(float phi = 0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0; theta < 0.5 * PI; theta += sampleDelta)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += pViewParams.cubeMap.Sample(pViewParams.sampler, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / nrSamples);

    return float4(irradiance, 1);
}