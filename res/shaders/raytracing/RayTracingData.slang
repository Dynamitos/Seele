
struct Payload
{
	float3 color;
};

struct RayTracingParams
{
	RaytracingAccelerationStructure scene;
	RWTexture2D<float3> image;
    StructuredBuffer<uint> indexBuffer;
};
ParameterBlock<RayTracingParams> pRayTracingParams;
