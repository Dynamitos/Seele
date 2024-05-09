import Common;
import BRDF;
import Scene;
import VertexData;
import MaterialParameter;

struct PrimitiveAttributes
{
	uint cull: SV_CullPrimitive;
};

struct CulledMeshletList
{
	StructuredBuffer<uint> meshletIndices;
};
ParameterBlock<CulledMeshletList> pCullingList;

[numthreads(MESH_GROUP_SIZE, 1, 1)]
[outputtopology("triangle")]
[shader("mesh")]
void meshMain(
	in uint threadID: SV_GroupIndex,
	in uint groupID: SV_GroupID,
	out vertices FragmentParameter vertices[MAX_VERTICES], 
	out indices uint3 indices[MAX_PRIMITIVES]
){
	uint meshletIndex = pCullingList.meshletIndices[groupID];
	MeshletDescription m = pScene.meshletInfos[meshletIndex];
	SetMeshOutputCounts(m.vertexCount, m.primitiveCount);
	
	for(uint i = threadID; i < MAX_PRIMITIVES; i += MESH_GROUP_SIZE)
	{
		uint p = min(i, m.primitiveCount - 1);
		{
			uint local_idx0 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 0];
			uint local_idx1 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 1];
			uint local_idx2 = pScene.primitiveIndices[m.primitiveOffset + (p * 3) + 2];
			indices[p] = uint3(local_idx0, local_idx1, local_idx2);
		}
	}
	for(uint i = threadID; i < MAX_VERTICES; i+=MESH_GROUP_SIZE)
	{
		uint v = min(i, m.vertexCount - 1);
		{
			uint vertexIndex = pScene.vertexIndices[m.vertexOffset + v];
			VertexAttributes attr = pVertexData.getAttributes(m.indicesOffset + vertexIndex);
			attr.meshletId = meshletIndex;
			vertices[v] = attr.getParameter(float4x4(1.0));
		}
	}
}