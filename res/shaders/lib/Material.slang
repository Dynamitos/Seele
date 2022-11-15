import Common;
import BRDF;
import MaterialParameter;

interface IMaterial
{
	associatedtype BRDF : IBRDF;
	BRDF prepare(MaterialFragmentParameter input);

	float3 getWorldOffset();

	float3 getBaseColor(MaterialFragmentParameter input);
	float getMetallic(MaterialFragmentParameter input);
	float3 getNormal(MaterialFragmentParameter input);
	float getSpecular(MaterialFragmentParameter input);
	float getRoughness(MaterialFragmentParameter input);
	float getSheen(MaterialFragmentParameter input);
};

layout(set = INDEX_MATERIAL, binding = 0, std430)
ParameterBlock<IMaterial> gMaterial;
