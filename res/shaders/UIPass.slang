struct VertexStageOutput
{
    float4 position : SV_Position;
    float2 uvCoords : TEXCOORD;
};


[shader("vertex")]
VertexStageOutput vertexMain(uint vertexId : SV_VertexID)
{
    float2 coordinates[4] = {
        float2(0, 0),
        float2(0, 1),
        float2(1, 0),
        float2(1, 1)
    };
    VertexStageOutput output;
    output.uvCoords = coordinates[vertexId];
    output.position = float4(output.uvCoords * 2 - 1, 1, 1);
    return output;
}

[shader("fragment")]
float4 fragmentMain(
    float4 position : SV_Position,
    float2 uvCoords : TEXCOORD
) : SV_Target
{
    return float4(0, 1, 0, 1);
}