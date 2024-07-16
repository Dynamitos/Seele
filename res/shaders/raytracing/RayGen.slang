import Common;
import RayTracingData;

[shader("raygeneration")]
void raygen()
{
    uint3 LaunchID = DispatchRaysIndex();
    uint3 LaunchSize = DispatchRaysDimensions();

    const float2 pixelCenter = float2(LaunchID.xy) + float2(0.5, 0.5);
    const float2 inUV = pixelCenter / float2(LaunchSize.xy);
    float2 d = float2(inUV.x, 1 - inUV.y) * 2.0 - 1.0;
    float4 target = mul(pViewParams.inverseProjection, float4(d.x, d.y, 1, 1));

    RayDesc rayDesc;
    rayDesc.Origin = mul(pViewParams.inverseViewMatrix, float4(0, 0, 0, 1)).xyz;
    rayDesc.Direction = mul(pViewParams.inverseViewMatrix, float4(normalize(target.xyz), 0)).xyz;
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 10000.0;

    RayPayload payload;
    TraceRay(pRayTracingParams.scene, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, rayDesc, payload);

    pRayTracingParams.image[int2(LaunchID.xy)] = float4(payload.color, 1.0);
}
