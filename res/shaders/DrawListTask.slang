import Common;
import Scene;

groupshared MeshPayload p;
groupshared uint head;

[numthreads(TASK_GROUP_SIZE, 1, 1)]
[shader("amplification")]
void taskMain(
    uint threadID: SV_GroupIndex,
    uint groupID: SV_GroupID, )
{
    if (threadID == 0)
    {
        head = 0;
        p.instanceId = pOffsets.instanceOffset + groupID;
    }
    GroupMemoryBarrierWithGroupSync();
    MeshData mesh = pScene.meshData[p.instanceId];
    for (uint i = threadID; i < mesh.numMeshlets; i += TASK_GROUP_SIZE)
    {
        uint m = mesh.meshletOffset + i;
        //MeshletDescription meshlet = pScene.meshletInfos[m];
        //MeshletCullingInfo culling = pScene.culledMeshlets[pScene.cullingOffsets[p.instanceId] + i];
        //if(culling.anyVisible())
        {
            uint index;
            InterlockedAdd(head, 1, index);
            p.culledMeshlets[i] = m;
        }
    }
    GroupMemoryBarrierWithGroupSync();
    DispatchMesh(mesh.numMeshlets, 1, 1, p);
}
