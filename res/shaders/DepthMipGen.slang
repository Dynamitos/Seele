import Common;
import DepthCommon;

ParameterBlock<DepthData> pDepthAttachment;

struct MipParam
{
    uint srcMipOffset;
    uint dstMipOffset;
    uint2 srcMipDim;
    uint2 dstMipDim;
}
layout(push_constant)
ConstantBuffer<MipParam> pMipParam;

float getSrcDepth(uint2 pos)
{
    return pDepthAttachment.buffer[pMipParam.srcMipOffset + pos.x + (pos.y * pMipParam.srcMipDim.x)];
}

void setDstDepth(uint2 pos, float depth)
{
    pDepthAttachment.buffer[pMipParam.dstMipOffset + pos.x + (pos.y * pMipParam.dstMipDim.x)] = depth;
}

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
[shader("compute")]
void reduceLevel(
    uint3 threadID: SV_GroupThreadID,
    uint3 groupID: SV_GroupID,
){
    uint2 minCoords = (groupID.xy * BLOCK_SIZE + threadID.xy) * 2;
    
    if(minCoords.x >= pMipParam.srcMipDim.x || minCoords.y >= pMipParam.srcMipDim.y)
    {
        return;
    }
    
    uint2 maxCoords = uint2(min(minCoords.x + 1, pMipParam.srcMipDim.x - 1), min(minCoords.y + 1, pMipParam.srcMipDim.y - 1));

    float d0 = getSrcDepth(uint2(minCoords.x, minCoords.y));
    float d1 = getSrcDepth(uint2(maxCoords.x, minCoords.y));
    float d2 = getSrcDepth(uint2(minCoords.x, maxCoords.y));
    float d3 = getSrcDepth(uint2(maxCoords.x, maxCoords.y));

    float minDepth = min(min(d0, d1), min(d2, d3));
    setDstDepth(minCoords / 2, minDepth);
}

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
[shader("compute")]
void initialReduce(
    uint3 threadID: SV_GroupThreadID,
    uint3 groupID: SV_GroupID,
) {
    uint width = uint(pViewParams.screenDimensions.x);
    int2 groupOffset = groupID.xy * BLOCK_SIZE;
    int2 texCoord = groupOffset + threadID.xy;
    float fDepth = pDepthAttachment.texture.Load(int3(texCoord, 0)).r;
    pDepthAttachment.buffer[texCoord.x + (texCoord.y * width)] = fDepth;
}
