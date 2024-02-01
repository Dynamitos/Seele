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

    bool insidePlane(Plane plane, float3 position_CS)
    {
        return plane.pointInside(position_CS);
    }

    bool insideFrustum(Frustum frustum)
    {
        bool result = true;
        float4 clipPos = mul(pViewParams.projectionMatrix, mul(pViewParams.viewMatrix, position_WS));
        float3 position_CS = clipPos.xyz / clipPos.w;
        for(int i = 0; i < 4; ++i)
        {
            
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
