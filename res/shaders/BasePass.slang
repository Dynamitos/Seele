import Common;
import LightEnv;
import MaterialParameter;
import MATERIAL_FILE_NAME;

struct LightCullingData
{
    RWStructuredBuffer<uint> lightIndexList;

    RWTexture2D<uint2> lightGrid;
};
layout(set=5)
ParameterBlock<LightCullingData> pLightCullingData;

struct FragmentOutput
{
    float4 color : SV_Target0;
    uint meshletId : SV_Target1;
};

[shader("pixel")]
FragmentOutput fragmentMain(in FragmentParameter params)
{
    LightingParameter lightingParams = params.getLightingParameter();
    MaterialParameter materialParams = params.getMaterialParameter();
	let brdf = pMaterial.prepare(materialParams);
    uint2 tileIndex = uint2(floor(params.position_CS.xy / BLOCK_SIZE));
    uint startOffset = pLightCullingData.lightGrid[tileIndex].x;
    uint lightCount = pLightCullingData.lightGrid[tileIndex].y;
    float3 result = float3(0, 0, 0);
    for(int i = 0; i < pLightEnv.numDirectionalLights; ++i)
    {
        result += pLightEnv.directionalLights[i].illuminate(lightingParams, brdf);
    }
    for(uint i = 0; i < pLightEnv.numPointLights; ++i)
    {
        uint lightIndex = pLightCullingData.lightIndexList[startOffset + i];
        result += pLightEnv.pointLights[lightIndex].illuminate(lightingParams, brdf);
    }
    result += brdf.evaluateAmbient();
    // gamma correction
    result = result / (result + float3(1.0));
    result = pow(result, float3(1.0/2.2));
    FragmentOutput output;
    output.color = float4(result, 1.0f);
    output.meshletId = params.meshletId;
    return output;
}
            