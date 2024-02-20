import Common;
import LightEnv;
import Material;
import MaterialParameter;

struct LightCullingData
{
    RWStructuredBuffer<uint> oLightIndexList;
    RWStructuredBuffer<uint> tLightIndexList;

    RWTexture2D<uint2> oLightGrid;
    RWTexture2D<uint2> tLightGrid;
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
    uint startOffset = pLightCullingData.oLightGrid[tileIndex].x;
    uint lightCount = pLightCullingData.oLightGrid[tileIndex].y;
    float3 result = float3(0, 0, 0);
    for(int i = 0; i < pLightEnv.numDirectionalLights; ++i)
    {
        result += pLightEnv.directionalLights[i].illuminate(lightingParams, brdf);
    }
    for(uint i = 0; i < lightCount; ++i)
    {
        uint lightIndex = pLightCullingData.oLightIndexList[startOffset + lightCount];
        result += pLightEnv.pointLights[lightIndex].illuminate(lightingParams, brdf);
    }
    return float4(result, 1.0f);
}
