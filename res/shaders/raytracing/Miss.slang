import RayTracingData;

[shader("miss")]
void miss(inout RayPayload p)
{
    p.color = float3(0, 1, 0);
    p.alpha = 1;
    p.intersection_WS = float3(0, 0, 0);
    p.normal_WS = float3(0, 0, 0);
}