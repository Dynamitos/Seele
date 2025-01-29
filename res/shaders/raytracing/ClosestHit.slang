import Common;
import MaterialParameter;
import LightEnv;
import Scene;
import RayTracingData;
import StaticMeshVertexData;
import Material;
import MATERIAL_FILE_NAME;

// simplification: all BLAS only have 1 geometry

/*vec3 nextEventEstimation(vec3 accmat, vec3 w, vec3 x, vec3 nl, float kt, bool useAtt, vec3 rnd) {
            uint triId = 0;
            uint sphereId = 0;
            ShadingParams paramsls;
            Material matls;
            vec3 result = vec3(0);
            // Direct Illumination: Next Event Estimation over any present lights
            for (int i = spheres.length(); i-->0;) {
                Sphere ls = spheres[i];
                if (all(equal(ls.e, vec3(0)))) continue; // skip non-emissive spheres 
                vec3 xc = ls.geo.xyz - x;
                vec3 sw = normalize(xc), su = normalize(cross((abs(sw.x)>.1 ? vec3(0,1,0) : vec3(1,0,0)), sw)), sv = cross(sw,su);
                float cos_a_max = sqrt(float(1 - ls.geo.w*ls.geo.w / dot(xc,xc)));
                float cos_a = 1 - rnd.x + rnd.x*cos_a_max, sin_a = sqrt(1 - cos_a*cos_a);
                float phi = 2 * pi * rnd.y;
                vec3 l = normalize(su*cos(phi)*sin_a + sv*sin(phi)*sin_a + sw*cos_a);   // sampled direction towards light
                if (intersect(Ray(x,l), matls, paramsls, sphereId, triId) && sphereId == i) {      // test if shadow ray hits this light source
                    float omega = 2 * pi * (1-cos_a_max);
                    if(useAtt) {
                        float tau = exp(-kt * length(x - paramsls.x));
                        result += max(dot(l,nl),0) * ls.e * omega * tau;
                    } else {
                        result += accmat / pi * max(dot(l,nl),0) * ls.e * omega;   // brdf term obj.c.xyz already in accmat, 1/pi for brdf
                    }
                }
            }
            for(int i = meshLights.length(); i-->0;) {
                MeshDescriptor mesh = meshes[meshLights[i]];
                for(int j = 0; j < mesh.numIndices; j+=3) {
                    vec3 A = vec3(vertices[mesh.vertexOffset + indices[mesh.indexOffset + j + 0]]);
                    vec3 B = vec3(vertices[mesh.vertexOffset + indices[mesh.indexOffset + j + 1]]);
                    vec3 C = vec3(vertices[mesh.vertexOffset + indices[mesh.indexOffset + j + 2]]);
                    float b1 = 1 - sqrt(rnd.x);
                    float b2 = (1 - rnd.y) * sqrt(rnd.x);
                    float b3 = rnd.y * sqrt(rnd.x);
                    vec3 P = A * b1 + B * b2 + C * b3;
                    vec3 omega = P - x;
                    vec3 l = normalize(omega);
                    float v = 0.f;
                    if(intersect(Ray(x,l), matls, paramsls, sphereId, triId) && triId == j) {
                        v = 1.0f;
                    }
                    vec3 e1 = C - A;
                    vec3 e2 = B - A;
                    float area = length(cross(e1, e2)) / 2;
                    float rayLen = length(omega);
                    float cosTheta = dot(nl, l);
                    float cosThetaDash = dot(paramsls.n, -l);
                    float factor = area * (cosThetaDash / (rayLen * rayLen));
                    if(useAtt) {
                        float tau = phase(w, l) * exp(-kt * length(x - paramsls.x));
                        result += tau * matls.e * max(cosTheta, 0) * factor;
                    } else {
                        result += accmat * (matls.e * max(cosTheta, 0) * factor) / pi;
                    }
                }
            }
            return result;
        }

*/

const float eps = 1e-5;
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
    float3 normalLight_WS = dot(normal_WS,WorldRayDirection())<0 ? normal_WS : -normal_WS;
    float3 intersection_WS = params.position_WS;
    
    hitValue.depth++;
    float3 rnd = rand01(hitValue.rndSeed);
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

    //float p = max(max(brdf.baseColor.x, brdf.baseColor.y), brdf.baseColor.z);
    //if(hitValue.depth > 5) {
    //    if (rnd.z >= p) return;
    //    else hitValue.accmat /= p;
    //}

    hitValue.light += brdf.emissive * hitValue.emissive + brdf.evaluateAmbient();
    //-- Ideal DIFFUSE reflection
    //if(bool(useNEE)) {
    //    accrad += nextEventEstimation(accmat, r.d, params.x, params.nl, kt, false, rnd);
    //}
    for(uint i = 0; i < pLightEnv.numDirectionalLights; ++i) {
        float3 x = intersection_WS;
        float3 l = -pLightEnv.directionalLights[i].direction.xyz;
        RayDesc rayDesc;
        rayDesc.TMax = 10000.0f;
        rayDesc.TMin = eps;
        rayDesc.Origin = x;
        rayDesc.Direction = l;
        RayPayload payload;
        payload.depth = hitValue.depth;
        payload.emissive = 1;
        payload.anyHit = true;
        payload.hit = false;
        payload.rndSeed = hitValue.rndSeed;
        TraceRay(pRayTracingParams.scene, 0, 0xff, 0, 0, 0, rayDesc, payload);

        // we have missed all geometry, so directional light is affecting us
        //if(!payload.hit) {
            //hitValue.light += pLightEnv.directionalLights[i].illuminate(lightingParams, brdf);
        //}
    }
    for(uint i = 0; i < pLightEnv.numPointLights; ++i) {
        RayPayload payload;
        payload.rndSeed = hitValue.rndSeed;
        float3 x = intersection_WS;
        float3 l = pLightEnv.pointLights[i].position_WS.xyz - intersection_WS;
        if(length(l) > pLightEnv.pointLights[i].colorRange.w) {
            continue;
        }
        RayDesc rayDesc;
        rayDesc.TMax = 1.0f;
        rayDesc.TMin = eps;
        rayDesc.Origin = x;
        rayDesc.Direction = l;
        TraceRay(pRayTracingParams.scene, 0, 0xff, 0, 0, 0, rayDesc, payload);
    
        // hitting only after the light
        if(!payload.hit) {
            hitValue.light += pLightEnv.pointLights[i].illuminate(lightingParams, brdf);
        }
    }
    // Indirect Illumination: cosine-weighted importance sampling
    if(hitValue.depth < 12) {
    	float r1 = 2 * PI * rnd.x, r2 = rnd.y, r2s = sqrt(r2);
        float3 w = normalLight_WS;
        float3 u = normalize(cross(abs(w.x)>0.1 ? float3(0,1,0) : float3(1,0,0), w));
        float3 v = cross(w,u);
        RayDesc rayDesc;
        rayDesc.TMax = 10000.0f;
        rayDesc.TMin = eps;
        rayDesc.Origin = intersection_WS;
        rayDesc.Direction = normalize(u*cos(r1)*r2s + v * sin(r1)*r2s + w * sqrt(1 - r2));
        RayPayload payload;
        payload.light = float3(0);
        payload.emissive = 1;
        payload.hit = false;
        //payload.emissive = 0;   // in the next bounce, consider reflective part only!
        payload.depth = hitValue.depth;
        payload.anyHit = false;
        payload.rndSeed = hitValue.rndSeed + 1;
        TraceRay(pRayTracingParams.scene, 0, 0xff, 0, 0, 0, rayDesc, payload);
        float bias = dot(normalLight_WS, rayDesc.Direction);
        hitValue.light += brdf.evaluate(params.getTangentToWorld(), -WorldRayDirection(), rayDesc.Direction, payload.light / bias);
    }
}