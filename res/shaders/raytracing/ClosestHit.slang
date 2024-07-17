import Common;
import MaterialParameter;
import LightEnv;
import Scene;
import RayTracingData;
import VertexData;
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

    LightingParameter lightingParams = params.getLightingParameter();
    MaterialParameter materialParams = params.getMaterialParameter();
	let brdf = Material.prepare(materialParams);

    float3 result = float3(0, 0, 0);
    for(int i = 0; i < pLightEnv.numDirectionalLights; ++i)
    {
        result += pLightEnv.directionalLights[i].illuminate(lightingParams, brdf);
    }
    for(uint i = 0; i < pLightEnv.numPointLights; ++i)
    {
        result += pLightEnv.pointLights[i].illuminate(lightingParams, brdf);
    }
    result += brdf.evaluateAmbient();
    // gamma correction
    result = result / (result + float3(1.0));
    result = pow(result, float3(1.0/2.2));
    
    hitValue.color = result;
}