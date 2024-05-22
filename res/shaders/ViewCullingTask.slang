import Common;
import Scene;

groupshared MeshPayload p;
groupshared uint head;
groupshared Frustum viewFrustum;

[numthreads(TASK_GROUP_SIZE, 1, 1)]
[shader("amplification")]
void taskMain(
	uint threadID: SV_GroupIndex,
	uint groupID: SV_GroupThreadID
){
	InstanceData instance = pScene.instances[pOffsets.instanceOffset + groupID];
	MeshData mesh = pScene.meshData[pOffsets.instanceOffset + groupID];
	if(threadID == 0)
	{
		head = 0;
		float3 origin = viewToModel(instance.inverseTransformMatrix, float4(0, 0, 0, 1)).xyz;
		const float offset = 0.0f;
		float3 corners[4] = {
			screenToModel(instance.inverseTransformMatrix, float4(offset, offset, -1.0f, 1.0f)).xyz,
			screenToModel(instance.inverseTransformMatrix, float4(pViewParams.screenDimensions.x - offset, offset, -1.0f, 1.0f)).xyz,
			screenToModel(instance.inverseTransformMatrix, float4(offset, pViewParams.screenDimensions.y - offset, -1.0f, 1.0f)).xyz,
			screenToModel(instance.inverseTransformMatrix, float4(pViewParams.screenDimensions - float2(offset, offset), -1.0f, 1.0f)).xyz
		};
		viewFrustum.sides[0] = computePlane(origin, corners[2], corners[0]);
		viewFrustum.sides[1] = computePlane(origin, corners[1], corners[3]);
		viewFrustum.sides[2] = computePlane(origin, corners[0], corners[1]);
		viewFrustum.sides[3] = computePlane(origin, corners[3], corners[2]);
		//p.instanceId = pOffsets.instanceOffset + groupID;
		p.instanceData = instance;
		p.meshData = mesh;
		//p.cullingOffset = pScene.cullingOffsets[pOffsets.cullingCounterOffset + groupID];
	}
	GroupMemoryBarrierWithGroupSync();
	//for(uint i = threadID; i < mesh.numMeshlets; i += TASK_GROUP_SIZE)
	//{
	//	uint m = mesh.meshletOffset + i;
	//	MeshletDescription meshlet = pScene.meshletInfos[m];
	//	//if(meshlet.bounding.insideFrustum(viewFrustum))
	//	{
	//		uint index;
	//		InterlockedAdd(head, 1, index);
	//		p.culledMeshlets[index] = m;
	//		//pScene.culledMeshlets[p.cullingOffset + index] = m;
	//	}
	//}
	//GroupMemoryBarrierWithGroupSync();
	DispatchMesh(mesh.numMeshlets, 1, 1, p);
}