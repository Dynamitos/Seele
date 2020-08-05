import LightEnv;
import Material;
import BRDF;
import MaterialParameter;

struct FlatColorMaterial : IMaterial
{
	float3 diffuseColor;
	float specularity;

    typedef BlinnPhong BRDF;
    BlinnPhong prepare(MaterialFragmentParameter input)
	{
        BlinnPhong result;
		result.baseColor = diffuseColor;
		result.specular = specularity;
        result.normal = normalize(input.worldNormal);
        result.roughness = 0.5;
		result.specularTint = 0;
		result.anisotropic = 1;
		result.sheen = 1;
		result.sheenTint = 0.5;
		result.clearCoat = 0;
		result.clearCoatGloss = 0;
		return result;
	}
};
