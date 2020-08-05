import LightEnv;
import BRDF;
import Material;
import TexturedMaterial;
import FlatColorMaterial;
import Common;
//TODO revert to pre processed shader attributes
import StaticMeshShaderAttributes;
import PrimitiveSceneData;
import MaterialParameter;

//layout(set = 0, binding = 2)
//StructuredBuffer<uint> lightIndexList;
//layout(set = 0, binding = 3)
//RWTexture2D<uint2> lightGrid;

layout(set = 1, std430)
//type_param TMaterial : IMaterial;
ParameterBlock<TexturedMaterial> gMaterial;

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
	float3 worldPosition = input.getWorldPosition(cache);
    float4 clipSpacePosition;
    
    float3x3 tangentToLocal = cache.tangentToLocal;
    MaterialVertexParameter vertexParams = input.getMaterialVertexParameters(cache, worldPosition, tangentToLocal);

    float4 viewSpacePosition = mul(gViewParams.viewMatrix, float4(worldPosition, 1));
    clipSpacePosition = mul(gViewParams.projectionMatrix, viewSpacePosition);
    output.position = clipSpacePosition;
    output.shaderAttributeInterpolation = input.getInterpolants(cache, vertexParams);
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
	BlinnPhong brdf = gMaterial.prepare(materialParams);
	
	float3 viewDir = normalize(materialParams.viewDir);
    
    float3 result = float3(0, 0, 0);

    for (int i = 0; i < 5; i++)
    {
        result += gLightEnv.directionalLights[i].illuminate(materialParams, brdf, viewDir);
    }
    
	uint2 tileIndex = uint2(floor(materialParams.clipPosition.xy) / BLOCK_SIZE);

	//uint startOffset = lightGrid[tileIndex].x;
	//uint lightCount = lightGrid[tileIndex].y;

    for (int j = 0; j < gLightEnv.numPointLights; ++j)
    {
		//uint lightIndex = lightIndexList[startOffset + j];
		PointLight pointLight = gLightEnv.pointLights[j];
        result += pointLight.illuminate(materialParams, brdf, viewDir);
    }
	
    
    return float4(0, 1, 0, 1);
}
