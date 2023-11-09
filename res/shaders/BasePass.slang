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
ParameterBlock<LightCullingData> pLightCullingData;

[shader("pixel")]
void pixelMain(in MaterialParameter params : PARAMETER, out float4 baseColor)
{
	BRDF brdf = pMaterial.prepare(params);
    float3 result = float3(0, 0, 0);
    for(int i = 0; i < pLightEnv.numDirectionalLights; ++i)
    {
        result += pLightEnv.directionalLights[i].illuminate(params, brdf);
    }
    for(int i = 0; i < pLightEnv.numPointLights; ++i)
    {
        result += pLightEnv.pointLights[i].illuminate(params, brdf);
    }
    return float4(1, 1, 0, 1.0f);
}
