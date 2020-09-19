struct MaterialVertexParameter
{
    float3 worldPosition;
    float3x3 tangentToWorld;
#if NUM_MATERIAL_TEXCOORDS
    float2 texCoords[NUM_MATERIAL_TEXCOORDS];
#endif
#ifdef USE_INSTANCING
    float4x4 instanceLocalToWorld;
    float3 instanceLocalPosition;
    float4 perInstanceParams;
    uint instanceId;
#endif
    //float3 preSkinnedPosition;
    //float3 perSkinnedNormal;
    float4 vertexColor;
};

struct MaterialFragmentParameter
{
    float3 worldPosition;
    float3 viewDir;
    float3 worldNormal;
    float3 worldTangent;
    float3 worldBiTangent;
	float4 clipPosition;
    float4 vertexColor;
    float3x3 tangentToWorld;
#ifdef USE_INSTANCING
    float3 perInstanceParams;
#endif
#if NUM_MATERIAL_TEXCOORDS
    float2 texCoords[NUM_MATERIAL_TEXCOORDS];
#endif
    float3 transformNormalToWorld(float3 tangentSpaceNormal)
    {
        float3 result = mul(tangentToWorld, tangentSpaceNormal);
        return result;
    }
    float3 transformNormalTexture(float3 rawNormal)
    {
	    rawNormal = 2.0 * rawNormal - float3(1.0, 1.0, 1.0);
	    return transformNormalToWorld(rawNormal);
    }
};


