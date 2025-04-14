import RayTracingData;
import Scene;
import VertexData;
import MaterialParameter;
import StaticMeshVertexData;

[shader("anyhit")]
void anyhit(inout RayPayload hitValue, in BuiltInTriangleIntersectionAttributes attr)
{
    const float3 barycentricCoords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);

    InstanceData inst = pScene.instances[InstanceID()];
    MeshData m = pScene.meshData[InstanceID()];

    // offset into the index buffer
    uint indexOffset = m.indicesRange.offset;
    // added to indices to reference correct part of global mesh pool
    uint vertexOffset = pScene.meshletInfos[m.meshletRange.offset].indicesOffset;

    uint vertexIndex0 = vertexOffset + pRayTracingParams.indexBuffer[indexOffset + 3 * PrimitiveIndex() + 0];
    uint vertexIndex1 = vertexOffset + pRayTracingParams.indexBuffer[indexOffset + 3 * PrimitiveIndex() + 1];
    uint vertexIndex2 = vertexOffset + pRayTracingParams.indexBuffer[indexOffset + 3 * PrimitiveIndex() + 2];

    VertexAttributes attr0 = pVertexData.getAttributes(vertexIndex0);
    VertexAttributes attr1 = pVertexData.getAttributes(vertexIndex1);
    VertexAttributes attr2 = pVertexData.getAttributes(vertexIndex2);

    FragmentParameter f0 = attr0.getParameter(inst.transformMatrix, inst.inverseTransformMatrix);
    FragmentParameter f1 = attr1.getParameter(inst.transformMatrix, inst.inverseTransformMatrix);
    FragmentParameter f2 = attr2.getParameter(inst.transformMatrix, inst.inverseTransformMatrix);

    FragmentParameter params = FragmentParameter.interpolate(f0, f1, f2, barycentricCoords);

    hitValue.params = params.getLightingParameter();
    hitValue.materialParams = params.getMaterialParameter();
    AcceptHitAndEndSearch();
}