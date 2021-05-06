import LightEnv;
import Common;

struct ComputeShaderInput
{
	uint3 groupID : SV_GroupID;
	uint3 groupThreadID : SV_GroupThreadID;
	uint3 dispatchThreadID : SV_DispatchThreadID;
	uint groupIndex : SV_GroupIndex;
};
layout(binding = 0)
cbuffer DispatchParams
{
	uint3 numThreadGroups;
	uint pad0;
	uint3 numThreads;
	uint pad1;
}

layout(binding = 2)
RWTexture2D depthTextureVS;
layout(binding = 4)
StructuredBuffer<Frustum> frustums;

layout(binding = 5)
RWStructuredBuffer<uint> oLightIndexCounter;
layout(binding = 6)
RWStructuredBuffer<uint> tLightIndexCounter;

layout(binding = 7)
RWStructuredBuffer<uint> oLightIndexList;
layout(binding = 8)
RWStructuredBuffer<uint> tLightIndexList;
layout(binding = 9)
RWTexture2D<uint2> oLightGrid;
layout(binding = 10)
RWTexture2D<uint2> tLightGrid;

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
void cullLights(ComputeShaderInput in)
{
	int2 texCoord = in.dispatchThreadID.xy;
	float fDepth = depthTextureVS.Load(texCoord).r;

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

	float minDepthVS = clipToView(float4(0, 0, fMinDepth, 1)).z;
	float maxDepthVS = clipToView(float4(0, 0, fMaxDepth, 1)).z;
	float nearClipVS = clipToView(float4(0, 0, 0, 1.0f)).z;

	Plane minPlane = {float3(0, 0, -1), -minDepthVS};

    for ( uint i = in.groupIndex; i < lights.numPointLights; i += BLOCK_SIZE * BLOCK_SIZE )
	{
        PointLight light = lights.pointLights[i];
		//if(light.insideFrustum(groupFrustum, nearClipVS, maxDepthVS))
		{
			//InterlockedAdd(tLightCount, 1, index);
            //if(index < 1024)
            //{
            //    tLightList[index] = i;
            //}
            //if(!light.insidePlane(minPlane))
			//{
            oAppendLight(i);
			//}
		}
    }
	
	GroupMemoryBarrierWithGroupSync();

	if(in.groupIndex == 0)
	{
		InterlockedAdd(oLightIndexCounter[0], (uint)oLightCount, oLightIndexStartOffset);
		oLightGrid[in.groupID.xy] = uint2(oLightIndexStartOffset, oLightCount);

		InterlockedAdd(tLightIndexCounter[0], (uint)tLightCount, tLightIndexStartOffset);
		tLightGrid[in.groupID.xy] = uint2(tLightIndexStartOffset, tLightCount);
	}
	GroupMemoryBarrierWithGroupSync();

    if(in.groupIndex == 0)
    {
        for (uint j = 0; j < (uint)oLightCount; j += 1/*BLOCK_SIZE * BLOCK_SIZE*/)
	    {
		    oLightIndexList[oLightIndexStartOffset + j] = oLightList[j];
        }
    }
    
	
	// For transparent geometry.
	for ( uint k = in.groupIndex; k < (uint)tLightCount; k += BLOCK_SIZE * BLOCK_SIZE )
	{
		tLightIndexList[tLightIndexStartOffset + k] = tLightList[k];
	}
}