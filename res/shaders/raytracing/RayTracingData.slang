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

struct ShadingParams
{
    float3 position;
    float3 normal;
    float3 normalLight;
};

struct MaterialParams
{
    float4 color;
    float3 emissive;
}

struct RayPayload
{
    ShadingParams shading;
    MaterialParams material;
};