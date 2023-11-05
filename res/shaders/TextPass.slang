import Common;

struct GlyphData
{
    float4 bearingSize;
};
struct TextData
{
    float4 textColor;
    float scale;
}
ParameterBuffer<SamplerState> glyphSampler;
//layout(set = 1)
//ShaderBuffer<GlyphData> glyphData;
ParameterBuffer<Texture2D<uint>[]> glyphTextures;
[[vk::push_constant]]
ConstantBuffer<TextData> textData;

struct VertexStageInput
{
    float2 position;
    float2 widthHeight;
    uint glyphIndex;
    uint vertexId : SV_VertexID;
};

struct VertexStageOutput
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD;
    uint glyphIndex : GLYPHINDEX;
};

[shader("vertex")]
VertexStageOutput vertexMain(VertexStageInput input)
{
    float xpos = input.position.x;
    float ypos = input.position.y;
    
    float w = input.widthHeight.x;
    float h = input.widthHeight.y;
    
    float4 coordinates[4] = {
        float4(xpos,     ypos,     0, 1),
        float4(xpos,     ypos + h, 0, 0),
        float4(xpos + w, ypos,     1, 1),
        float4(xpos + w, ypos + h, 1, 0)
    };
    float4 vertex = coordinates[input.vertexId];
    VertexStageOutput output;
    output.texCoords = vertex.zw;
    output.position = mul(viewData.projectionMatrix, float4(vertex.xy, 0, 1));
    output.glyphIndex = input.glyphIndex;
    return output;
}

[shader("fragment")]
float4 fragmentMain(
    float4 position : SV_Position,
    float2 texCoords : TEXCOORD,
    uint glyphIndex : GLYPHINDEX
) : SV_Target
{
    return textData.textColor * glyphTextures[glyphIndex].Sample(glyphSampler, texCoords);
}