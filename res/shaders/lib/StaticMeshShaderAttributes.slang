import MaterialParameter;
import PrimitiveSceneData;

struct VertexValueCache
{
    float3x3 tangentToLocal;
    float3x3 tangentToWorld;
    float tangentToWorldSign;
    float4 color;

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
            float4(instanceOrigin.xyz, 1.0f));
        )
    }
#endif // USE_INSTANCING
};

struct ShaderAttributeInterpolation
{
    float3 worldPosition;
    float3 tangentX;
    float4 tangentZ;
    float4 color;
#if NUM_MATERIAL_TEXCOORDS
    float4 packedTexCoords[(NUM_MATERIAL_TEXCOORDS+1) / 2];
#endif // NUM_MATERIAL_TEXCOORDS
#if USE_INSTANCING
    float4 perInstanceParams;
#endif

    MaterialFragmentParameter getMaterialParameter(float4 clipSpacePosition)
    {
        MaterialFragmentParameter result;
#if NUM_MATERIAL_TEXCOORDS
        for(int i = 0; i < NUM_MATERIAL_TEXCOORDS; ++i)
        {
            if(i % 2)
            {
                result.texCoords[i] = packedTexCoords[i / 2].zw;
            }
            else
            {
                result.texCoords[i] = packedTexCoords[i / 2].xy;
            }
        }
#endif
        float3 tangentY = cross(tangentZ.xyz, tangentX) * tangentZ.w;
        result.tangentToWorld[0] = cross(tangentY, tangentZ.xyz) * tangentZ.w;
        result.tangentToWorld[1] = tangentY;
        result.tangentToWorld[2] = tangentZ.xyz;
        result.clipPosition = clipSpacePosition;
        result.worldPosition = worldPosition;
        result.viewDir = gViewParams.cameraPos_WS.xyz - worldPosition;
        result.worldNormal = tangentZ.xyz;
        result.worldTangent = tangentX;
        result.worldBiTangent = tangentY;
        result.vertexColor = color;
        return result;
    }
};

struct VertexShaderInput
{
	float4 position;
    float3 tangentX;
    float4 tangentZ;
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
    
    float3x3 calcTangentToLocal(out float tangentSign)
    {
        float3x3 result;
        tangentSign = tangentZ.w;

        float3 tangentY = cross(tangentZ.xyz, tangentX) * tangentZ.w;
        result[0] = cross(tangentY, tangentZ.xyz) * tangentZ.w;
        result[1] = tangentY;
        result[2] = tangentZ.xyz;

        return result;
    }
    
    float3 getWorldPosition(VertexValueCache cache)
    {
        float4x4 localToWorld = gSceneData.localToWorld;
        float3 rotatedPosition = localToWorld[0].xyz * position.xxx + localToWorld[1].xyz * position.yyy + localToWorld[2].xyz * position.zzz;
        return rotatedPosition + localToWorld[3].xyz;
    }

    VertexValueCache getVertexCache()
    {
        VertexValueCache cache;
        cache.color = color;
#if USE_INSTANCING
        cache.instanceTransform1 = instanceTransform1;
        cache.instanceTransform2 = instanceTransform2;
        cache.instanceTransform3 = instanceTransform3;
        cache.instanceOrigin = instanceOrigin;
#endif

        float tangentSign;
        cache.tangentToLocal = calcTangentToLocal(tangentSign);
        float3x3 localToWorld = float3x3(gSceneData.localToWorld[0].xyz, gSceneData.localToWorld[1].xyz, gSceneData.localToWorld[2].xyz);
        cache.tangentToWorld = mul(cache.tangentToLocal, localToWorld);
        cache.tangentToWorldSign = tangentSign;

        return cache;
    }

    MaterialVertexParameter getMaterialVertexParameters(VertexValueCache cache, float3 worldPosition, float3x3 tangentToLocal)
    {
        MaterialVertexParameter result;
        result.worldPosition = worldPosition;
        result.vertexColor = cache.color;
        result.tangentToWorld = cache.tangentToWorld;
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

    ShaderAttributeInterpolation getInterpolants(VertexValueCache cache, MaterialVertexParameter vertexParams)
    {
        ShaderAttributeInterpolation result = (ShaderAttributeInterpolation)0;
        result.tangentX = tangentX;
        result.tangentZ = tangentZ;
        result.color = color;
        result.worldPosition = vertexParams.worldPosition;
        for(int i = 0; i < NUM_MATERIAL_TEXCOORDS; ++i)
        {
            if(i % 2)
            {
                result.packedTexCoords[i / 2].zw = vertexParams.texCoords[i];
            }
            else
            {
                result.packedTexCoords[i / 2].xy = vertexParams.texCoords[i];
            }
        }
        return result;
    }
};

struct PositionOnlyVertexShaderInput
{
    float4 position;
#if USE_INSTANCING
    float4 instanceOrigin;
    float3 instanceTransform1;
    float3 instanceTransform2;
    float3 instanceTransform3;
#endif // USE_INSTANCING
};
