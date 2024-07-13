import Common;
import RayTracingData;

[shader("raygeneration")]
void raygen()
{
	const float2 pixelCenter = float2(DispatchRaysIndex().xy) + float2(0.5);
	const float2 inUV = pixelCenter / float2(DispatchRaysDimensions().xy);
	float2 d = inUV * 2.0 - 1.0;

	float4 origin = mul(pViewParams.inverseViewMatrix, float4(0, 0, 0, 1));
    float4 target = mul(pViewParams.inverseProjection, float4(d.x, d.y, 1, 1));
    float4 direction = mul(pViewParams.inverseViewMatrix, float4(target.xyz, 0));

	float tmin = 0.0001;
	float tmax = 10000.0;

	uint max_rays = 24;

    RayPayload payload;

    RayDesc desc;
    desc.Origin = origin.xyz;
    desc.Direction = direction.xyz;
    desc.TMin = tmin;
    desc.TMax = tmax;

    TraceRay(pRayTracingParams.scene, 0, 0xff, 0, 0, 0, desc, payload);

    pRayTracingParams.image[DispatchRaysIndex().xy] = float4(payload.color, 1.0f);
}
