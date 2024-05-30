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
    MeshData mesh = pScene.meshData[pOffsets.instanceOffset + groupID];
    if (threadID == 0)
    {
        head = 0;
        p.instanceId = pOffsets.instanceOffset + groupID;
        p.meshletOffset = mesh.meshletOffset;
        p.cullingOffset = pScene.cullingOffsets[p.instanceId];
    }
    GroupMemoryBarrierWithGroupSync();
    for (uint i = threadID; i < mesh.numMeshlets; i += TASK_GROUP_SIZE)
    {
        uint m = p.meshletOffset + i;
        uint cull = p.cullingOffset + i;
        MeshletDescription meshlet = pScene.meshletInfos[m];
        MeshletCullingInfo culling = pScene.culledMeshlets[cull];
        if(false)
        {
            uint index;
            InterlockedAdd(head, 1, index);
            p.culledMeshlets[index] = i;
        }
    }
    GroupMemoryBarrierWithGroupSync();
    DispatchMesh(head, 1, 1, p);
}
