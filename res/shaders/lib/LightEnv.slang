import MaterialParameter;
import BRDF;
import Common;

interface ILightEnv
{
    float3 illuminate<B:IBRDF>(MaterialParameter input, B brdf);
};

struct DirectionalLight : ILightEnv
{
    float4 color;
    float4 direction;

    float3 illuminate<B:IBRDF>(MaterialParameter input, B brdf)
    {
        float3 lightDir_TS = normalize(direction.xyz);
        return brdf.evaluate(input.viewDir_TS, -lightDir_TS, color.xyz);	
    }
};

struct PointLight : ILightEnv
{
    float4 position_WS;
    float4 colorRange;
    
    float3 illuminate<B:IBRDF>(MaterialParameter input, B brdf)
    {
        float3 position_TS = position_WS.xyz;
        float3 lightDir_TS = position_TS - input.position_TS;
        float d = length(lightDir_TS);
        float illuminance = max(1 - d / colorRange.w, 0);
        return illuminance * brdf.evaluate(input.viewDir_TS, normalize(lightDir_TS), colorRange.xyz);
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
            if(insidePlane(frustum.sides[i]))
            {
                //result = false;
            }
        }
        return result;
    }
    float3 getViewPos()
    {
        return mul(viewParams.viewMatrix, position_WS).xyz;
    }
};

struct LightEnv
{
    StructuredBuffer<DirectionalLight> directionalLights;
    uint numDirectionalLights;
    StructuredBuffer<PointLight> pointLights;
    uint numPointLights;
};

ParameterBlock<LightEnv> gLightEnv;
