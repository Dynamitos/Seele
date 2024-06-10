import Common;
import Scene;

groupshared MeshPayload p;
groupshared uint head;
groupshared MeshData mesh;
groupshared InstanceData instance;
groupshared Frustum viewFrustum;

ParameterBlock<Texture2D> pDepthAttachment;

[numthreads(TASK_GROUP_SIZE, 1, 1)]
[shader("amplification")]
void taskMain(
    uint threadID: SV_GroupThreadID,
    uint groupID: SV_GroupID, )
{
    if (threadID == 0)
    {
        head = 0;
	    instance = pScene.instances[pOffsets.instanceOffset + groupID];
        mesh = pScene.meshData[pOffsets.instanceOffset + groupID];
        p.instanceId = pOffsets.instanceOffset + groupID;
        p.meshletOffset = mesh.meshletOffset;
        p.cullingOffset = pScene.cullingOffsets[p.instanceId];
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
    }
    GroupMemoryBarrierWithGroupSync();
    for (uint i = threadID; i < mesh.numMeshlets; i += TASK_GROUP_SIZE)
    {
        uint m = p.meshletOffset + i;
        uint cull = p.cullingOffset + i;
        MeshletDescription meshlet = pScene.meshletInfos[m];
        MeshletCullingInfo culling = pScene.cullingInfos[cull];
        // if any triangle was visible last frame, it was drawn by the cached pass already
        if(!culling.anyVisible())
        {
#ifdef VIEW_CULLING
		    if(meshlet.bounding.insideFrustum(viewFrustum))
#endif
            {
                uint index;
                InterlockedAdd(head, 1, index);
                p.culledMeshlets[index] = i;
            }
        }
    }
    GroupMemoryBarrierWithGroupSync();
    DispatchMesh(head, 1, 1, p);
}
