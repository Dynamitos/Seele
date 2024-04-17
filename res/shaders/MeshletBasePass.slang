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
groupshared float4x4 localToView;
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
		localToView = mul(pViewParams.viewMatrix, instance.transformMatrix);
		float3 origin = float3(0, 0, 0);
		float3 corners[4] = {
			screenToView(float4(0.0f, 0.0f, -1.0f, 1.0f)).xyz,
			screenToView(float4(pViewParams.screenDimensions.x, 0.0f, -1.0f, 1.0f)).xyz,
			screenToView(float4(0.0f, pViewParams.screenDimensions.y, -1.0f, 1.0f)).xyz,
			screenToView(float4(pViewParams.screenDimensions, -1.0f, 1.0f)).xyz
		};
		viewFrustum.sides[0] = computePlane(origin, corners[2], corners[0]);
		viewFrustum.sides[1] = computePlane(origin, corners[1], corners[3]);
		viewFrustum.sides[2] = computePlane(origin, corners[0], corners[1]);
		viewFrustum.sides[3] = computePlane(origin, corners[3], corners[2]);
	}
	GroupMemoryBarrierWithGroupSync();
	MeshData mesh = pScene.meshData[groupID];
	for(uint i = threadID; i < MAX_MESHLETS_PER_MESH; i += TASK_GROUP_SIZE)
	{
		if(i < mesh.numMeshlets)
		{
			uint m = mesh.meshletOffset + i;
			MeshletDescription meshlet = pScene.meshletInfos[m];
			if(meshlet.bounding.insideFrustum(localToView, viewFrustum))
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

uint unpackPrimitiveIndices(uint index)
{
  uint32_t packed = pScene.primitiveIndices[index / 4];
  return (packed >> (index % 4)) & 0xff;
}

[numthreads(MESH_GROUP_SIZE, 1, 1)]
[outputtopology("triangle")]
[shader("mesh")]
void meshMain(
	in uint threadID: SV_GroupIndex,
	in uint groupID: SV_GroupID,
	in payload MeshPayload meshPayload,
	OutputVertices<FragmentParameter, MAX_VERTICES> vertices, 
	OutputIndices<uint3, MAX_PRIMITIVES> indices
){
	InstanceData inst = pScene.instances[meshPayload.instanceId[groupID]];
	MeshData md = pScene.meshData[meshPayload.instanceId[groupID]];
	MeshletDescription m = pScene.meshletInfos[meshPayload.meshletId[groupID]];
	SetMeshOutputCounts(m.vertexCount, m.primitiveCount);
	
	for(uint i = threadID; i < MAX_PRIMITIVES; i += MESH_GROUP_SIZE)
	{
		uint p = min(i, m.primitiveCount - 1);
		{
			uint local_idx0 = unpackPrimitiveIndices(m.primitiveOffset + (p * 3) + 0);
			uint local_idx1 = unpackPrimitiveIndices(m.primitiveOffset + (p * 3) + 1);
			uint local_idx2 = unpackPrimitiveIndices(m.primitiveOffset + (p * 3) + 2);
			indices[p] = uint3(local_idx0, local_idx1, local_idx2);
		}
	}
	for(uint i = threadID; i < MAX_VERTICES; i+=MESH_GROUP_SIZE)
	{
		uint v = min(i, m.vertexCount - 1);
		{
			uint vertexIndex = pScene.vertexIndices[m.vertexOffset + v];
			VertexAttributes attr = pVertexData.getAttributes(md.indicesOffset + vertexIndex);
			vertices[v] = attr.getParameter(inst.transformMatrix);
			vertices[v].vertexColor = m.color;
		}
	}
}