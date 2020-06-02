import Common;
import BRDF;
import InputGeometry;

interface IMaterial
{
	associatedtype BRDF : IBRDF;
	BRDF prepare(MaterialPixelParameter geometry);
};
