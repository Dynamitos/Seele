import MaterialParameter;

struct RayTracingParams
{
	RaytracingAccelerationStructure scene;
	RWTexture2D<float4> image;
    StructuredBuffer<uint> indexBuffer;
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
    float alpha;
    float3 intersection_WS;
    float3 normal_WS;
};