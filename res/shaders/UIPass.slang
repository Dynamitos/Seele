import Common;

struct RenderElementStyle
{
    float3 position;
    uint backgroundImageIndex;
    float3 backgroundColor;
    float opacity;
    //float4 borderBottomColor;
    //float4 borderLeftColor;
    //float4 borderRightColor;
    //float4 borderTopColor;
    //float borderBottomLeftRadius;
    //float borderBottomRightRadius;
    //float borderTopLeftRadius;
    //float borderTopRightRadius;
    float2 dimensions;
};

struct UIParameter
{
    SamplerState backgroundSampler;
    uint numBackgroundTextures;
    Texture2D<float4> backgroundTextures[];
}

ParameterBlock<UIParameter> pParams;

struct VertexOutput
{
    float4 position  : SV_Position;
    float2 texCoords : TEXCOORD;
    RenderElementStyle style : RENDER_STYLE;
};

[shader("vertex")]
VertexOutput vertexMain(uint vertexId : SV_VertexID, RenderElementStyle style)
{
    float xMin = style.position.x;
    float xMax = xMin + style.dimensions.x;
    float yMin = style.position.y;
    float yMax = yMin + style.dimensions.y;
    float4 coordinates[] = {
        float4(xMin, yMin, 0, 0),
        float4(xMin, yMax, 0, 1),
        float4(xMax, yMin, 1, 0),
        float4(xMax, yMax, 1, 1)
    };
    VertexOutput output;
    output.position = mul(pViewParams.projectionMatrix, float4(coordinates[vertexId].xy, style.position.z, 1));
    output.texCoords = coordinates[vertexId].zw;
    output.style = style;
    return output;
}

[shader("fragment")]
float4 fragmentMain(
    float4 position : SV_Position,
    float2 texCoords : TEXCOORD,
    RenderElementStyle style : RENDER_STYLE
) : SV_Target
{
    float4 bgTextureColor = float4(1, 1, 1, 1);
    uint imageIndex = style.backgroundImageIndex;
    return float4(0, 1, 0, 1);
}