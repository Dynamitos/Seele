interface IMaterial
{
    float4 sample(float2 texCoord);
}

type_param TMaterial : IMaterial;
ParameterBlock<TMaterial> gMaterial;