import Common;
import LightEnv;
import Material;
import MaterialParameter;

struct LightCullingData
{
    RWStructuredBuffer<uint> lightIndexList;

    RWTexture2D<uint2> lightGrid;
};
layout(set=5)
ParameterBlock<LightCullingData> pLightCullingData;

[shader("pixel")]
float4 fragmentMain(in FragmentParameter params) : SV_Target
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
        //uint lightIndex = pLightCullingData.lightIndexList[startOffset + i];
        result += pLightEnv.pointLights[i].illuminate(lightingParams, brdf);
    }
    return float4(1.0f, 0.1f, 0.5f, 1.0f);
}
