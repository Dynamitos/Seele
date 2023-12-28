import Common;
import BRDF;
import Scene;
import VertexData;
import MaterialParameter;

struct MeshPayload
{
	uint instanceId[MAX_MESHLETS_PER_MESH];
	uint meshletId[MAX_MESHLETS_PER_MESH];
};

groupshared MeshPayload p;
groupshared uint head;
groupshared float4x4 localToClip;
groupshared Frustum viewFrustum;

[numthreads(TASK_GROUP_SIZE, 1, 1)]
[outputtopology("triangle")]
[shader("amplification")]
void taskMain(
	uint threadID: SV_GroupIndex,
	uint groupID: SV_GroupID
){
	InstanceData instance = pScene.instances[groupID];
	if(threadID == 0)
	{
		head = 0;
		localToClip = mul(pViewParams.projectionMatrix, mul(pViewParams.viewMatrix, instance.transformMatrix));
		// Left
		viewFrustum.sides[0].n = float3(1, 0, 0);
		viewFrustum.sides[0].d = -1;
		// Right
		viewFrustum.sides[1].n = float3(-1, 0, 0);
		viewFrustum.sides[1].d = -1;
		// Top
		viewFrustum.sides[2].n = float3(0, -1, 0);
		viewFrustum.sides[2].d = -1;
		// Bottom
		viewFrustum.sides[3].n = float3(0, 1, 0);
		viewFrustum.sides[3].d = -1;
		// Base
		viewFrustum.basePlane.n = float3(0, 0, -1);
		viewFrustum.basePlane.d = 0;
	}
	GroupMemoryBarrierWithGroupSync();
	MeshData mesh = pScene.meshData[groupID];
	for(uint i = threadID; i < MAX_MESHLETS_PER_MESH; i += TASK_GROUP_SIZE)
	{
		if(i < mesh.numMeshlets)
		{
			uint m = mesh.meshletOffset + i;
			MeshletDescription meshlet = pScene.meshletInfos[m];
			if(meshlet.boundingBox.insideFrustum(localToClip, viewFrustum))
			{
				uint index;
				InterlockedAdd(head, 1, index);
				p.meshletId[index] = m;
				p.instanceId[index] = groupID;
			}
		}
	}
	GroupMemoryBarrierWithGroupSync();
	DispatchMesh(head, 1, 1, p);
}

struct PrimitiveAttributes
{
	uint cull: SV_CullPrimitive;
};

[numthreads(MESH_GROUP_SIZE, 1, 1)]
[outputtopology("triangle")]
[shader("mesh")]
void meshMain(
	in uint threadID: SV_GroupIndex,
	in uint groupID: SV_GroupID,
	in payload MeshPayload meshPayload,
	out Vertices<FragmentParameter, MAX_VERTICES> vertices, 
	out Indices<uint3, MAX_PRIMITIVES> indices
){
	InstanceData inst = pScene.instances[meshPayload.instanceId[groupID]];
	MeshData md = pScene.meshData[meshPayload.instanceId[groupID]];
	MeshletDescription m = pScene.meshletInfos[meshPayload.meshletId[groupID]];
	SetMeshOutputCounts(m.vertexCount, m.primitiveCount);
	
	uint p = min(threadID, m.primitiveCount - 1);
	{
		uint local_idx0 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 0];
		uint local_idx1 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 1];
		uint local_idx2 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 2];
		indices[p] = uint3(local_idx0, local_idx1, local_idx2);
	}
	p = min(threadID + 32, m.primitiveCount - 1);
	{
		uint local_idx0 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 0];
		uint local_idx1 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 1];
		uint local_idx2 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 2];
		indices[p] = uint3(local_idx0, local_idx1, local_idx2);
	}
	p = min(threadID + 64, m.primitiveCount - 1);
	{
		uint local_idx0 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 0];
		uint local_idx1 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 1];
		uint local_idx2 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 2];
		indices[p] = uint3(local_idx0, local_idx1, local_idx2);
	}
	p = min(threadID + 96, m.primitiveCount - 1);
	{
		uint local_idx0 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 0];
		uint local_idx1 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 1];
		uint local_idx2 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 2];
		indices[p] = uint3(local_idx0, local_idx1, local_idx2);
	}
	
	GroupMemoryBarrierWithGroupSync();
	for(uint i = threadID; i < MAX_VERTICES; i+=MESH_GROUP_SIZE)
	{
		uint v = min(i, m.vertexCount - 1);
		{
			uint vertexIndex = pScene.vertexIndices[m.vertexOffset + v];
			VertexAttributes attr = pVertexData.getAttributes(md.indicesOffset + vertexIndex);
			attr.vertexColor = m.color;
			vertices[v] = attr.getParameter(inst.transformMatrix);
		}
	}
}