import Mat;


struct SimpleMaterial : IMaterial
{
    float4 sample(float2 texCoord)
    {
        return float4(1, 0, 1, 1);
    }
}
