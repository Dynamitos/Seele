import Common;
import Scene;

groupshared MeshPayload p;
groupshared uint head;
groupshared MeshData mesh;

[numthreads(TASK_GROUP_SIZE, 1, 1)]
[shader("amplification")]
void taskMain(
    uint threadID: SV_GroupThreadID,
    uint groupID: SV_GroupID, )
{
    if (threadID == 0)
    {
        head = 0;
        mesh = pScene.meshData[pOffsets.instanceOffset + groupID];
        p.instanceId = pOffsets.instanceOffset + groupID;
        p.meshletOffset = mesh.meshletRange.offset;
        p.cullingOffset = pScene.cullingOffsets[p.instanceId];
    }
    GroupMemoryBarrierWithGroupSync();
    for (uint i = threadID; i < mesh.meshletRange.size; i += TASK_GROUP_SIZE)
    {
        uint m = p.meshletOffset + i;
        uint cull = p.cullingOffset + i;
        MeshletDescription meshlet = pScene.meshletInfos[m];
        MeshletCullingInfo culling = pScene.cullingInfos[cull];
        if(culling.wasVisible())
        {
            uint index;
            InterlockedAdd(head, 1, index);
            p.culledMeshlets[index] = i;
        }
    }
    GroupMemoryBarrierWithGroupSync();
    DispatchMesh(head, 1, 1, p);
}
