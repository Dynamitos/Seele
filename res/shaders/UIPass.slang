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

ParameterBuffer<UIParameter> params;

struct VertexStageOutput
{
    float4 position  : SV_Position;
    float2 texCoords : TEXCOORD;
    RenderElementStyle style : RENDER_STYLE;
};

[shader("vertex")]
VertexStageOutput vertexMain(uint vertexId : SV_VertexID, RenderElementStyle style)
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
    VertexStageOutput output;
    output.position = mul(viewData.projectionMatrix, float4(coordinates[vertexId].xy, style.position.z, 1));
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
    if(imageIndex < numBackgroundTextures)
    {
        bgTextureColor = backgroundTextures[imageIndex].Sample(backgroundSampler, texCoords);
    }
    return float4(style.backgroundColor, style.opacity) * bgTextureColor;
}