import Common;
import BRDF;
import MaterialParameter;

interface IMaterial
{
	associatedtype BRDF : IBRDF;
	BRDF prepare(MaterialFragmentParameter input);
};
