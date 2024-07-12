import RayTracingData;

[shader("miss")]
void miss(inout RayPayload p)
{
    p.color = float3(0, 1, 0);
}