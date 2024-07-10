import RayTracingData;

[shader("miss")]
void main(inout float3 p)
{
    p = float3(1, 0, 1);
}