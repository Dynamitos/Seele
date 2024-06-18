import Common;
import BRDF;
import MaterialParameter;
import Scene;

//interface IMaterial
//{
//	associatedtype BRDF : IBRDF;
//	BRDF prepare(MaterialParameter input);
//};

layout(set=4, binding=0)
Texture2D textureArray[];
layout(set=4, binding=1)
SamplerState samplerArray[];
layout(set=4, binding=2)
StructuredBuffer<float> floatArray;

Texture2D getMaterialTextureParameter(uint index)
{
	return textureArray[pOffsets.textureOffset + index];
}

SamplerState getMaterialSamplerParameter(uint index)
{
	return samplerArray[pOffsets.samplerOffset + index];
}

float getMaterialFloatParameter(uint index)
{
	return floatArray[pOffsets.floatOffset + index];
}

float3 getMaterialVectorParameter(uint index)
{
	return float3(
		floatArray[pOffsets.floatOffset + index],
		floatArray[pOffsets.floatOffset + index + 1],
		floatArray[pOffsets.floatOffset + index + 2]
	);
}
