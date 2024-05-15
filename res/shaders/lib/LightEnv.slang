import Common;
import BRDF;
import MaterialParameter;

interface ILightEnv
{
    float3 illuminate<B:IBRDF>(LightingParameter input, B brdf);
};

struct DirectionalLight : ILightEnv
{
    float4 color;
    float4 direction;

    float3 illuminate<B:IBRDF>(LightingParameter params, B brdf)
    {
        return brdf.evaluate(params.tbn, params.viewDir_WS, -normalize(direction.xyz), params.normal_WS, color.xyz);
    }
};

struct PointLight : ILightEnv
{
    float4 position_WS;
    float4 colorRange;
    
    float3 illuminate<B:IBRDF>(LightingParameter params, B brdf)
    {
        float3 lightDir_WS = position_WS.xyz - params.position_WS;
        float d = length(lightDir_WS);
        float illuminance = max(1 - d / colorRange.w, 0);
        return illuminance * brdf.evaluate(params.tbn, params.viewDir_WS, normalize(lightDir_WS), normalize(params.normal_WS), colorRange.xyz);
    }

    bool insidePlane(Plane plane, float3 position)
    {
        return dot(plane.n, position) - plane.d < -colorRange.w;
    }

    bool insideFrustum(Frustum frustum, float3 position, float minDepth, float maxDepth)
    {
        bool result = true;
        if(position.z - colorRange.w > minDepth || position.z + colorRange.w < maxDepth)
        {
            result = false;
        }
        for(int i = 0; i < 4 && result; ++i)
        {
            if(insidePlane(frustum.sides[i], position))
            {
                result = false;
            }
        }
        return result;
    }
};

struct LightEnv
{
    StructuredBuffer<DirectionalLight> directionalLights;
    uint numDirectionalLights;
    StructuredBuffer<PointLight> pointLights;
    uint numPointLights;
};
layout(set=3)
ParameterBlock<LightEnv> pLightEnv;
