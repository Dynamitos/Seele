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
        float3 lightDir_TS = mul(params.tbn, direction.xyz);
        return brdf.evaluate(params.tbn, params.viewDir_TS, -normalize(lightDir_TS), color.xyz);
    }
};

struct PointLight : ILightEnv
{
    float4 position_WS;
    float4 colorRange;
    
    float3 illuminate<B:IBRDF>(LightingParameter params, B brdf)
    {
        float3 lightDir_WS = position_WS.xyz - params.position_WS;
        float3 lightDir_TS = mul(params.tbn, lightDir_WS);
        float d = length(lightDir_TS);
        float illuminance = max(1 - d / colorRange.w, 0);
        return illuminance * brdf.evaluate(params.tbn, params.viewDir_TS, normalize(lightDir_TS), colorRange.xyz);
    }

    bool insidePlane(Plane plane, float3 center_CS)
    {
        float3 position_CS = center_CS + plane.n * colorRange.w;
        return plane.pointInside(position_CS);
    }

    bool insideFrustum(Frustum frustum, float nearClipVS, float maxDepthVS)
    {
        float4 clipPos = mul(pViewParams.projectionMatrix, mul(pViewParams.viewMatrix, position_WS));
        float3 center_CS = clipPos.xyz / clipPos.w;
        if(insidePlane(frustum.basePlane, center_CS))
        {
            return true;
        }
        uint result = 0;
        for(int i = 0; i < 4; ++i)
        {
            if(insidePlane(frustum.sides[i], center_CS))
            {
                result++;
            }
        }
        return result > 0;
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
