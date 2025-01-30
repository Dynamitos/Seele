import Common;
import BRDF;
import MaterialParameter;
import Scene;

//interface IMaterial
//{
//    associatedtype BRDF: IBRDF;
//	static BRDF prepare(MaterialParameter input);
//};
struct MaterialResources
{
    Texture2D textureArray[512];
    SamplerState samplerArray[512];
    StructuredBuffer<float> floatArray;
};
layout(set=4)
ParameterBlock<MaterialResources> pResources;

Texture2D getMaterialTextureParameter(uint index)
{
	return pResources.textureArray[pOffsets.textureOffset + index];
}

SamplerState getMaterialSamplerParameter(uint index)
{
	return pResources.samplerArray[pOffsets.samplerOffset + index];
}

float getMaterialFloatParameter(uint index)
{
	return pResources.floatArray[pOffsets.floatOffset + index];
}

float3 getMaterialVectorParameter(uint index)
{
	return float3(
		pResources.floatArray[pOffsets.floatOffset + index],
		pResources.floatArray[pOffsets.floatOffset + index + 1],
		pResources.floatArray[pOffsets.floatOffset + index + 2]
	);
}
