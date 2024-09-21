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
        mesh.numMeshlets = 1; //= pScene.meshData[0]; // pOffsets.instanceOffset + groupID];
        mesh.meshletOffset = 0;
        p.instanceId = 0;//pOffsets.instanceOffset + groupID;
        p.meshletOffset = mesh.meshletOffset;
        p.cullingOffset = 0;//pScene.cullingOffsets[p.instanceId];
    }
    GroupMemoryBarrierWithGroupSync();
    for (uint i = threadID; i < mesh.numMeshlets; i += TASK_GROUP_SIZE)
    {
        uint m = p.meshletOffset + i;
        uint cull = p.cullingOffset + i;
        //MeshletDescription meshlet = pScene.meshletInfos[m];
        //MeshletCullingInfo culling = pScene.cullingInfos[cull];
        //if(culling.wasVisible())
        {
            //uint index;
            //InterlockedAdd(head, 1, index);
            p.culledMeshlets[0] = 0;
        }
    }
    GroupMemoryBarrierWithGroupSync();
    DispatchMesh(head, 1, 1, p);
}
