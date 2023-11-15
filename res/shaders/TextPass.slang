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
ParameterBlock<SamplerState> glyphSampler;
//layout(set = 1)
//ShaderBuffer<GlyphData> glyphData;
ParameterBuffer<Texture2D<uint>[]> pGlyphTextures;

struct GlyphInstanceData
{
    float2 position;
    float2 widthHeight;
    uint glyphIndex;
};
struct TextRender
{
    StructuredBuffer<GlyphInstanceData> instances;
    TextData textData;
}
ParameterBuffer<TextRender> pRender;

struct VertexInput
{
    uint vertexId : SV_VertexID;
    uint instanceId : SV_InstanceID;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD;
    uint glyphIndex : GLYPHINDEX;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    float xpos = pRender.instances[input.instanceId].position.x;
    float ypos = pRender.instances[input.instanceId].position.y;
    
    float w = pRender.instances[input.instanceId].widthHeight.x;
    float h = pRender.instances[input.instanceId].widthHeight.y;
    
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
    output.glyphIndex = pRender.instances[input.instanceId].glyphIndex;
    return output;
}

[shader("fragment")]
float4 fragmentMain(
    float4 position : SV_Position,
    float2 texCoords : TEXCOORD,
    uint glyphIndex : GLYPHINDEX
) : SV_Target
{
    return textData.textColor * pGlyphTextures[glyphIndex].Sample(glyphSampler, texCoords);
}