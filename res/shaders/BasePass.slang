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
        float4 lightSpacePos = mul(pLightEnv.directionalLights[i].lightSpaceMatrix, float4(params.position_WS, 1));
        float3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
        projCoords.xy = projCoords.xy * 0.5 + 0.5;
        projCoords.z /= 2;
        float closestDepth = pLightEnv.shadowMap.Sample(pLightEnv.shadowSampler, projCoords.xy).r;
        float currentDepth = projCoords.z;
        float bias = max(0.05 * (1.0 - dot(brdf.getNormal(), pLightEnv.directionalLights[i].direction.xyz)), 0.005);
        float shadow = currentDepth + bias < closestDepth ? 1.0 : 0.0;

        result += float3(currentDepth, 0, 0);//shadow * pLightEnv.directionalLights[i].illuminate(lightingParams, brdf);
    }
    for (uint i = 0; i < lightCount; ++i)
    {
        uint lightIndex = pLightCullingData.lightIndexList[startOffset + i];
        result += pLightEnv.pointLights[lightIndex].illuminate(lightingParams, brdf);
    }
    result += brdf.evaluateAmbient(lightingParams.viewDir_WS);
    return float4(result, brdf.getAlpha());
}