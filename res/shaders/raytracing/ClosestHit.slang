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
    hitValue.hit = true;
    // todo: replace with anyhit shader
    if(hitValue.anyHit)
        return;
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

    FragmentParameter f0 = attr0.getParameter(inst.transformMatrix, inst.inverseTransformMatrix);
    FragmentParameter f1 = attr1.getParameter(inst.transformMatrix, inst.inverseTransformMatrix);
    FragmentParameter f2 = attr2.getParameter(inst.transformMatrix, inst.inverseTransformMatrix);

    FragmentParameter params = FragmentParameter.interpolate(f0, f1, f2, barycentricCoords);

    MaterialParameter materialParams = params.getMaterialParameter();
    LightingParameter lightingParams = params.getLightingParameter();
    lightingParams.viewDir_WS = -WorldRayDirection();
    let brdf = Material.prepare(materialParams);

    float3 normal_WS = normalize(mul(params.getTangentToWorld(), brdf.normal));
    float3 normalLight_WS = dot(normal_WS,WorldRayOrigin())<0 ? normal_WS : -normal_WS;
    float3 intersection_WS = params.position_WS + normal_WS * 0.001f;
    
    hitValue.depth++;
    float3 localAccRad = float3(0);
    float3 rnd = rand01(uint3(vertexIndex0, vertexIndex1, vertexIndex2));
    //float kt = ka + ks;
    //float s = -log(rnd.z) / kt;
    //float3 xs = r.o + s * r.d;
    //if (s < t) {
    //    float p = kt * rnd.z;
    //    if (depth > 5) {
    //        if (rnd.z >= p) break;
    //        else accmat /= p;
    //    }
    //    float3 ldirect = nextEventEstimation(accmat, r.d, xs, -r.d, kt, true, rnd);
    //    accrad += (fogEmm + ks * ldirect) / kt;
    //    accmat *= ks / kt;
    //    rayDesc.Origin = xs;
    //    rayDesc.Direction = float3(
    //        cos(2*PI*rnd.x)*sqrt(1-rnd.y*rnd.y),
    //        sin(2*PI*rnd.x)*sqrt(1-rnd.y*rnd.y),
    //        rnd.y
    //    );
    //    continue;
    //}

    //float p = max(max(brdf.baseColor.x, brdf.baseColor.color.y), brdf.baseColor.color.z);
    //if(hitValue.depth > 5) {
    //    if (rnd.z >= p) return;
    //    else hitValue.accmat /= p;
    //}


    //-- Ideal DIFFUSE reflection
    //if(bool(useNEE)) {
    //    accrad += nextEventEstimation(accmat, r.d, params.x, params.nl, kt, false, rnd);
    //}
    for(uint i = 0; i < pLightEnv.numDirectionalLights; ++i) {
        float3 x = intersection_WS;
        float3 l = -pLightEnv.directionalLights[i].direction.xyz;
        RayDesc rayDesc;
        rayDesc.TMax = 10000.0f;
        rayDesc.TMin = 0.001f;
        rayDesc.Origin = x;
        rayDesc.Direction = l;
        RayPayload payload;
        payload.depth = hitValue.depth;
        payload.emissive = 1;
        payload.anyHit = true;
        TraceRay(pRayTracingParams.scene, 0, 0xff, 0, 0, 0, rayDesc, payload);

        // we have missed all geometry, so directional light is affecting us
        if(!payload.hit) {
            localAccRad += pLightEnv.directionalLights[i].illuminate(lightingParams, brdf);
        }
    }
    for(uint i = 0; i < pLightEnv.numPointLights; ++i) {
        RayPayload payload;
        float3 x = intersection_WS;
        float3 l = pLightEnv.pointLights[i].position_WS.xyz - intersection_WS;
        if(length(l) > pLightEnv.pointLights[i].colorRange.w) {
            continue;
        }
        RayDesc rayDesc;
        rayDesc.TMax = 1.0f;
        rayDesc.TMin = 0.001f;
        rayDesc.Origin = x;
        rayDesc.Direction = l;
        TraceRay(pRayTracingParams.scene, 0, 0xff, 0, 0, 0, rayDesc, payload);
    
        // hitting only after the light
        if(!payload.hit) {
            localAccRad += pLightEnv.pointLights[i].illuminate(lightingParams, brdf);
        }
    }
    // Indirect Illumination: cosine-weighted importance sampling
    if(hitValue.depth < 12) {
    	float r1 = 2 * PI * rnd.x, r2 = rnd.y, r2s = sqrt(r2);
        float3 w = normalLight_WS;
        float3 u = normalize((cross(abs(w.x)>0.1 ? float3(0,1,0) : float3(1,0,0), w)));
        float3 v = cross(w,u);
        RayDesc rayDesc;
        rayDesc.TMax = 10000.0f;
        rayDesc.TMin = 0.001f;
        rayDesc.Origin = intersection_WS;
        rayDesc.Direction = normalize(u*cos(r1)*r2s + v * sin(r1)*r2s + w * sqrt(1 - r2));
        RayPayload payload;
        payload.light = float3(0);
        payload.emissive = 0;   // in the next bounce, consider reflective part only!
        payload.depth = hitValue.depth+1;
        payload.anyHit = false;
        TraceRay(pRayTracingParams.scene, 0, 0xff, 0, 0, 0, rayDesc, payload);
        if(payload.hit) {
            DirectionalLight dir;
            dir.color = float4(payload.light, 0);
            dir.direction = float4(-rayDesc.Direction, 0);
            localAccRad += dir.illuminate(lightingParams, brdf);
        }
    }

    hitValue.light += localAccRad;
}