import Common;

struct DebugVertex
{
    float3 position;
    float3 color;
};

struct VertexStageOutput
{
    float3 color : VERTEX_COLOR;
    float4 position : SV_Position;
};

[shader("vertex")]
VertexStageOutput vertexMain(
    DebugVertex input)
{
    VertexStageOutput output;
    float4 viewpos = mul(gViewParams.viewMatrix, float4(input.position, 1.0f));
    output.position = mul(gViewParams.projectionMatrix, viewpos);
    output.color = input.color;
    return output;
}


[shader("fragment")]
float4 fragmentMain(
    float3 color : VERTEX_COLOR,
    float4 position : SV_Position
    ) : SV_Target
{
    return float4(color, 1.0f);
}