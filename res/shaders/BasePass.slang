import Common;
import LightEnv;
import MaterialParameter;
import MATERIAL_FILE_NAME;

struct LightCullingData
{
    RWStructuredBuffer<uint> lightIndexList;

    RWTexture2D<uint2> lightGrid;
};
ParameterBlock<LightCullingData> pLightCullingData;

static const float4x4 biasMat = float4x4(
	0.5, 0.0, 0.0, 0.5,
	0.0, 0.5, 0.0, 0.5,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0);

[shader("pixel")]
float4 fragmentMain(in FragmentParameter params) : SV_Target
{
    LightingParameter lightingParams = params.getLightingParameter();
    MaterialParameter materialParams = params.getMaterialParameter();
    var brdf = Material.prepare(materialParams);
    brdf.setNormal(normalize(params.normal_WS));
    uint2 tileIndex = uint2(floor(params.position_CS.xy / BLOCK_SIZE));
    uint startOffset = pLightCullingData.lightGrid[tileIndex].x;
    uint lightCount = pLightCullingData.lightGrid[tileIndex].y;
    float3 result = float3(0, 0, 0);
    for(int i = 0; i < pLightEnv.numDirectionalLights; ++i)
    {
        float4 lightSpacePos = mul(biasMat, mul(pLightEnv.directionalLights[i].lightSpaceMatrix, float4(params.position_WS, 1)));
        float4 shadowCoord = lightSpacePos / lightSpacePos.w;
        float shadow = 1.0f;
        float dist = pLightEnv.shadowMap.Sample(pLightEnv.shadowSampler, shadowCoord.xy).r;
        if (shadowCoord.w > 0 && dist > shadowCoord.z)
        {
            shadow = 0;
        }

        result += shadow * pLightEnv.directionalLights[i].illuminate(lightingParams, brdf);
    }
    for (uint i = 0; i < pLightEnv.numPointLights; ++i)
    {
        //uint lightIndex = pLightCullingData.lightIndexList[startOffset + i];
        result += pLightEnv.pointLights[i].illuminate(lightingParams, brdf);
    }
    //result += brdf.evaluateAmbient(lightingParams.viewDir_WS);
    return float4(result, brdf.getAlpha());
}