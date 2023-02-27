struct MaterialVertexParameter
{
    float3 worldPosition;
    float3 viewPosition;
    float3x3 worldToTangent;
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
    float3 position_TS;
    float3 viewDir_TS;
    float4 vertexColor;
    float3x3 worldToTangent;
#ifdef USE_INSTANCING
    float3 perInstanceParams;
#endif
#if NUM_MATERIAL_TEXCOORDS
    float2 texCoords[NUM_MATERIAL_TEXCOORDS];
#endif
    float3 transformWorldToTangent(float3 vec)
    {
        return mul(worldToTangent, vec);
    }
};

