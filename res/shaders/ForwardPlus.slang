import InputGeometry;
import LightEnv;
import BRDF;
import Material;
import TexturedMaterial;
import FlatColorMaterial;
import Common;

struct ViewParameter
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4 cameraPos_WS;
}
layout(set = 0, binding = 0, std430)
ConstantBuffer<ViewParameter> gViewParams;

layout(set = 0, binding = 1, std430)
ConstantBuffer<Lights> gLightEnv;
layout(set = 0, binding = 2)
StructuredBuffer<uint> lightIndexList;
layout(set = 0, binding = 3)
RWTexture2D<uint2> lightGrid;

layout(set = 1, std430)
type_param TMaterial : IMaterial;
ParameterBlock<TMaterial> gMaterial;

struct ModelParameter
{
	float4x4 modelMatrix;
}

layout(set = 2)
ConstantBuffer<ModelParameter> gModelParams;

struct VertexStageOutput
{
    MaterialPixelParameter materialParameter : MaterialParameter;
	float4 sv_position : SV_Position;
};

[shader("vertex")]
VertexStageOutput vertexMain(
	InputGeometry inputGeometry)
{
	VertexStageOutput output;
    MaterialPixelParameter pixelParams;
    float4 worldPosition = mul(gModelParams.modelMatrix, float4(inputGeometry.getVertexPosition(), 1));
    pixelParams.position = worldPosition.xyz;
    pixelParams.texCoord = inputGeometry.texCoord;

    float4 viewPosition = mul(gViewParams.viewMatrix, worldPosition);
    pixelParams.viewDir = gViewParams.cameraPos_WS.xyz - worldPosition.xyz;
    
    pixelParams.normal = mul(gModelParams.modelMatrix, float4(inputGeometry.getNormal(), 0)).xyz;
    pixelParams.tangent = mul(gModelParams.modelMatrix, float4(inputGeometry.getTangent(), 0)).xyz;
    pixelParams.biTangent = mul(gModelParams.modelMatrix, float4(inputGeometry.getBiTangent(), 0)).xyz;
    pixelParams.clipPosition = mul(gViewParams.projectionMatrix, viewPosition);

    output.materialParameter = pixelParams;
	output.sv_position = pixelParams.clipPosition;

	return output;
}

[shader("fragment")]
float4 fragmentMain(MaterialPixelParameter input : MaterialParameter) : SV_Target
{
	TMaterial.BRDF brdf = gMaterial.prepare(input);
	
	float3 viewDir = normalize(input.viewDir);
    
    float3 result = float3(0, 0, 0);
    for (int i = 0; i < gLightEnv.numDirectionalLights; ++i)
    {
        result += gLightEnv.directionalLights[i].illuminate(input, brdf, viewDir);
    }
	uint2 tileIndex = uint2(floor(input.clipPosition.xy) / BLOCK_SIZE);

	uint startOffset = lightGrid[tileIndex].x;
	uint lightCount = lightGrid[tileIndex].y;

    for (int j = 0; j < lightCount; ++j)
    {
		uint lightIndex = lightIndexList[startOffset + j];
		PointLight pointLight = gLightEnv.pointLights[lightIndex];
        result += pointLight.illuminate(input, brdf, viewDir);
    }
	
    return float4(result, 1);
}
