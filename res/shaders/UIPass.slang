import Common;

struct RenderElementStyle
{
    float x;
    float y;
    float w;
    float h;
    float3 color;
    float opacity;
    float z;
    uint textureIndex;
    uint pad0;
    uint pad1;
};

struct UIParameter
{
    StructuredBuffer<RenderElementStyle> elements;
    SamplerState sampler;
    Texture2D textures[];
};
ParameterBlock<UIParameter> pParams;

struct VertexOutput
{
    float4 position  : SV_Position;
    float2 texCoords : TEXCOORD;
    RenderElementStyle style : STYLE;
};

[shader("vertex")]
VertexOutput vertexMain(uint vertexId : SV_VertexID, uint instanceId: SV_InstanceID)
{
    RenderElementStyle style = pParams.elements[instanceId];
    float xMin = style.x;
    float xMax = xMin + style.w;
    float yMin = style.y;
    float yMax = yMin + style.h;
    float4 coordinates[] = {
        float4(xMin, yMin, 0, 0),
        float4(xMin, yMax, 0, 1),
        float4(xMax, yMin, 1, 0),
        float4(xMax, yMax, 1, 1)
    };
    VertexOutput output;
    output.position = mul(pViewParams.projectionMatrix, float4(coordinates[vertexId].xy, style.z-1, 1));
    output.texCoords = coordinates[vertexId].zw;
    output.style = style;
    return output;
}

[shader("fragment")]
float4 fragmentMain(
    float2 texCoords : TEXCOORD,
    RenderElementStyle style : STYLE
) : SV_Target
{
    if(style.textureIndex == -1) {
        return float4(style.color, style.opacity);
    }
    return float4(pParams.textures[style.textureIndex].Sample(pParams.sampler, texCoords.xy).xyz, style.opacity);
}