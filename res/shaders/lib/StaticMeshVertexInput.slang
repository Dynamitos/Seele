import Common;
import MaterialParameter;
import PrimitiveSceneData;

struct VertexValueCache
{
    //This struct is passed between vertex and fragment stage
    //which means, that it is passed as out mat3x3 in glsl
    //but Slang for some reason puts a layout(row_major) above
    //every attribute, including this matrix, but matrix layout
    //qualifiers are illegal for non-uniform and buffer fields,
    //leading to a compiler error
    float3 position_MS;
    float3 tangent;
    float3 biTangent;
    float3 normal;
    float4 color;
#if NUM_MATERIAL_TEXCOORDS
    float2 texCoords[NUM_MATERIAL_TEXCOORDS];
#endif

    float4x4 getModelToTangent()
    {
        return transpose(float4x4(
            float4(normalize(tangent), 0.0f),
            float4(normalize(biTangent), 0.0f),
            float4(normalize(normal), 0.0f),
            float4(0, 0, 0, 1)
        ));
    }

    float4x4 getWorldToTangent()
    {
        return transpose(float4x4(
            normalize(mul(getSceneData().worldToModel, float4(tangent, 0))),
            normalize(mul(getSceneData().worldToModel, float4(biTangent, 0))),
            normalize(mul(getSceneData().worldToModel, float4(normal, 0))),
            float4(0, 0, 0, 1)
        ));
    }
    

    MaterialFragmentParameter getFragmentParameters()
    {
        MaterialFragmentParameter result = (MaterialFragmentParameter)0;
        float4x4 worldToTangent = getWorldToTangent();
        float4 cameraPos_TS = mul(worldToTangent, gViewParams.cameraPos_WS);
        float4 position_TS = mul(getModelToTangent(), float4(position_MS, 1.0f));
        result.worldToTangent = float3x3(worldToTangent);
        result.position_TS = position_TS.xyz;
        result.viewDir_TS = normalize((cameraPos_TS - position_TS).xyz);
        result.vertexColor = color;
        for(uint i = 0; i < NUM_MATERIAL_TEXCOORDS; ++i)
        {
            result.texCoords[i] = texCoords[i];
        }
        return result;
    }
#if USE_INSTANCING
    float4 instanceOrigin;
    float3 instanceTransform1;
    float3 instanceTransform2;
    float3 instanceTransform3;

    float4x4 getInstanceTransform()
    {
        return float4x4(
            float4(instanceTransform1.xyz, 0.0f),
            float4(instanceTransform2.xyz, 0.0f),
            float4(instanceTransform3.xyz, 0.0f),
            float4(instanceOrigin.xyz, 1.0f)
        );
    }
#endif // USE_INSTANCING
};

struct VertexShaderInput
{
	float3 position;
    float3 normal;
    float3 tangent;
    float3 biTangent;
    float4 color;
#if NUM_MATERIAL_TEXCOORDS
    #if NUM_MATERIAL_TEXCOORDS > 1
        float4 packedTexCoords[NUM_MATERIAL_TEXCOORDS / 2];
    #endif
    #if NUM_MATERIAL_TEXCOORDS == 1
        float2 packedTexCoords2;
    #endif
    #if NUM_MATERIAL_TEXCOORDS == 3
        float2 packedTexCoords2;
    #endif
    #if NUM_MATERIAL_TEXCOORDS == 5
        float2 packedTexCoords2;
    #endif
    #if NUM_MATERIAL_TEXCOORDS == 7
        float2 packedTexCoords2;
    #endif
#endif // NUM_MATERIAL_TEXCOORDS

#if USE_INSTANCING
    float4 instanceOrigin;
    float3 instanceTransform1;
    float3 instanceTransform2;
    float3 instanceTransform3;
#endif // USE_INSTANCING

    float3 getModelPosition()
    {
        return position;
    }

    float3 getWorldPosition()
    {
        return mul(getSceneData().modelToWorld, float4(position, 1.0f)).xyz;
    }

    VertexValueCache getVertexCache()
    {
        VertexValueCache cache;
        cache.position_MS = position;
        cache.tangent = tangent;
        cache.biTangent = biTangent;
        cache.normal = normal;
        cache.color = color;
#if USE_INSTANCING
        cache.instanceTransform1 = instanceTransform1;
        cache.instanceTransform2 = instanceTransform2;
        cache.instanceTransform3 = instanceTransform3;
        cache.instanceOrigin = instanceOrigin;
#endif

        cache.texCoords[0] = packedTexCoords2;

        return cache;
    }

    MaterialVertexParameter getMaterialVertexParameters(VertexValueCache cache, float3 worldPosition, float3 viewPosition)
    {
        MaterialVertexParameter result;
        result.worldPosition = worldPosition;
        result.viewPosition = viewPosition;
        result.vertexColor = cache.color;
        result.worldToTangent = float3x3(cache.getWorldToTangent());
        // TODO instancing
        /*for(int i = 0; i < NUM_MATERIAL_TEXCOORDS-1; i+=2)
        {
            result.texCoords[i] = packedTexCoords[i/2].xy;
            if(i+1 < NUM_MATERIAL_TEXCOORDS)
            {
                result.texCoords = packedTexCoords[i / 2].zw;
            }
        }*/
        result.texCoords[0] = packedTexCoords2;
        return result;
    }
};

struct PositionOnlyVertexShaderInput
{
    float3 position;
#if USE_INSTANCING
    float4 instanceOrigin;
    float3 instanceTransform1;
    float3 instanceTransform2;
    float3 instanceTransform3;
#endif // USE_INSTANCING
    float3 getWorldPosition()
    {
        return mul(getSceneData().modelToWorld, float4(position, 1.0f)).xyz;
    }
};
