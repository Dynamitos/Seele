import LightEnv;
import Material;
import BRDF;
import MaterialParameter;

struct TexturedMaterial : IMaterial
{
	Texture2D diffuseTexture;
	Texture2D specularTexture;
	Texture2D normalTexture;
    float uvScale;
    float metallic = 0;
    float subsurface = 0;
    float roughness = 0.5;
    float specularTint = 0;
    float anisotropic = 0;
    float sheen = 0;
    float sheenTint = 0.5f;
    float clearCoat = 0;
    float clearCoatGloss = 1;
	SamplerState textureSampler;
	
	typedef BlinnPhong BRDF;
    BlinnPhong prepare(MaterialFragmentParameter geometry)
	{
        BlinnPhong result;
		result.baseColor = diffuseTexture.Sample(textureSampler, geometry.texCoords[0] * uvScale).xyz;
		result.metallic = 0;
		float3 bumpMapNormal = normalTexture.Sample(textureSampler, geometry.texCoords[0] * uvScale).xyz;
        bumpMapNormal = 2.0 * bumpMapNormal - float3(1.0, 1.0, 1.0);
        result.normal = geometry.transformNormalToWorld(bumpMapNormal);
        result.specular = specularTexture.Sample(textureSampler, geometry.texCoords[0] * uvScale).x;
		result.roughness = roughness;
		result.specularTint = specularTint;
		result.anisotropic = anisotropic;
		result.sheen = sheen;
		result.sheenTint = sheenTint;
		result.clearCoat = clearCoat;
		result.clearCoatGloss = clearCoatGloss;
		return result;
	}
};