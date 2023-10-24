import Common;
import BRDF;

interface IMaterial
{
	associatedtype BRDF : IBRDF;
	BRDF prepare(MaterialFragmentParameter input);

};

layout(set = INDEX_MATERIAL)
ParameterBlock<IMaterial> gMaterial;
