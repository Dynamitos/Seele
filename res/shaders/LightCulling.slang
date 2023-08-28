import LightEnv;
import Common;

struct ComputeShaderInput
{
    uint3 groupID : SV_GroupID;
    uint3 groupThreadID : SV_GroupThreadID;
    uint3 dispatchThreadID : SV_DispatchThreadID;
    uint groupIndex : SV_GroupIndex;
};
layout(set = INDEX_VIEW_PARAMS, binding = 1)
cbuffer DispatchParams
{
    uint3 numThreadGroups;
    uint pad0;
    uint3 numThreads;
    uint pad1;
}

layout(set = INDEX_VIEW_PARAMS, binding = 2)
Texture2D depthTextureVS;

layout(set = INDEX_VIEW_PARAMS, binding = 3)
globallycoherent RWShaderBuffer<uint> oLightIndexCounter;
layout(set = INDEX_VIEW_PARAMS, binding = 4)
globallycoherent RWShaderBuffer<uint> tLightIndexCounter;

layout(set = INDEX_VIEW_PARAMS, binding = 5)
RWShaderBuffer<uint> oLightIndexList;
layout(set = INDEX_VIEW_PARAMS, binding = 6)
RWShaderBuffer<uint> tLightIndexList;

layout(set = INDEX_VIEW_PARAMS, binding = 7)
RWTexture2D<uint2> oLightGrid;
layout(set = INDEX_VIEW_PARAMS, binding = 8)
RWTexture2D<uint2> tLightGrid;

layout(set = INDEX_VIEW_PARAMS, binding = 9)
ShaderBuffer<Frustum> frustums;

// Debug
//layout(set = INDEX_VIEW_PARAMS, binding = 10)
//Texture2D lightCountHeatMap;
//layout(set = INDEX_VIEW_PARAMS, binding = 11)
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
    float fDepth = depthTextureVS.Load(int3(texCoord, 0)).r;

    uint uDepth = asuint(fDepth);
    if(in.groupIndex == 0)
    {
        uMinDepth = 0xffffffff;
        uMaxDepth = 0x0;
        oLightCount = 0;
        tLightCount = 0;
        groupFrustum = frustums[in.groupID.x + (in.groupID.y * numThreadGroups.x)];
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

    for ( uint i = in.groupIndex; i < numPointLights; i += BLOCK_SIZE * BLOCK_SIZE )
    {
        PointLight light = pointLights[i];
        //TODO: why doesn't this check go through?
        //if(light.insideFrustum(groupFrustum, nearClipVS, maxDepthVS))
        {
            tAppendLight(i);
            if(!light.insidePlane(minPlane))
            {
                oAppendLight(i);
            }
        }
    }
    
    GroupMemoryBarrierWithGroupSync();

    if(in.groupIndex == 0)
    {
        InterlockedAdd(oLightIndexCounter[0], oLightCount, oLightIndexStartOffset);
        oLightGrid[in.groupID.xy] = uint2(oLightIndexStartOffset, oLightCount);

        InterlockedAdd(tLightIndexCounter[0], tLightCount, tLightIndexStartOffset);
        tLightGrid[in.groupID.xy] = uint2(tLightIndexStartOffset, tLightCount);
    }
    GroupMemoryBarrierWithGroupSync();

    for (uint j = in.groupIndex; j < oLightCount; j += BLOCK_SIZE * BLOCK_SIZE)
    {
        oLightIndexList[oLightIndexStartOffset + j] = oLightList[j];
    }
    
    // For transparent geometry.
    for ( uint k = in.groupIndex; k < tLightCount; k += BLOCK_SIZE * BLOCK_SIZE )
    {
        tLightIndexList[tLightIndexStartOffset + k] = tLightList[k];
    }

    
}
