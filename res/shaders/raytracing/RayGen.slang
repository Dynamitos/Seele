import Common;
import LightEnv;
import RayTracingData;

struct Ray
{
    float3 o;
    float3 d;
}

const static float S_O = 6.9;
const static float f = 0.035;
const static float A = 0.04;
const static float ka = 0;
const static float ks = 0;
const static float3 fogEmm = float3(0, 0.01, 0.01);

struct SampleParams
{
    uint pass;
    uint samplesPerPixel;
};
layout(push_constant)
ConstantBuffer<SampleParams> pSamps;

float3 rand01(uint3 x){                   // pseudo-random number generator
    for (int i=3; i-->0;) x = ((x>>8U)^x.yzx)*1103515245U;
    return float3(x)*(1.0/float(0xffffffffU));
}

float3 nextEventEstimation(float3 accmat, float3 w, float3 x, float3 nl, float kt, bool useAtt, float3 rnd) {
    float3 result = float3(0);
    // Direct Illumination: Next Event Estimation over any present lights
    /*for(int i = meshLights.length(); i-->0;) {
        MeshDescriptor mesh = meshes[meshLights[i]];
        for(int j = 0; j < mesh.numIndices; j+=3) {
            float3 A = float3(vertices[mesh.vertexOffset + indices[mesh.indexOffset + j + 0]]);
            float3 B = float3(vertices[mesh.vertexOffset + indices[mesh.indexOffset + j + 1]]);
            float3 C = float3(vertices[mesh.vertexOffset + indices[mesh.indexOffset + j + 2]]);
            float b1 = 1 - sqrt(rnd.x);
            float b2 = (1 - rnd.y) * sqrt(rnd.x);
            float b3 = rnd.y * sqrt(rnd.x);
            float3 P = A * b1 + B * b2 + C * b3;
            float3 omega = P - x;
            float3 l = normalize(omega);
            float v = 0.f;
            if(intersect(Ray(x,l), matls, paramsls, sphereId, triId) && triId == j) {
                v = 1.0f;
            }
            float3 e1 = C - A;
            float3 e2 = B - A;
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
    }*/
    return result;
}
[shader("raygeneration")]
void raygen()
{
    uint2 pix = DispatchRaysIndex().xy;
    uint2 imgdim = DispatchRaysDimensions().xy;

    //-- define cam
    Ray cam = Ray(pViewParams.cameraPosition_WS.xyz, pViewParams.cameraForward_WS.xyz);
    float3 cx = -normalize(cross(cam.d, abs(cam.d.y) < 0.9 ? float3(0, 1, 0) : float3(0, 0, 1))), cy = cross(cam.d, cx);
    const float2 sdim = float2(0.036, 0.024);

    float S_I = (S_O * f) / (S_O - f);

    //-- sample sensor
    float2 rnd2 = 2*rand01(uint3(pix, pSamps.pass)).xy;   // vvv tent filter sample  
    float2 tent = float2(rnd2.x<1 ? sqrt(rnd2.x)-1 : 1-sqrt(2-rnd2.x), rnd2.y<1 ? sqrt(rnd2.y)-1 : 1-sqrt(2-rnd2.y));
    float2 s = ((pix + 0.5 * (0.5 + float2((pSamps.pass/2)%2, pSamps.pass%2) + tent)) / float2(imgdim) - 0.5) * sdim;
    float3 spos = cam.o + cx*s.x + cy*s.y, lc = cam.o + cam.d * 0.035;           // sample on 3d sensor plane
    float3 accrad=float3(0), accmat=float3(1);        // initialize accumulated radiance and bxdf
    Ray r = Ray(lc, normalize(lc - spos));      // construct ray


    //-- setup lens
    float3 lensP = lc;
    float3 lensN = -cam.d;
    float3 lensX = cross(lensN, float3(0, 1, 0)); // the exact vector doesnt matter
    float3 lensY = cross(lensN, lensX);
    uint3 rndSeed = uint3(pix, pSamps.pass);
    float2 rnd01 = rand01(rndSeed).xy;

    float3 lensSample = lensP + rnd01.x * A * lensX + rnd01.y * A * lensY;

    float3 focalPoint = cam.o + (S_O + S_I) * cam.d;
    float t = dot(focalPoint - r.o, lensN) / dot(r.d, lensN);
    float3 focus = r.o + t * r.d;

    RayDesc rayDesc;
    rayDesc.Origin = lensSample;
    rayDesc.Direction = normalize(focus - lensSample);
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 10000.0;
	
    const uint maxDepth = 12;
    float emissive = 1;
    RayPayload payload;
    for(uint depth = 0; depth < maxDepth; ++depth) {
        TraceRay(pRayTracingParams.scene, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, rayDesc, payload);
        float3 rnd = rand01(uint3(pix, pSamps.pass*maxDepth + depth));
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
        float p = max(max(payload.material.color.x, payload.material.color.y), payload.material.color.z);
        if(depth > 5) {
            if (rnd.z >= p) break;
            else accmat /= p;
        }
        accrad += accmat * payload.material.emissive * emissive;
        accmat *= payload.material.color.xyz;
        if (payload.material.color.w == 0) {
            break;
        }
        //-- Ideal DIFFUSE reflection
        else if (payload.material.color.w == 1) {
            //if(bool(useNEE)) {
            //    accrad += nextEventEstimation(accmat, r.d, params.x, params.nl, kt, false, rnd);
            //}
            for(uint i = 0; i < pLightEnv.numDirectionalLights; ++i) {
                float3 x = payload.shading.position;
                float3 l = -pLightEnv.directionalLights[i].direction.xyz;
                float3 nl = l;
                rayDesc.Origin = x;
                rayDesc.Direction = l;
                TraceRay(pRayTracingParams.scene, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, rayDesc, payload);

                // we have missed all geometry, so directional light is affecting us
                if(payload.material.color.w == 0) {
                    float omega = 2 * PI;
                    accrad += accmat / PI * max(dot(l, nl), 0) * pLightEnv.directionalLights[i].color.xyz * omega;
                }
            }
            //for(uint i = 0; i < pLightEnv.numPointLights; ++i) {
            //    float3 x = payload.shading.position;
            //    float3 l = pLightEnv.pointLights[i].position_WS.xyz - payload.shading.position;
            //    float3 nl = -payload.shading.normal;
            //    rayDesc.Origin = x;
            //    rayDesc.Direction = l;
            //    TraceRay(pRayTracingParams.scene, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, rayDesc, payload);
//
            //    // hitting only after the light
            //    if(length(payload.shading.position - x) > length(pLightEnv.pointLights[i].position_WS.xyz - x))
            //    {
            //        float omega = 2 * PI;
            //        accrad += accmat / PI * max(dot(l,nl),0) * pLightEnv.pointLights[i].colorRange.xyz * omega; 
            //    }
            //}
            // Indirect Illumination: cosine-weighted importance sampling
            float r1 = 2 * PI * rnd.x, r2 = rnd.y, r2s = sqrt(r2);
            float3 w = payload.shading.normalLight, u = normalize((cross(abs(w.x)>0.1 ? float3(0,1,0) : float3(1,0,0), w))), v = cross(w,u);
            rayDesc.Origin = payload.shading.position;
            rayDesc.Direction = normalize(u*cos(r1)*r2s + v * sin(r1)*r2s + w * sqrt(1 - r2));
            emissive = 0;   // in the next bounce, consider reflective part only!
        } 
        //-- Ideal SPECULAR reflection
        else if (payload.material.color.w == 2) {   
            rayDesc.Origin = payload.shading.position;
            rayDesc.Direction = reflect(r.d,payload.shading.normal);
            emissive = 1; 
        } 
        //-- Ideal dielectric REFRACTION
        else if (payload.material.color.w == 3) {  
            bool into = all(payload.shading.normal==payload.shading.normalLight);
            float cos2t, nc=1, nt=1.5, nnt = into ? nc/nt : nt/nc, ddn = dot(r.d,payload.shading.normalLight);
            if ((cos2t = 1 - nnt * nnt*(1 - ddn * ddn)) >= 0) {     // Fresnel reflection/refraction
                float3 tdir = normalize(r.d*nnt - payload.shading.normal * ((into ? 1 : -1)*(ddn*nnt + sqrt(cos2t))));
                float a = nt - nc, b = nt + nc, R0 = a*a/(b*b), c = 1 - (into ? -ddn : dot(tdir,payload.shading.normal));
                float Re = R0 + (1 - R0)*c*c*c*c*c, Tr = 1 - Re, P = 0.25 + 0.5*Re, RP = Re/P, TP = Tr/(1-P);
                rayDesc.Origin = payload.shading.position;
                rayDesc.Direction = rnd.x < P ? reflect(r.d,payload.shading.normal) : tdir;      // pick reflection with probability P
                accmat *=  rnd.x < P ? RP : TP;                     // energy compensation
            } else {
                rayDesc.Origin = payload.shading.position;
                rayDesc.Direction = reflect(r.d,payload.shading.normal);                      // Total internal reflection
            }
            emissive = 1; 
        }
    }
    if(pSamps.pass == 0) pRayTracingParams.radianceAccumulator[pix] = float4(0);
    pRayTracingParams.radianceAccumulator[pix] += float4(accrad / pSamps.samplesPerPixel, 0);
    pRayTracingParams.image[pix] = float4(clamp(pRayTracingParams.radianceAccumulator[pix].xyz, 0, 1), 1);
}
