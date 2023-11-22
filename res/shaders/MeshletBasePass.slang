import Common;
import BRDF;
import Meshlet;
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
	MeshData mesh = pScene.meshData[groupID];
	for(uint i = threadID; i < MAX_MESHLETS_PER_MESH; i += TASK_GROUP_SIZE)
	{
		if(i < mesh.numMeshlets)
		{
			uint m = mesh.meshletOffset + i;
			MeshletDescription meshlet = pScene.meshletInfos[m];
			//if(meshlet.boundingBox.insideFrustum(localToClip, viewFrustum))
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

groupshared VertexAttributes gs_vertices[MAX_VERTICES];
groupshared uint3 gs_indices[MAX_PRIMITIVES];
groupshared uint gs_numVertices;
groupshared uint gs_numPrimitives;

struct PrimitiveAttributes
{
	uint cull: SV_CullPrimitive;
};

struct MeshOutput
{
	FragmentParameter parameter : PARAMETER;
	float4 position_CS : SV_Position;
};

[numthreads(MESH_GROUP_SIZE, 1, 1)]
[outputtopology("triangle")]
[shader("mesh")]
void meshMain(
	in uint threadID: SV_GroupIndex,
	in uint groupID: SV_GroupID,
	in payload MeshPayload meshPayload,
	out Vertices<MeshOutput, MAX_VERTICES> vertices, 
	out Indices<uint3, MAX_PRIMITIVES> indices
){
	InstanceData inst = pScene.instances[meshPayload.instanceId[groupID]];
	MeshData md = pScene.meshData[meshPayload.instanceId[groupID]];
	MeshletDescription m = pScene.meshletInfos[meshPayload.meshletId[groupID]];
	const uint vertexLoops = (MAX_VERTICES + MESH_GROUP_SIZE - 1) / MESH_GROUP_SIZE;
	for(uint loop = 0; loop < vertexLoops; ++loop) 
	{
		uint v = threadID + loop * MESH_GROUP_SIZE;
		v = min(v, m.vertexCount - 1);
		InterlockedMax(gs_numVertices, v + 1);
		{
			int vertexIndex = pScene.vertexIndices[m.vertexOffset + v];
			gs_vertices[v] = pVertexData.getAttributes(md.indicesOffset + vertexIndex, inst.transformMatrix);
		}
	}
	GroupMemoryBarrierWithGroupSync();
	const uint primitiveLoops = (MAX_PRIMITIVES + MESH_GROUP_SIZE - 1) / MESH_GROUP_SIZE;
	for(uint loop = 0; loop < primitiveLoops; ++loop)
	{
		uint p = threadID + loop * MESH_GROUP_SIZE;
		p = min(p, m.primitiveCount - 1);
		InterlockedMax(gs_numPrimitives, p + 1);
		{
			uint32_t local_idx0 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 0];
			uint32_t local_idx1 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 1];
			uint32_t local_idx2 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 2];
			gs_indices[p].x = local_idx0;
			gs_indices[p].y = local_idx1;
			gs_indices[p].z = local_idx2;
		}
	}
	GroupMemoryBarrierWithGroupSync();
	SetMeshOutputCounts(gs_numVertices, gs_numPrimitives);
	GroupMemoryBarrierWithGroupSync();
	
	uint v = threadID;
	v = min(v, m.vertexCount - 1);
	FragmentParameter parameter = gs_vertices[v].getParameter(inst.transformMatrix);
	vertices[v].parameter = parameter;
	vertices[v].position_CS = parameter.position_CS;
	if(vertexLoops >= 1)
	{
		uint v = threadID + MESH_GROUP_SIZE;
		v = min(v, m.vertexCount - 1);
		FragmentParameter parameter = gs_vertices[v].getParameter(inst.transformMatrix);
		vertices[v].parameter = parameter;
		vertices[v].position_CS = parameter.position_CS;
	}

	uint p = threadID;
	p = min(p, m.primitiveCount - 1);
	indices[p] = gs_indices[p];
	
	if(primitiveLoops >= 1)
	{
		p += MESH_GROUP_SIZE;
		p = min(p, m.primitiveCount - 1);
		indices[p] = gs_indices[p];
	}
	if(primitiveLoops >= 2)
	{
		p += MESH_GROUP_SIZE;
		p = min(p, m.primitiveCount - 1);
		indices[p] = gs_indices[p];
	}
	if(primitiveLoops >= 3)
	{
		p += MESH_GROUP_SIZE;
		p = min(p, m.primitiveCount - 1);
		indices[p] = gs_indices[p];
	}
}