import Common;
import Scene;

groupshared MeshPayload p;
groupshared uint head;

[numthreads(TASK_GROUP_SIZE, 1, 1)]
[shader("amplification")]
void taskMain(
	uint threadID: SV_GroupIndex,
	uint groupID: SV_GroupID,
){
	InstanceData instance = pScene.instances[pOffsets.instanceOffset + groupID];
	MeshData mesh = pScene.meshData[pOffsets.instanceOffset + groupID];
	//head = 0;
	//GroupMemoryBarrierWithGroupSync();
	//for(uint i = threadID; i < mesh.numMeshlets; i += TASK_GROUP_SIZE)
	//{
	//	uint m = mesh.meshletOffset + i;
	//	MeshletDescription meshlet = pScene.meshletInfos[m];
	//	MeshletCullingInfo culling = pScene.culledMeshlets[instance.meshletCullingOffset];
	//	if(culling.anyVisible())
	//	{
	//		uint index;
	//		InterlockedAdd(head, 1, index);
	//		p.culledMeshlets[index] = m;
	//	}
	//}
  //GroupMemoryBarrierWithGroupSync();
  //DispatchMesh(head, 1, 1, p);
}
