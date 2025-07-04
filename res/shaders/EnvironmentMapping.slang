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

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i)/float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    float3 up        = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent   = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

layout(push_constant)
ConstantBuffer<float> pRoughness;

[shader("pixel")]
float4 computePrefilteredCubemap(float3 localPos : LOCALPOS) : SV_Target
{
    float3 N = normalize(localPos);    
    float3 R = N;
    float3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;   
    float3 prefilteredColor = float3(0.0);     
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H  = ImportanceSampleGGX(Xi, N, pRoughness);
        float3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            prefilteredColor += pViewParams.cubeMap.Sample(pViewParams.sampler, L).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    return float4(prefilteredColor, 1.0);
}