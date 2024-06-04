import Common;
import Scene;

struct VisibilityCullingData
{
    Texture2D<uint> visibilityTexture;
    RWStructuredBuffer<MeshletCullingInfo> cullingInfos;
};
ParameterBlock<VisibilityCullingData> pVisibilityParams;

groupshared MeshletCullingInfo cullInfo;

[numthreads(BLOCK_SIZE, 1, 1)]
[shader("compute")]
void computeMain(
    uint threadID: SV_GroupIndex,
    uint groupID: SV_GroupID,
){
    if (threadID < MAX_PRIMITIVES / 32)
    {
        cullInfo.visible[threadID] = 0;
    }
    GroupMemoryBarrierWithGroupSync();
    for (uint y = 0; y < pViewParams.screenDimensions.y; y++)
    {
        for (uint x = threadID; x < pViewParams.screenDimensions.x; x += BLOCK_SIZE)
        {
            int3 texCoords = int3(x, y, 0);
            uint encoded = pVisibilityParams.visibilityTexture.Load(texCoords).r;
            uint2 decoded = decodePrimitive(encoded);
            uint base = decoded.y == groupID ? 1 : 0;
            uint arrIdx = decoded.x / 32;
            uint bit = decoded.x % 32;
            cullInfo.visible[arrIdx] |= (base << bit);
        }
    }
    GroupMemoryBarrierWithGroupSync();
    if (threadID < MAX_PRIMITIVES / 32)
    {
        pVisibilityParams.cullingInfos[groupID].visible[threadID] = cullInfo.visible[threadID];
    }
}