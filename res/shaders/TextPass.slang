import Common;

struct GlyphInstanceData
{
    float x;
    float y;
    float z;
    float width;
    float height;
    uint glyphIndex;
};

struct TextData
{
    StructuredBuffer<GlyphInstanceData> glyphs;
    SamplerState glyphSampler;
    Texture2D<float> glyphTextures[];
};
ParameterBlock<TextData> pText;

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
    float xpos = pText.glyphs[input.instanceId].x;
    float ypos = pText.glyphs[input.instanceId].y;
    
    float w = pText.glyphs[input.instanceId].width;
    float h = pText.glyphs[input.instanceId].height;
    
    const float4 coordinates[4] = {
        float4(xpos,     ypos,     0, 0),
        float4(xpos,     ypos + h, 0, 1),
        float4(xpos + w, ypos,     1, 0),
        float4(xpos + w, ypos + h, 1, 1),
    };
    float4 vertex = coordinates[input.vertexId];
    VertexOutput output;
    output.texCoords = vertex.zw;
    output.position = mul(pViewParams.projectionMatrix, float4(vertex.xy, pText.glyphs[input.instanceId].z-1, 1));
    output.glyphIndex = pText.glyphs[input.instanceId].glyphIndex;
    return output;
}

[shader("fragment")]
float4 fragmentMain(
    float4 position : SV_Position,
    float2 texCoords : TEXCOORD,
    uint glyphIndex : GLYPHINDEX
) : SV_Target
{
    return float4(0,0,0,pText.glyphTextures[glyphIndex].Sample(pText.glyphSampler, texCoords));
}