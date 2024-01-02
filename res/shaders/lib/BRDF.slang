import Common;

interface IBRDF
{
    float3 evaluate(float3x3 tbn, float3 viewDir_WS, float3 lightDir_WS, float3 lightColor);
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

    float3 evaluate(float3x3 tbn, float3 viewDir_TS, float3 lightDir_TS, float3 lightColor)
    {
        float3 normal_TS = normalize(normal);
        float diffuse = max(dot(normal_TS, lightDir_TS), 0);
        float3 h = normalize(lightDir_TS + viewDir_TS);
        float specular = saturate(dot(normal_TS, h));
		
        return baseColor * (diffuse + specular) * lightColor;
    }
};

struct CelShading : IBRDF
{
    float3 baseColor;
    float3 normal;

    __init()
    {
        normal = float3(0, 0, 1);
    }

    float3 evaluate(float3x3 tbn, float3 viewDir_TS, float3 lightDir_TS, float3 lightColor)
    {
        float3 normal_TS = normalize(normal);
        float nDotL = dot(normal_TS, lightDir_TS);
        float diffuse = max(nDotL, 0);

        float3 darkenedBase = baseColor * 0.8;
        if(diffuse > 0.5)
        {
            return baseColor * lightColor;
        }
        else
        {
            return darkenedBase * lightColor;
        }
    }
};
