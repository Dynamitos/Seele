import Common;
import Scene;

struct VisibilityCullingData
{
    Texture2D<uint> visibilityTexture;
    RWStructuredBuffer<MeshletCullingInfo> cullingInfos;
};
ParameterBlock<VisibilityCullingData> pVisibilityParams;

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
[shader("compute")]
void computeMain(
    uint3 dispatchThreadID: SV_DispatchThreadID,
){
    int3 texCoords = int3(dispatchThreadID.xy, 0);
    uint encoded = pVisibilityParams.visibilityTexture.Load(texCoords).r;
    uint meshletId = decodePrimitive(encoded);
    pVisibilityParams.cullingInfos[meshletId].visible = 1;
}