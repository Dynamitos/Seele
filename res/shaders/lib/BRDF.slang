import Common;

interface IBRDF
{
    float3 evaluate(float3 viewDir_TS, float3 lightDir_TS, float3 lightColor);
};

struct BlinnPhong : IBRDF
{
    float3 baseColor;
    float metallic;
    float3 normal;
    float specular;
    float roughness;
    float sheen;

    __init()
    {
        metallic = 0;
        normal = float3(0, 0, 1);
        specular = 0.5;
        roughness = 0.5;
        sheen = 1;
    }

    float3 evaluate(float3 viewDir_TS, float3 lightDir_TS, float3 lightColor)
    {
        float nDotL = saturate(dot(normal, lightDir_TS));
        float3 h = normalize(lightDir_TS + viewDir_TS);
        float nDotH = saturate(dot(normal, h));
		
        return baseColor;
    }
};

struct DisneyBRDF : IBRDF
{
    float3 baseColor;
    float metallic = 0;
    float3 normal = float3(0, 1, 0);
    float subsurface = 0;
    float specular = 0.5;
    float roughness = 0.5;
    float specularTint = 0;
    float anisotropic = 0;
    float sheen = 0;
    float sheenTint = 0.5f;
    float clearCoat = 0;
    float clearCoatGloss = 1;

    float3 evaluate(float3 viewDir_TS, float3 lightDir_TS, float3 lightColor)
    {
        return baseColor;
    }
};
