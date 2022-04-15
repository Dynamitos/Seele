
struct GlyphData
{
    float2 bearing;
    float2 size;
};
struct ViewData
{
    float4x4 projectionMatrix;
}
layout(set = 0, binding = 0)
ConstantBuffer<ViewData> viewData;
layout(set = 0, binding = 1)
SamplerState glyphSampler;
layout(set = 1)
RWStructuredBuffer<GlyphData> glyphData;
layout(set = 2)
Texture2D<uint> glyphTextures[];
layout(push_constant)
ConstantBuffer<float> scale;

struct VertexStageInput
{
    uint glyphIndex;
    float2 position;
    uint vertexId : SV_VertexID;
};

struct VertexStageOutput
{
    float4 position : SV_Position;
    float2 uvCoords : TEXCOORD;
    uint glyphIndex : GLYPHINDEX;
};

[shader("vertex")]
VertexStageOutput vertexMain(VertexStageInput input)
{
    GlyphData glyph = glyphData[input.glyphIndex];
    
    float xpos = input.position.x + glyph.bearing.x * scale;
    float ypos = input.position.y - (glyph.size.y - glyph.bearing.y) * scale;
    
    float w = glyph.size.x * scale;
    float h = glyph.size.y * scale;
    
    float4 coordinates[4] = {
        float4(xpos,     ypos,     0, 1),
        float4(xpos,     ypos + h, 0, 0),
        float4(xpos + w, ypos,     1, 1),
        float4(xpos + w, ypos + h, 1, 0)
    };
    float4 vertex = coordinates[input.vertexId];
    VertexStageOutput output;
    output.uvCoords = vertex.zw;
    output.position = mul(viewData.projectionMatrix, float4(vertex.xy, 0, 1));
    output.glyphIndex = input.glyphIndex;
    return output;
}

[shader("fragment")]
float4 fragmentMain(
    float4 position : SV_Position,
    float2 uvCoords : TEXCOORD,
    uint glyphIndex : GLYPHINDEX
) : SV_Target
{
    return glyphTextures[glyphIndex].Sample(glyphSampler, uvCoords);
}