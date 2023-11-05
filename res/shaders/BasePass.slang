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
ParameterBuffer<LightCullingData> gLightCullingData;

[shader("pixel")]
void pixelMain(in VertexAttributes attribs, out float4 baseColor)
{
    MaterialParameter params = MaterialParameter.create(attribs);
	BRDF brdf = gMaterial.prepare(params);
    float3 result = float3(0, 0, 0);
    for(int i = 0; i < gLightEnv.numDirectionalLights; ++i)
    {
        result += gLightEnv.directionalLights[i].illuminate(params, brdf);
    }
    for(int i = 0; i < gLightEnv.numPointLights; ++i)
    {
        result += gLightEnv.pointLights[i].illuminate(params, brdf);
    }
    return float4(result, 1.0f);
}
