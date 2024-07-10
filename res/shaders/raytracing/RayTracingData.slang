struct RayTracingParams
{
	RaytracingAccelerationStructure scene;
	RWTexture2D<float4> image;
    StructuredBuffer<uint> indexBuffer;
};
ParameterBlock<RayTracingParams> pRayTracingParams;
