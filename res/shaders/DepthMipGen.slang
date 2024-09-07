import Common;
import DepthCommon;

ParameterBlock<DepthData> pDepthAttachment;

struct MipParam
{
    uint sourceOffset;
    uint destOffset;
    uint2 sourceDim;
    uint2 destDim;
}
layout(push_constant)
ConstantBuffer<MipParam> pMipParam;

float readBuffer(uint2 pos)
{
    if(pos.x >= pMipParam.sourceDim.x || pos.y >= pMipParam.sourceDim.y)
    {
        return 1.0f;
    }
    return pDepthAttachment.buffer[pMipParam.sourceOffset + pos.x + (pos.y * pMipParam.sourceDim.x)];
}

[shader("compute")]
[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void reduceLevel(
    uint2 dispatchID: SV_DispatchThreadID,
) {
    if(dispatchID.x >= pMipParam.destDim.x || dispatchID.y >= pMipParam.destDim.y)
    {
        return;
    }
    uint2 readOffset = dispatchID * 2;
    float d0 = readBuffer(readOffset + uint2(0, 0));
    float d1 = readBuffer(readOffset + uint2(0, 1));
    float d2 = readBuffer(readOffset + uint2(1, 0));
    float d3 = readBuffer(readOffset + uint2(1, 1));
    pDepthAttachment.buffer[pMipParam.destOffset + dispatchID.x + (dispatchID.y * pMipParam.destDim.x)] = min(min(d0, d1), min(d2, d3));
}

// each thread reduces the a pixel indivdually
// used for mip levels where the read size is smaller than
// the group dimensions
[shader("compute")]
[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void sourceCopy(
    uint2 dispatchID: SV_DispatchThreadID
) {
    if(dispatchID.x >= pViewParams.screenDimensions.x || dispatchID.y >= pViewParams.screenDimensions.y)
    {
        return;
    }
    pDepthAttachment.buffer[dispatchID.x + (dispatchID.y * uint(pViewParams.screenDimensions.x))] = pDepthAttachment.texture[uint2(dispatchID)];
}
