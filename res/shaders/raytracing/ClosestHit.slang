import Common;
import MaterialParameter;
import LightEnv;
import Scene;
import RayTracingData;
import StaticMeshVertexData;
import Material;
import MATERIAL_FILE_NAME;

// simplification: all BLAS only have 1 geometry

[shader("closesthit")]
void closestHit(inout RayPayload hitValue, in BuiltInTriangleIntersectionAttributes attr)
{
    const float3 barycentricCoords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);

    InstanceData inst = pScene.instances[InstanceID()];
    MeshData m = pScene.meshData[InstanceID()];

    // offset into the index buffer
    uint indexOffset = m.firstIndex;
    // added to indices to reference correct part of global mesh pool
    uint vertexOffset = pScene.meshletInfos[m.meshletOffset].indicesOffset;

    uint vertexIndex0 = vertexOffset + pRayTracingParams.indexBuffer[indexOffset + 3 * PrimitiveIndex() + 0];
    uint vertexIndex1 = vertexOffset + pRayTracingParams.indexBuffer[indexOffset + 3 * PrimitiveIndex() + 1];
    uint vertexIndex2 = vertexOffset + pRayTracingParams.indexBuffer[indexOffset + 3 * PrimitiveIndex() + 2];

    VertexAttributes attr0 = pVertexData.getAttributes(vertexIndex0);
    VertexAttributes attr1 = pVertexData.getAttributes(vertexIndex1);
    VertexAttributes attr2 = pVertexData.getAttributes(vertexIndex2);

    FragmentParameter f0 = attr0.getParameter(inst.transformMatrix);
    FragmentParameter f1 = attr1.getParameter(inst.transformMatrix);
    FragmentParameter f2 = attr2.getParameter(inst.transformMatrix);

    FragmentParameter params = FragmentParameter.interpolate(f0, f1, f2, barycentricCoords);

    MaterialParameter materialParams = params.getMaterialParameter();
    let brdf = Material.prepare(materialParams);
    

    hitValue.material.color = float4(brdf.baseColor, 1);
    hitValue.material.emissive = float3(0, 0, 0);
    hitValue.shading.position = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    hitValue.shading.normal = cross(f2.position_WS - f0.position_WS, f1.position_WS - f0.position_WS);
    hitValue.shading.normalLight = dot(hitValue.shading.normal, WorldRayDirection()) < 0 ? hitValue.shading.normal : -hitValue.shading.normal;
}