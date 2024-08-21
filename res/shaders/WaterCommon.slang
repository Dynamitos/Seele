struct WaterPayload
{
    float3 offset;
    float extent;
};

struct WaterTile
{
    int2 location;
    float extent;
    float height;
}

struct WaterVertex
{
    float4 position_CS : SV_Position;
    float3 position_WS : POSITION0;
    float2 texCoord : TEXCOORD0;
    float depth : DEPTH;
};

struct MaterialParams
{
    float3 sunDirection;
    float displacementDepthAttenuation;
    
    float foamSubtract0;
    float foamSubtract1;
    float foamSubtract2;
    float foamSubtract3;
    
    float tile1;
    float tile2;
    float tile3;
    float tile4;

    float normalStrength;
    float foamDepthAttenuation;
    float normalDepthAttenuation;
    float roughness;
    
    float3 sunIrradiance;
    float foamRoughnessModifier;

    float3 scatterColor;
    float environmentLightStrength;

    float3 bubbleColor;
    float heightModifier;

    float bubbleDensity;
    float wavePeakScatterStrength;
    float scatterStrength;
    float scatterShadowStrength;

    uint contributeDisplacement1;
    uint contributeDisplacement2;
    uint contributeDisplacement3;
    uint contributeDisplacement4;
    
    float3 foamColor;

    Texture2DArray displacementTextures;
    Texture2DArray<float2> slopeTextures;
    TextureCube environmentMap;
    SamplerState sampler;
    StructuredBuffer<WaterTile> tiles;
};
layout(set = 1)
ParameterBlock<MaterialParams> pWaterMaterial;
