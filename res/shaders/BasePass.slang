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
        int2 texDim;
        pLightEnv.shadowMap.GetDimensions(texDim.x, texDim.y);
        float scale = 1.5f;
        float dx = scale * 1.0 / float(texDim.x);
        float dy = scale * 1.0 / float(texDim.y);

        float shadowFactor = 0.0f;
        int count = 0;
        int range = 1;
        for (int x = -range; x <= range; ++x) {
            for (int y = -range; y <= range; ++y) {
                float shadow = 1.0f;
                if (shadowCoord.z > 0.0 && shadowCoord.z < 1.0)
                {
                    float dist = pLightEnv.shadowMap.Sample(pLightEnv.shadowSampler, shadowCoord.xy + float2(dx * x, dy * y)).r;
                    if (shadowCoord.w > 0 && dist > shadowCoord.z)
                    {
                        shadow = 0;
                    }
                }
                shadowFactor += shadow;
                count++;
            }
        }

        result += (shadowFactor / count) * pLightEnv.directionalLights[i].illuminate(lightingParams, brdf);
    }
    for (uint i = 0; i < pLightEnv.numPointLights; ++i)
    {
        //uint lightIndex = pLightCullingData.lightIndexList[startOffset + i];
        result += pLightEnv.pointLights[i].illuminate(lightingParams, brdf);
    }
    result += brdf.evaluateAmbient(lightingParams.viewDir_WS);
    return float4(result, brdf.getAlpha());
}