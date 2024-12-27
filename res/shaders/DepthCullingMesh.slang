import Common;
import Scene;
import VertexData;
import StaticMeshVertexData;
import MaterialParameter;

struct PrimitiveAttributes
{
#ifdef VISIBILITY
    uint32_t prim : SV_PrimitiveID;
#endif
};

[numthreads(MESH_GROUP_SIZE, 1, 1)]
[outputtopology("triangle")]
[shader("mesh")]
void meshMain(
    in uint threadID: SV_GroupThreadID,
    in uint groupID: SV_GroupID,
    in payload MeshPayload meshPayload,
    out vertices FragmentParameter vertices[MAX_VERTICES],
    out indices uint3 indices[MAX_PRIMITIVES],
    out primitives PrimitiveAttributes prim[MAX_PRIMITIVES]
) {
    // meshlet number relative to start for this instance
    uint meshletNumber = meshPayload.culledMeshlets[groupID];
    uint meshletId = meshPayload.cullingOffset + meshletNumber;
    InstanceData inst = pScene.instances[meshPayload.instanceId];
    MeshletDescription m = pScene.meshletInfos[meshPayload.meshletOffset + meshletNumber];
    MeshletCullingInfo cull = pScene.cullingInfos[meshletId];
	SetMeshOutputCounts(m.vertexCount, m.primitiveCount);
	
	for(uint i = threadID; i < MAX_PRIMITIVES; i += MESH_GROUP_SIZE)
	{
		uint p = min(i, m.primitiveCount - 1);
		{
            uint baseIndex = m.primitiveOffset + (p * 3);
			uint local_idx0 = pScene.primitiveIndices[baseIndex + 0];
			uint local_idx1 = pScene.primitiveIndices[baseIndex + 1];
			uint local_idx2 = pScene.primitiveIndices[baseIndex + 2];
            indices[p] = uint3(local_idx0, local_idx1, local_idx2);
            PrimitiveAttributes primAttr;
#ifdef VISIBILITY
            primAttr.prim = encodePrimitive(meshletId);
#endif
            prim[p] = primAttr;
		}
	}
	for(uint i = threadID; i < MAX_VERTICES; i += MESH_GROUP_SIZE)
	{
		uint v = min(i, m.vertexCount - 1);
		{
			uint vertexIndex = pScene.vertexIndices[m.vertexOffset + v];
			VertexAttributes attr = pVertexData.getAttributes(m.indicesOffset + vertexIndex);
			vertices[v] = attr.getParameter(inst.transformMatrix, inst.inverseTransformMatrix);
		}
	}
}