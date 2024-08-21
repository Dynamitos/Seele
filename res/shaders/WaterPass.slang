import Common;
import WaterCommon;

float SchlickFresnel(float3 normal, float3 viewDir) {
    // 0.02f comes from the reflectivity bias of water kinda idk it's from a paper somewhere i'm not gonna link it tho lmaooo
    return 0.02f + (1 - 0.02f) * (pow(1 - clamp(dot(normal, viewDir), 0, 1), 5.0f));
}

float SmithMaskingBeckmann(float3 H, float3 S, float roughness) {
    float hdots = max(0.001f, clamp(dot(H, S), 0, 1));
    float a = hdots / (roughness * sqrt(1 - hdots * hdots));
    float a2 = a * a;

    return a < 1.6f ? (1.0f - 1.259f * a + 0.396f * a2) / (3.535f * a + 2.181 * a2) : 0.0f;
}

float Beckmann(float ndoth, float roughness) {
    float exp_arg = (ndoth * ndoth - 1) / (roughness * roughness * ndoth * ndoth);

    return exp(exp_arg) / (PI * roughness * roughness * ndoth * ndoth * ndoth * ndoth);
}

[shader("pixel")]
float4 main(WaterVertex vert) : SV_TARGET {
    float3 lightDir = -normalize(pWaterMaterial.sunDirection);
    float3 viewDir = normalize(pViewParams.cameraPos_WS.xyz - vert.position_WS);
    float3 halfwayDir = normalize(lightDir + viewDir);
    float depth = vert.depth;
    float LdotH = clamp(dot(lightDir, halfwayDir), 0, 1);
    float VdotH = clamp(dot(viewDir, halfwayDir), 0, 1);
    
    
    float4 displacementFoam1 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile1, 0));
    displacementFoam1.a += pWaterMaterial.foamSubtract0;
    float4 displacementFoam2 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile2, 0));
    displacementFoam2.a += pWaterMaterial.foamSubtract1;
    float4 displacementFoam3 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile3, 0));
    displacementFoam3.a += pWaterMaterial.foamSubtract2;
    float4 displacementFoam4 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile4, 0));
    displacementFoam4.a += pWaterMaterial.foamSubtract3;
    float4 displacementFoam = displacementFoam1 + displacementFoam2 + displacementFoam3 + displacementFoam4;

    float2 slopes1 = pWaterMaterial.slopeTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile1, 0));
    float2 slopes2 = pWaterMaterial.slopeTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile2, 1));
    float2 slopes3 = pWaterMaterial.slopeTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile3, 2));
    float2 slopes4 = pWaterMaterial.slopeTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile4, 3));
    float2 slopes = slopes1 + slopes2 + slopes3 + slopes4;
    
    slopes *= pWaterMaterial.normalStrength;
    float foam = lerp(0.0f, saturate(displacementFoam.a), pow(depth, pWaterMaterial.foamDepthAttenuation));

    float3 macroNormal = float3(0, 1, 0);
    float3 mesoNormal = normalize(float3(-slopes.x, 1.0f, -slopes.y));
    mesoNormal = normalize(lerp(macroNormal, mesoNormal, pow(saturate(depth), pWaterMaterial.normalDepthAttenuation)));
    mesoNormal = normalize(mesoNormal);

    float NdotL = clamp(dot(mesoNormal, lightDir), 0, 1);

    
    float a = pWaterMaterial.roughness + foam * pWaterMaterial.foamRoughnessModifier;
    float ndoth = max(0.0001f, dot(mesoNormal, halfwayDir));

    float viewMask = SmithMaskingBeckmann(halfwayDir, viewDir, a);
    float lightMask = SmithMaskingBeckmann(halfwayDir, lightDir, a);
    
    float G = rcp(1 + viewMask + lightMask);

    float eta = 1.33f;
    float R = ((eta - 1) * (eta - 1)) / ((eta + 1) * (eta + 1));
    float thetaV = acos(viewDir.y);

    float numerator = pow(1 - dot(mesoNormal, viewDir), 5 * exp(-2.69 * a));
    float F = R + (1 - R) * numerator / (1.0f + 22.7f * pow(a, 1.5f));
    F = saturate(F);
    
    float3 specular = pWaterMaterial.sunIrradiance * F * G * Beckmann(ndoth, a);
    specular /= 4.0f * max(0.001f, clamp(dot(macroNormal, lightDir), 0, 1));
    specular *= clamp(dot(mesoNormal, lightDir), 0, 1);

    float3 envReflection = pWaterMaterial.environmentMap.Sample(pWaterMaterial.sampler, reflect(-viewDir, mesoNormal)).rgb;
    envReflection *= pWaterMaterial.environmentLightStrength;

    float H = max(0.0f, displacementFoam.y) * pWaterMaterial.heightModifier;
    float3 scatterColor = pWaterMaterial.scatterColor;
    float3 bubbleColor = pWaterMaterial.bubbleColor;
    float bubbleDensity = pWaterMaterial.bubbleDensity;

    
    float k1 = pWaterMaterial.wavePeakScatterStrength * H * pow(clamp(dot(lightDir, -viewDir), 0, 1), 4.0f) * pow(0.5f - 0.5f * dot(lightDir, mesoNormal), 3.0f);
    float k2 = pWaterMaterial.scatterStrength * pow(clamp(dot(viewDir, mesoNormal), 0, 1), 2.0f);
    float k3 = pWaterMaterial.scatterShadowStrength * NdotL;
    float k4 = bubbleDensity;

    float3 scatter = (k1 + k2) * scatterColor * pWaterMaterial.sunIrradiance * rcp(1 + lightMask);
    scatter += k3 * scatterColor * pWaterMaterial.sunIrradiance + k4 * bubbleColor * pWaterMaterial.sunIrradiance;

    
    float3 output = (1 - F) * scatter + specular + F * envReflection;
    output = max(0.0f, output);
    output = lerp(output, pWaterMaterial.foamColor, saturate(foam));

    return float4(output, 1.0f);
}
