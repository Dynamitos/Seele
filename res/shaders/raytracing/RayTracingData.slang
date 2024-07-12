import MaterialParameter;

struct RayTracingParams
{
	RaytracingAccelerationStructure scene;
	RWTexture2D<float4> image;
    StructuredBuffer<uint> indexBuffer;
    RWStructuredBuffer<float3> rayDirections;
    RWStructuredBuffer<float3> origins;
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
    float3 color;
};