import Common;
import Scene;

struct VisibilityCullingData
{
    Texture2D<uint> visibilityTexture;
    RWStructuredBuffer<MeshletCullingInfo> cullingInfos;
};
ParameterBlock<VisibilityCullingData> pVisibilityParams;

[numthreads(BLOCK_SIZE, 1, 1)]
[shader("compute")]
void computeMain(
    uint3 dispatchThreadID: SV_DispatchThreadID,
){
    //int3 texCoords = int3(dispatchThreadID.xy, 0);
    //uint encoded = pVisibilityParams.visibilityTexture.Load(texCoords).r;
    //uint2 decoded = decodePrimitive(encoded);
    //uint arrIdx = decoded.x / 32;
    //uint bit = decoded.x % 32;
    pVisibilityParams.cullingInfos[dispatchThreadID.x].visible[0] = 0x0;
    pVisibilityParams.cullingInfos[dispatchThreadID.x].visible[1] = 0x0;
    pVisibilityParams.cullingInfos[dispatchThreadID.x].visible[2] = 0x0;
    pVisibilityParams.cullingInfos[dispatchThreadID.x].visible[3] = 0x0;
    pVisibilityParams.cullingInfos[dispatchThreadID.x].visible[4] = 0x0;
    pVisibilityParams.cullingInfos[dispatchThreadID.x].visible[5] = 0x0;
    pVisibilityParams.cullingInfos[dispatchThreadID.x].visible[6] = 0x0;
    pVisibilityParams.cullingInfos[dispatchThreadID.x].visible[7] = 0x0;
}