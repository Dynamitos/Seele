import Common;
import LightEnv;
import BRDF;
import Material;

import VERTEX_INPUT_IMPORT;
import MATERIAL_IMPORT;
import PrimitiveSceneData;
import MaterialParameter;

layout(set = INDEX_LIGHT_ENV, binding = 4)
StructuredBuffer<uint> lightIndexList;
layout(set = INDEX_LIGHT_ENV, binding = 5)
RWTexture2D<uint2> lightGrid;


struct VertexStageOutput
{
    ShaderAttributeInterpolation shaderAttributeInterpolation : Interpolation;
    VertexValueCache cache : ShaderCache;
    float4 position : SV_Position;
};

[shader("vertex")]
VertexStageOutput vertexMain(
    VertexShaderInput input)
{
    VertexStageOutput output;
    VertexValueCache cache = input.getVertexCache();
    float3 worldPosition = input.getWorldPosition();
    worldPosition += gMaterial.getWorldOffset();
    float4 clipSpacePosition;
    
    float3x3 tangentToLocal = cache.getTangentToLocal();
    float4 viewSpacePosition = mul(gViewParams.viewMatrix, float4(worldPosition, 1));
    MaterialVertexParameter vertexParams = input.getMaterialVertexParameters(cache, worldPosition, viewSpacePosition.xyz, tangentToLocal);

    clipSpacePosition = mul(gViewParams.projectionMatrix, viewSpacePosition);
    output.position = clipSpacePosition;
    output.shaderAttributeInterpolation = input.getInterpolants(cache, vertexParams);
    output.cache = cache;
    return output;
}

[shader("fragment")]
float4 fragmentMain(
    ShaderAttributeInterpolation input : Interpolation,
    VertexValueCache cache : ShaderCache,
    float4 position : SV_Position
    ) : SV_Target
{
    MaterialFragmentParameter materialParams = input.getMaterialParameter(position);
    TMaterial.BRDF brdf = gMaterial.prepare(materialParams);
    
    float3 viewDir = normalize(materialParams.viewDir);
    
    float3 result = float3(0, 0, 0);

    for (int i = 0; i < numDirectionalLights; i++)
    {
        result += directionalLights[i].illuminate(materialParams, brdf, viewDir);
    }
    
    uint2 tileIndex = uint2(floor(position.xy / BLOCK_SIZE));
    uint2 gridValue = lightGrid[tileIndex];
    uint startOffset = gridValue.x;
    uint lightCount = gridValue.y;

    for (int j = 0; j < numPointLights; ++j)
    {
        //uint lightIndex = lightIndexList[startOffset + j];
        PointLight pointLight = pointLights[j];
        if(pointLight.colorRange.w < length(pointLight.getViewPos() - input.viewPosition)) continue;
        result += pointLight.illuminate(materialParams, brdf, viewDir);
    }
    return float4(result, 1.0f);
}

