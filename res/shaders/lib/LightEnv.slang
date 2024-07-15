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
        float3 dir_TS = mul(params.tbn, -normalize(direction.xyz));
        return brdf.evaluate(params.viewDir_TS, dir_TS, color.xyz);
    }
};

struct PointLight : ILightEnv
{
    float4 position_WS;
    float4 colorRange;
    
    float3 illuminate<B:IBRDF>(LightingParameter params, B brdf)
    {
        float3 pos_TS = mul(params.tbn, position_WS.xyz);
        float3 lightDir_TS = pos_TS.xyz - params.position_TS;
        float d = length(lightDir_TS);
        float illuminance = max(1 - d / colorRange.w, 0);
        return illuminance * brdf.evaluate(params.viewDir_TS, normalize(lightDir_TS), colorRange.xyz);
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
    float3 getPosition()
    {
        return position_WS.xyz;
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
