import Common;
import BRDF;

interface IMaterial
{
	associatedtype BRDF : IBRDF;
	BRDF prepare(MaterialFragmentParameter input);

};

ParameterBlock<IMaterial> gMaterial;
