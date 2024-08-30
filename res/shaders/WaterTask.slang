import Bounding;
import Common;
import Scene;
import WaterCommon;

[numthreads(1, 1, 1)]
[shader("amplification")]
void taskMain(
    uint threadID: SV_GroupThreadID,
    uint3 groupID: SV_GroupID
    ) {
    WaterTile tile = pWaterMaterial.tiles[groupID.x];
    GroupMemoryBarrierWithGroupSync();
    AABB bounding;
    bounding.minCorner = float3(tile.location.x, tile.height, tile.location.y) * tile.extent;
    bounding.maxCorner = float3(tile.location.x + 1, tile.height, tile.location.y + 1) * tile.extent;
    float3 median = (bounding.minCorner + bounding.maxCorner) / 2;
    float distance = distance(median, pViewParams.cameraPos_WS.xyz);
    float tileDistance = distance / tile.extent;
    uint numMeshes = groupID.y + 1;
    
    if(numMeshes == 4 && tileDistance > 2)
    {
        return;
    }
    if(numMeshes == 3 && (tileDistance > 4 || tileDistance < 1))
    {
        return;
    }
    if(numMeshes == 2 && (tileDistance > 8 || tileDistance < 3))
    {
        return;
    }
    if(numMeshes == 1 && tileDistance < 7)
    {
        return;
    }
    WaterPayload payload;
    payload.offset = bounding.minCorner;
    payload.extent = tile.extent / numMeshes;
    payload.numMeshes = numMeshes;
    DispatchMesh(numMeshes, numMeshes, 1, payload);
}
