import Common;
import RayTracingData;
import MaterialParameter;
import LightEnv;
import MATERIAL_FILE_NAME;

[shader("callable")]
void callable(inout CallablePayload payload)
{
    LightingParameter lightingParams = payload.params.getLightingParameter();
    MaterialParameter materialParams = payload.params.getMaterialParameter();
	let brdf = Material.prepare(materialParams);
    
    float3 result = float3(0, 0, 0);
    for(int i = 0; i < pLightEnv.numDirectionalLights; ++i)
    {
        result += pLightEnv.directionalLights[i].illuminate(lightingParams, brdf);
    }
    for(uint i = 0; i < pLightEnv.numPointLights; ++i)
    {
        result += pLightEnv.pointLights[i].illuminate(lightingParams, brdf);
    }
    result += brdf.evaluateAmbient();
    // gamma correction
    result = result / (result + float3(1.0));
    result = pow(result, float3(1.0/2.2));
    payload.color = result;
}