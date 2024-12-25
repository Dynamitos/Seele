import RayTracingData;

[shader("miss")]
void miss(inout RayPayload p)
{
    p.shading.position = WorldRayDirection() * 1000.0f;
    p.shading.normal = -WorldRayDirection();
    p.shading.normalLight = -WorldRayDirection();
    p.material.color = float4(0, 0, 0, 0);
    p.material.emissive = float3(pRayTracingParams.skyBox.Sample(pRayTracingParams.skyBoxSampler, WorldRayDirection()).xyz);
}