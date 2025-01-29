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
const static float A = 0.0;

struct SampleParams
{
    uint pass;
    uint samplesPerPixel;
};
layout(push_constant)
ConstantBuffer<SampleParams> pSamps;


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
    if(pSamps.pass == pSamps.samplesPerPixel) return;
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
    RayPayload payload;
    // initialize accumulated radiance and bxdf
    payload.light=float3(0);
    payload.emissive = 1;
    payload.depth = 1;
    payload.rndSeed = rndSeed + 1;
    payload.anyHit = false;
    TraceRay(pRayTracingParams.scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xff, 0, 0, 0, rayDesc, payload);

    if(pSamps.pass == 0) pRayTracingParams.radianceAccumulator[pix] = float4(0);
    float3 accumulatedRadiance = payload.light / pSamps.samplesPerPixel;
    pRayTracingParams.radianceAccumulator[pix] += float4(accumulatedRadiance, 0);
    float3 compensatedRadiance = pRayTracingParams.radianceAccumulator[pix].xyz * pSamps.samplesPerPixel / (pSamps.pass + 1);
    pRayTracingParams.image[pix] = float4(clamp(compensatedRadiance, 0, 1), 1);
}
