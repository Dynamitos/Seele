import MaterialParameter;
import BRDF;
import Common;

interface ILightEnv
{
    float3 illuminate<B:IBRDF>(MaterialFragmentParameter input, B brdf, float3 wo);
};

struct DirectionalLight : ILightEnv
{
    float4 color;
    float4 direction;
    float4 intensity;

    float3 illuminate<B:IBRDF>(MaterialFragmentParameter input, B brdf, float3 wo)
    {
        return intensity.xyz * brdf.evaluate(wo, normalize(direction.xyz), input.worldNormal, input.worldTangent, input.worldBiTangent, color.xyz);	
    }
};

struct PointLight : ILightEnv
{
    float4 positionWS;
    float4 colorRange;
    
    float3 illuminate<B:IBRDF>(MaterialFragmentParameter input, B brdf, float3 viewDir)
    {
        float3 lightVec = positionWS.xyz - input.worldPosition;
        float d = length(lightVec);
        float3 direction = normalize(lightVec);
        float illuminance = max(1 - d / colorRange.w, 0);
        return illuminance * brdf.evaluate(viewDir, direction, input.worldNormal, input.worldTangent, input.worldBiTangent, colorRange.xyz);
    }

    bool insidePlane(Plane plane)
    {
        return dot(plane.n, getViewPos().xyz) - plane.d < -colorRange.w;
    }

    bool insideFrustum(Frustum frustum, float zNear, float zFar)
    {
        bool result = true;

        if(getViewPos().z - colorRange.w > zNear || getViewPos().z + colorRange.w < zFar)
        {
            //result = false;
        }
        for(int i = 0; i < 4 && result; ++i)
        {
            if(insidePlane(frustum.planes[i]))
            {
                //result = false;
            }
        }
        return result;
    }
    float3 getViewPos()
    {
        return mul(gViewParams.viewMatrix, positionWS).xyz;
    }
};

layout(set = INDEX_LIGHT_ENV, binding = 0, std430)
StructuredBuffer<DirectionalLight> directionalLights;
layout(set = INDEX_LIGHT_ENV, binding = 1, std430)
ConstantBuffer<uint> numDirectionalLights;
layout(set = INDEX_LIGHT_ENV, binding = 2, std430)
StructuredBuffer<PointLight> pointLights;
layout(set = INDEX_LIGHT_ENV, binding = 3, std430)
ConstantBuffer<uint> numPointLights;
