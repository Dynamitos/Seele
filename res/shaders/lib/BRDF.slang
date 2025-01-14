import Common;

interface IBRDF
{
    float3 evaluate(float3x3 tangentToWorld, float3 viewDir_WS, float3 lightDir_WS, float3 lightColor);
    float3 evaluateAmbient();
    float getAlpha();
    float3 getTangentNormal();
};

struct Phong : IBRDF
{
    float3 baseColor;
    float alpha;
    float3 specular;
    float3 normal;
    float3 ambient;
    float shininess;

    __init()
    {
        baseColor = float3(0, 0, 0);
        alpha = 1;
        specular = float3(0, 0, 0);
        normal = float3(0, 0, 1);
        ambient = float3(0, 0, 0);
        shininess = 0;
    }

    float3 evaluate(float3x3 tangentToWorld, float3 viewDir_WS, float3 lightDir_WS, float3 lightColor)
    {
        float3 normal_WS = normalize(mul(tangentToWorld, normal));
        float3 nDotL = dot(normal_WS, lightDir_WS);
        float3 r = 2 * (nDotL) * normal_WS - lightDir_WS;
        float rDotV = dot(r, viewDir_WS);

        return lightColor * (baseColor * max(nDotL, 0.0)) + specular * pow(max(rDotV, 0.0), max(shininess, 1));
    }

    float3 evaluateAmbient()
    {
        return ambient;
    }
    float getAlpha()
    {
        return alpha;
    }
    float3 getTangentNormal()
    {
        return normal;
    }
};

struct BlinnPhong : IBRDF
{
    float3 baseColor;
    float alpha;
    float3 specularColor;
    float3 normal;
    float shininess;
    float3 ambient;

    __init()
    {
        baseColor = float3(0, 0, 0);
        alpha = 1;
        specularColor = float3(0, 0, 0);
        normal = float3(0, 0, 1);
        shininess = 0;
        ambient = float3(0, 0, 0);
    }

    float3 evaluate(float3x3 tangentToWorld, float3 viewDir_WS, float3 lightDir_WS, float3 lightColor)
    {
        float3 normal_WS = normalize(mul(tangentToWorld, normal));
        float diffuse = max(dot(normal_WS, lightDir_WS), 0);
        float3 h = normalize(lightDir_WS + viewDir_WS);
        float specular = pow(saturate(dot(normal_WS, h)), shininess);
		
        return (baseColor * diffuse * lightColor);// + (specularColor * specular);
    }

    float3 evaluateAmbient()
    {
        return ambient;
    }
    float getAlpha()
    {
        return alpha;
    }
    float3 getTangentNormal()
    {
        return normal;
    }
};

struct CelShading : IBRDF
{
    float3 baseColor;
    float alpha;
    float3 normal;

    __init()
    {
        baseColor = float3(0, 0, 0);
        alpha = 1;
        normal = float3(0, 0, 1);
    }

    float3 evaluate(float3x3 tangentToWorld, float3 viewDir_WS, float3 lightDir_WS, float3 lightColor)
    {
        float3 normal_WS = normalize(mul(tangentToWorld, normal));
        float nDotL = dot(normal_WS, lightDir_WS);
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
    float3 evaluateAmbient()
    {
        return float3(0, 0, 0);
    }
    float getAlpha()
    {
        return alpha;
    }
    float3 getTangentNormal()
    {
        return normal;
    }
};

// https://learnopengl.com/PBR/Theory
struct CookTorrance : IBRDF
{
    float3 baseColor;
    float alpha;
    float3 normal;
    float roughness;
    float metallic;
    float ambientOcclusion;

    __init()
    {
        baseColor = float3(0, 0, 0);
        alpha = 1;
        normal = float3(0, 0, 1);
        roughness = 0;
        metallic = 0;
        ambientOcclusion = 1;
    }

    float TrowbridgeReitzGGX(float3 normal, float3 halfway)
    {
        float a_sqr = roughness * roughness;
        float nDotH = max(dot(normal, halfway), 0.0);
        float nDotH_sqr = nDotH * nDotH;

        float denom = (nDotH_sqr * (a_sqr - 1.0) + 1.0);

        return a_sqr / (PI * denom * denom);
    }

    float SchlickGGX(float nDotV, float k)
    {
        return nDotV / (nDotV * (1.0 - k) + k);
    }

    float Smith(float3 normal, float3 view, float3 light)
    {
        float k = (roughness + 1);
        k = (k * k) / 8;
        float nDotV = max(dot(normal, view), 0.0);
        float nDotL = max(dot(normal, light), 0.0);
        float ggx1 = SchlickGGX(nDotV, k);
        float ggx2 = SchlickGGX(nDotL, k);
        return ggx1 * ggx2;
    }

    float3 FresnelSchlick(float cosTheta, float3 F0)
    {
        return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    }

    float3 evaluate(float3x3 tangentToWorld, float3 viewDir_WS, float3 lightDir_WS, float3 lightColor)
    {
        float3 n = normalize(mul(tangentToWorld, normal));
        float3 h = normalize(lightDir_WS + viewDir_WS);

        float3 F0 = float3(0.04);
        F0 = lerp(F0, baseColor, metallic);
        
        float3 F = FresnelSchlick(max(dot(h, viewDir_WS), 0.0), F0);

        float NDF = TrowbridgeReitzGGX(n, h);
        float G = Smith(n, viewDir_WS, lightDir_WS);

        float3 num = NDF * G * F;
        float denom = 4.0 * max(dot(n, viewDir_WS), 0.0) * max(dot(n, lightDir_WS), 0.0) + 0.000001;
        float3 specular = num / denom;

        float3 k_s = F;
        float3 k_d = float3(1.0) - k_s;

        k_d *= 1.0 - metallic;

        float nDotL = max(dot(n, lightDir_WS), 0.0);

        float3 result = (k_d * baseColor / PI + specular) * nDotL * lightColor;
        return result * ambientOcclusion;
    }
    float3 evaluateAmbient()
    {
        return float3(0.03) * baseColor * ambientOcclusion;
    }
    float getAlpha()
    {
        return alpha;
    }
    float3 getTangentNormal()
    {
        return normal;
    }
};
