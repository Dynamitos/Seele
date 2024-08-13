import Common;
import Scene;
import WaterCommon;

groupshared WaterPayload p;

[numthreads(1, 1, 1)]
[shader("amplification")]
void taskMain(
    uint threadID: SV_GroupThreadID,
    uint groupID: SV_GroupID, )
{
    p = pWaterMaterial.tiles[groupID];
    DispatchMesh(1, 1, 1, p);
}
