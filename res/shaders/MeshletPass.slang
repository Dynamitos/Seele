import Common;
import Scene;
import VertexData;
import MaterialParameter;

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
	out vertices FragmentParameter vertices[MAX_VERTICES], 
	out indices uint3 indices[MAX_PRIMITIVES]
){
	InstanceData inst = pScene.instanceData[meshPayload.instanceId];
	MeshData data = pScene.meshData[meshPayload.instanceId];
	MeshletDescription m = pScene.meshletInfos[data.meshletOffset + groupID];
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
	for(uint i = threadID; i < MAX_VERTICES; i += MESH_GROUP_SIZE)
	{
		uint v = min(i, m.vertexCount - 1);
		{
			uint vertexIndex = pScene.vertexIndices[m.vertexOffset + v];
#ifdef POS_ONLY
            VertexAttributes attr = pVertexData.getPosition(m.indicesOffset + vertexIndex);
#else
			VertexAttributes attr = pVertexData.getAttributes(m.indicesOffset + vertexIndex);
#endif

			attr.meshletId = -1;
			vertices[v] = attr.getParameter(inst.transformMatrix);
		}
	}
}