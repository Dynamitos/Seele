import Common;
import DispatchParams;
import LightEnv;

struct ComputeShaderInput
{
    uint3 groupID : SV_GroupID;
    uint3 groupThreadID : SV_GroupThreadID;
    uint3 dispatchThreadID : SV_DispatchThreadID;
    uint groupIndex : SV_GroupIndex;
};
struct CullingParams
{
    Texture2D depthTexture;

    globallycoherent RWStructuredBuffer<uint> oLightIndexCounter;
    globallycoherent RWStructuredBuffer<uint> tLightIndexCounter;

    RWStructuredBuffer<uint> oLightIndexList;
    RWStructuredBuffer<uint> tLightIndexList;

    RWTexture2D<uint2> oLightGrid;
    RWTexture2D<uint2> tLightGrid;
};
layout(set=2)
ParameterBlock<CullingParams> pCullingParams;
// Debug
//Texture2D lightCountHeatMap;
//RWTexture2D<float4> debugTexture;

groupshared uint uMinDepth;
groupshared uint uMaxDepth;

groupshared Frustum groupFrustum;

groupshared uint oLightCount;
groupshared uint oLightIndexStartOffset;
groupshared uint oLightList[1024];

groupshared uint tLightCount;
groupshared uint tLightIndexStartOffset;
groupshared uint tLightList[1024];


void oAppendLight(uint lightIndex)
{
    uint index;
    InterlockedAdd(oLightCount, 1, index);
    if(index < 1024)
    {
        oLightList[index] = lightIndex;
    }
}

void tAppendLight(uint lightIndex)
{
    uint index;
    InterlockedAdd(tLightCount, 1, index);
    if(index < 1024)
    {
        tLightList[index] = lightIndex;
    }
}

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
[shader("compute")]
void cullLights(ComputeShaderInput in)
{
    int2 texCoord = int2(in.dispatchThreadID.xy);
    float fDepth = pCullingParams.depthTexture.Load(int3(texCoord, 0)).r;

    uint uDepth = asuint(fDepth);
    if(in.groupIndex == 0)
    {
        uMinDepth = 0xffffffff;
        uMaxDepth = 0x0;
        oLightCount = 0;
        tLightCount = 0;
        groupFrustum = pDispatchParams.frustums[in.groupID.x + (in.groupID.y * pDispatchParams.numThreadGroups.x)];
    }

    GroupMemoryBarrierWithGroupSync();

    InterlockedMin(uMinDepth, uDepth);
    InterlockedMax(uMaxDepth, uDepth);

    GroupMemoryBarrierWithGroupSync();

    float fMinDepth = asfloat(uMinDepth);
    float fMaxDepth = asfloat(uMaxDepth);

    float minDepthVS = fMinDepth;
    float maxDepthVS = fMaxDepth;
    float nearClipVS = 0.1f;

    Plane minPlane = {float3(0, 0, -1), -minDepthVS};

    for ( uint i = in.groupIndex; i < pLightEnv.numPointLights; i += BLOCK_SIZE * BLOCK_SIZE )
    {
        PointLight light = pLightEnv.pointLights[i];
        float3 lightClip = light.getClipPosition();
        //TODO: why doesn't this check go through?
        if(light.insideFrustum(groupFrustum, lightClip, nearClipVS, maxDepthVS))
        {
            tAppendLight(i);
            if(!light.insidePlane(minPlane, lightClip))
            {
                oAppendLight(i);
            }
        }
    }
    
    GroupMemoryBarrierWithGroupSync();

    if(in.groupIndex == 0)
    {
        InterlockedAdd(pCullingParams.oLightIndexCounter[0], oLightCount, oLightIndexStartOffset);
        pCullingParams.oLightGrid[in.groupID.xy] = uint2(oLightIndexStartOffset, oLightCount);

        InterlockedAdd(pCullingParams.tLightIndexCounter[0], tLightCount, tLightIndexStartOffset);
        pCullingParams.tLightGrid[in.groupID.xy] = uint2(tLightIndexStartOffset, tLightCount);
    }
    GroupMemoryBarrierWithGroupSync();

    for (uint j = in.groupIndex; j < oLightCount; j += BLOCK_SIZE * BLOCK_SIZE)
    {
        pCullingParams.oLightIndexList[oLightIndexStartOffset + j] = oLightList[j];
    }
    
    // For transparent geometry.
    for ( uint k = in.groupIndex; k < tLightCount; k += BLOCK_SIZE * BLOCK_SIZE )
    {
        pCullingParams.tLightIndexList[tLightIndexStartOffset + k] = tLightList[k];
    }
}
