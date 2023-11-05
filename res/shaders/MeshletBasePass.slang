import Common;
import BRDF;
import Meshlet;
import Scene;
import VertexData;

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
	InstanceData instance = scene.instances[groupID];
	if(threadID == 0)
	{
		head = 0;
		localToClip = mul(viewParams.projectionMatrix, mul(viewParams.viewMatrix, instance.transformMatrix));
		// Left
		viewFrustum.sides[0].n = float3(1, 0, 0);
		viewFrustum.sides[0].d = -1;
		// Right
		viewFrustum.sides[1].n = float3(-1, 0, 0);
		viewFrustum.sides[1].d = 1;
		// Top
		viewFrustum.sides[2].n = float3(0, -1, 0);
		viewFrustum.sides[2].d = 1;
		// Bottom
		viewFrustum.sides[1].n = float3(0, 1, 0);
		viewFrustum.sides[1].d = -1;
		// Base
		viewFrustum.basePlane.n = float3(0, 0, 1);
		viewFrustum.basePlane.d = 0;
	}
	GroupMemoryBarrierWithGroupSync();
	MeshData mesh = scene.meshData[groupID];
	for(uint i = threadID; i < MAX_MESHLETS_PER_MESH; i += TASK_GROUP_SIZE)
	{
		uint m = mesh.meshletOffset + min(mesh.numMeshlets, i);
		MeshletDescription meshlet = scene.meshlets.meshletInfos[m];
		//if(meshlet.boundingBox.insideFrustum(localToClip, viewFrustum))
		{
			uint index;
			InterlockedAdd(head, 1, index);
			p.meshletId[index] = m;
			p.instanceId[index] = groupID;
		}
	}
	GroupMemoryBarrierWithGroupSync();
	DispatchMesh(head, 1, 1, p);
}

groupshared VertexAttributes gs_vertices[MAX_VERTICES];
groupshared uint3 gs_indices[MAX_PRIMITIVES];
groupshared uint gs_numVertices;
groupshared uint gs_numPrimitives;

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
	out Vertices<VertexAttributes, MAX_VERTICES> vertices, 
	out Indices<uint3, MAX_PRIMITIVES> indices
){
	InstanceData inst = scene.instances[meshPayload.instanceId[groupID]];
	MeshData md = scene.meshData[meshPayload.instanceId[groupID]];
	MeshletDescription m = meshlets.meshletInfos[meshPayload.meshletId[groupID]];
	const uint vertexLoops = (MAX_VERTICES + MESH_GROUP_SIZE - 1) / MESH_GROUP_SIZE;
	for(uint loop = 0; loop < vertexLoops; ++loop) 
	{
		uint v = threadID + loop * MESH_GROUP_SIZE;
		v = min(v, m.vertexCount - 1);
		InterlockedMax(gs_numVertices, v + 1);
		{
			int vertexIndex = meshlets.vertexIndices[m.vertexOffset + v];
			gs_vertices[v] = vertexData.getAttributes(md.indexOffset + vertexIndex, inst);
		}
	}

	const uint primitiveLoops = (MAX_PRIMITIVES + MESH_GROUP_SIZE - 1) / MESH_GROUP_SIZE;
	for(uint loop = 0; loop < primitiveLoops; ++loop)
	{
		uint p = threadID + loop * MESH_GROUP_SIZE;
		p = min(p, m.primitiveCount - 1);
		InterlockedMax(gs_numPrimitives, p + 1);
		{
			uint8_t local_idx0 = meshlets.primitiveIndices[m.primitiveOffset + (p * 3) + 0];
			uint8_t local_idx1 = meshlets.primitiveIndices[m.primitiveOffset + (p * 3) + 1];
			uint8_t local_idx2 = meshlets.primitiveIndices[m.primitiveOffset + (p * 3) + 2];
			uint32_t idx0 = meshlets.vertexIndices[m.vertexOffset + local_idx0];
			uint32_t idx1 = meshlets.vertexIndices[m.vertexOffset + local_idx1];
			uint32_t idx2 = meshlets.vertexIndices[m.vertexOffset + local_idx2];
			gs_indices[p * 3 + 0] = idx0;
			gs_indices[p * 3 + 1] = idx1;
			gs_indices[p * 3 + 2] = idx2;
		}
	}
	GroupMemoryBarrierWithGroupSync();
	SetMeshOutputCounts(gs_numVertices, gs_numPrimitives);
	GroupMemoryBarrierWithGroupSync();
	
	uint v = threadID;
	v = min(v, m.vertexCount - 1);
	vertices[v] = gs_vertices[v];
	if(vertexLoops >= 1)
	{
		uint v = threadID + MESH_GROUP_SIZE;
		v = min(v, m.vertexCount - 1);
		vertices[v] = gs_vertices[v];	
	}

	uint p = threadID;
	p = min(p, m.primitiveCount - 1);
	indices[p] = gs_indices[p];
	
	if(primitiveLoops >= 1)
	{
		uint p = threadID + MESH_GROUP_SIZE;
		p = min(p, m.primitiveCount - 1);
		indices[p] = gs_indices[p];
	}
	if(primitiveLoops >= 2)
	{
		uint p = threadID + 2 * MESH_GROUP_SIZE;
		p = min(p, m.primitiveCount - 1);
		indices[p] = gs_indices[p];
	}
	if(primitiveLoops >= 3)
	{
		uint p = threadID + 3 * MESH_GROUP_SIZE;
		p = min(p, m.primitiveCount - 1);
		indices[p] = gs_indices[p];
	}
}