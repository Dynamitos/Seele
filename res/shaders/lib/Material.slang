import Common;
import BRDF;
import MaterialParameter;

interface IMaterial
{
	associatedtype BRDF : IBRDF;
	BRDF prepare(MaterialParameter input);
};

layout(set=4)
ParameterBlock<IMaterial> pMaterial;
