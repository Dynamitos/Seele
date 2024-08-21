import Bounding;
import Common;
import Scene;
import WaterCommon;


[numthreads(1, 1, 1)]
[shader("amplification")]
void taskMain(
    uint threadID: SV_GroupThreadID,
    uint groupID: SV_GroupID, )
{
    WaterTile tile = pWaterMaterial.tiles[groupID];
    GroupMemoryBarrierWithGroupSync();
    AABB bounding;
    bounding.minCorner = float3(tile.location.x, tile.height, tile.location.y) * tile.extent;
    bounding.maxCorner = float3(tile.location.x + 1, tile.height, tile.location.y + 1) * tile.extent;
    float3 median = (bounding.minCorner + bounding.maxCorner) / 2;
    float distance = distance(median, pViewParams.cameraPos_WS.xyz);
    uint numMeshes = max(4 - uint(distance / tile.extent), 1);
    WaterPayload payload;
    payload.offset = bounding.minCorner;
    payload.extent = tile.extent / numMeshes;
    DispatchMesh(numMeshes, numMeshes, 1, payload);
}
