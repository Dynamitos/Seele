import Common;
import Material;
import VERTEX_INPUT_IMPORT;
import MATERIAL_IMPORT;
import PrimitiveSceneData;

struct ModelParameter
{
	float4x4 modelMatrix;
}

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