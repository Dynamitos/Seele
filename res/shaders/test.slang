import Common;
import BRDF;
import Meshlet;
import Scene;
import VertexData;
import StaticMeshVertexData;

layout(push_constant)
ConstantBuffer<uint> meshId;

struct MeshShaderPayload
{
	uint instanceId;
	uint meshletId;
};

struct PrimitiveAttributes
{
	uint cull: SV_CullPrimitive;
};

groupshared StaticMeshVertexAttributes gs_vertices[MAX_VERTICES];
groupshared uint3 gs_indices[MAX_PRIMITIVES];
groupshared uint gs_numVertices;
groupshared uint gs_numPrimitives;

[numthreads(GROUP_SIZE, 1, 1)]
[shader("amplification")]
void taskMain(
	uint3 threadID: SV_GroupIndex,
	uint3 groupID: SV_GroupID
){
	
}

[numthreads(GROUP_SIZE, 1, 1)]
[outputtopology("triangle")]
[shader("mesh")]
void meshMain(
	uint threadID: SV_GroupIndex,
	uint3 groupID: SV_GroupID,
	out Vertices<StaticMeshVertexAttributes, MAX_VERTICES> vertices, 
	out Indices<uint3, MAX_PRIMITIVES> indices
){
	InstanceData inst = scene.instances[threadID];
	MeshletDescription m = meshlets.meshletInfos[groupID.x];
	const uint vertexLoops = (MAX_VERTICES + GROUP_SIZE - 1) / GROUP_SIZE;
	for(uint loop = 0; loop < vertexLoops; ++loop) 
	{
		uint v = threadID + loop * GROUP_SIZE;
		v = min(v, m.vertexCount - 1);
		InterlockedMax(gs_numVertices, v + 1);
		{
			int vertexIndex = meshlets.vertexIndices[m.vertexOffset + v];
			gs_vertices[v] = vertexData.getAttributes(vertexIndex, inst);
		}
	}

	const uint primitiveLoops = (MAX_PRIMITIVES + GROUP_SIZE - 1) / GROUP_SIZE;
	for(uint loop = 0; loop < primitiveLoops; ++loop)
	{
		uint p = threadID + loop * GROUP_SIZE;
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
	for(uint loop1 = 0; loop1 < vertexLoops; ++loop1) 
	{
		uint v = threadID + loop1 * GROUP_SIZE;
		vertices[v] = gs_vertices[v];
	}
	for(uint loop2 = 0; loop2 < primitiveLoops; ++loop2)
	{
		uint p = threadID + loop2 * GROUP_SIZE;
		indices[p] = gs_indices[p];
	}
}