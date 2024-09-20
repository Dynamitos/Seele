import BRDF;
import MaterialParameter;
import Material;
struct Material{
	typedef Phong BRDF;
	static Phong prepare(MaterialParameter input) {
		Phong result;
		result.alpha = 1;
		result.ambient = float3(1);
		result.baseColor = float3(0, 1, 0);
		result.shininess = 1;
		result.specular = 1;
		return result;
	}
};
