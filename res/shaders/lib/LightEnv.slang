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
		return intensity.xyz * brdf.evaluate(wo, direction.xyz, input.worldNormal, input.worldTangent, input.worldBiTangent, color.xyz);	
    }
};

struct PointLight : ILightEnv
{
    float4 positionWS;
    float4 positionVS;
    float3 color;
    float range;

    float3 illuminate<B:IBRDF>(MaterialFragmentParameter input, B brdf, float3 viewDir)
	{
        float3 lightVec = positionWS.xyz - input.worldPosition;
        float d = length(lightVec);
        float3 direction = normalize(lightVec);
        float illuminance = max(1 - d / range, 0);
		return illuminance * brdf.evaluate(viewDir, direction, input.worldNormal, input.worldTangent, input.worldBiTangent, color);
    }

	bool insidePlane(Plane plane)
	{
        return dot(plane.n, positionVS.xyz) - plane.d < -range;
	}

	bool insideFrustum(Frustum frustum, float zNear, float zFar)
	{
        bool result = true;

		//if(positionVS.z - range > zNear || positionVS.z + range < zFar)
		{
		//	result = false;
		}
		for(int i = 0; i < 4 && result; ++i)
		{
			if(insidePlane(frustum.planes[i]))
			{
				result = false;
			}
		}
		return result;
	}
};

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 256
struct Lights
{
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
    PointLight pointLights[MAX_POINT_LIGHTS];
    uint numDirectionalLights;
    uint numPointLights;
};

layout(set = 0, binding = 1)
ConstantBuffer<Lights> gLightEnv;
