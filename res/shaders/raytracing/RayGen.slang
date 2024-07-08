import Common;
import RayTracingData;

[shader("raygeneration")]
void main()
{
	const float2 pixelCenter = float2(DispatchRaysIndex().xy) + float2(0.5);
	const float2 inUV = pixelCenter / float2(DispatchRaysDimensions().xy);
	float2 d = inUV * 2.0 - 1.0;

	float4 origin = mul(pViewParams.inverseViewMatrix, float4(0, 0, 0, 1));
	float4 target = mul(pViewParams.inverseProjection, float4(d.x, d.y, 1, 1));
	float4 direction = mul(pViewParams.inverseViewMatrix, float4(normalize(target.xyz), 0));

	float tmin = 0.0001;
	float tmax = 10000.0;

	uint max_rays = 24;

    uint object_type = 100;
    float4 color = float4(0, 0, 0, 0);
    uint current_mode = 0;
    float expectedDistance = -1;
    Payload payload;

    RayDesc desc = {
        origin.xyz,
        tmin,
        direction.xyz,
        tmax,
    };

    TraceRay(pRayTracingParams.scene, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, desc, payload);

    pRayTracingParams.image[DispatchRaysIndex().xy] = payload.color;
}
