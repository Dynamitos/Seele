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

type_param TMaterial : IMaterial;
layout(set = 2, binding = 0, std430)
ParameterBlock<TMaterial> gMaterial;
