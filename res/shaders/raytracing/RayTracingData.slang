import MaterialParameter;

struct RayTracingParams
{
	RaytracingAccelerationStructure scene;
    RWTexture2D<float4> radianceAccumulator;
	RWTexture2D<float4> image;
    StructuredBuffer<uint> indexBuffer;
    TextureCube<float4> skyBox;
    SamplerState skyBoxSampler;
};
layout(set=5)
ParameterBlock<RayTracingParams> pRayTracingParams;

struct CallablePayload
{
    FragmentParameter params;
    float3 color;
};

struct RayPayload
{
    float3 light;
    float emissive;
    uint depth;
    bool hit;
    bool anyHit;
    uint3 rndSeed;
};

float3 rand01(uint3 x){                   // pseudo-random number generator
    for (int i=3; i-->0;) x = ((x>>8U)^x.yzx)*1103515245U;
    return float3(x)*(1.0/float(0xffffffffU));
}