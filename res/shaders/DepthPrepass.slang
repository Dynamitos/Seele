import Material;
import VERTEX_INPUT_IMPORT;
import MATERIAL_IMPORT;

struct ViewParams
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
    float4 cameraPos_WS;
};

layout(set = 0, binding = 0)
ParameterBlock<ViewParams> gViewParams;


layout(set = 1)
type_param TMaterial : IMaterial;
ParameterBlock<TMaterial> gMaterial;

struct ModelParameter
{
	float4x4 modelMatrix;
}

[[vk::push_constant]]
ConstantBuffer<ModelParameter> gModelParams;

struct VertexStageOutput
{
	float4 position : SV_Position;
}
[shader("vertex")]
VertexStageOutput vertexMain(PositionOnlyVertexShaderInput input)
{ 
	VertexStageOutput output;
    
	float3 worldPosition = input.getWorldPosition();
    worldPosition += gMaterial.getWorldOffset();
    float4 clipSpacePosition;
    
    float4 viewSpacePosition = mul(gViewParams.viewMatrix, float4(worldPosition, 1));
    clipSpacePosition = mul(gViewParams.projectionMatrix, viewSpacePosition);
    output.position = clipSpacePosition;
    return output;
}