import Material;
import InputGeometry;

struct ParallaxMaterial : IMaterial
{
    Texture2D<float4> diffuseTexture;
    Texture2D<float4> specularTexture;
    Texture2D<float4> displacementTexture;
    SamplerState textureSampler;
    float specularity;
	
    typedef BlinnPhong BRDF;
    BlinnPhong prepare(InputGeometry geometry)
    {
        BlinnPhong blinn;
        blinn.baseColor = diffuseTexture.Sample(textureSampler, geometry.getTexCoords()).xyz;
        blinn.specularColor = specularTexture.Sample(textureSampler, geometry.getTexCoords()).xyz;
        blinn.specular = specularity;
        return blinn;
    }
};
