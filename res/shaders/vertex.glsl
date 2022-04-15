#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 2 0
struct GlyphData_0
{
    vec2 bearing_0;
    vec2 size_0;
};


#line 16
layout(std430, binding = 0, set = 1) readonly buffer _S1 {
    GlyphData_0 _data[];
} glyphData_0;

layout(push_constant)
layout(std140) uniform _S2
{
    float _data;
} scale_0;

#line 7
struct ViewData_0
{
    mat4x4 projectionMatrix_0;
};

layout(binding = 0)
layout(std140) uniform _S3
{
    ViewData_0 _data;
} viewData_0;

#line 12
layout(location = 0)
out vec2 _S4;


#line 12
layout(location = 1)
out uint _S5;


#line 12
layout(location = 0)
in uint _S6;


#line 12
layout(location = 1)
in vec2 _S7;


#line 22
struct VertexStageInput_0
{
    uint glyphIndex_0;
    vec2 position_0;
    uint vertexId_0;
};

struct VertexStageOutput_0
{
    vec4 position_1;
    vec2 uvCoords_0;
    uint glyphIndex_1;
};


void main()
{

#line 37
    VertexStageInput_0 _S8 = VertexStageInput_0(_S6, _S7, uint(gl_VertexIndex));

    GlyphData_0 glyph_0 = ((glyphData_0)._data[(_S8.glyphIndex_0)]);

    float xpos_0 = _S8.position_0.x + glyph_0.bearing_0.x * scale_0._data;
    float ypos_0 = _S8.position_0.y - (glyph_0.size_0.y - glyph_0.bearing_0.y) * scale_0._data;

    float w_0 = glyph_0.size_0.x * scale_0._data;
    float h_0 = glyph_0.size_0.y * scale_0._data;

    vec4  coordinates_0[4] = { vec4(xpos_0, ypos_0 + h_0, float(0), float(0)), vec4(xpos_0, ypos_0, float(0), float(1)), vec4(xpos_0 + w_0, ypos_0, float(1), float(0)), vec4(xpos_0 + w_0, ypos_0 + h_0, float(1), float(1)) };

#line 47
    vec4 vertex_0 = coordinates_0[_S8.vertexId_0];

#line 54
    VertexStageOutput_0 output_0;
    output_0.uvCoords_0 = vertex_0.zw;
    vec4 _S9 = (((vec4(vertex_0.xy, float(0), float(1))) * (viewData_0._data.projectionMatrix_0)));

#line 56
    output_0.position_1 = _S9;
    output_0.glyphIndex_1 = _S8.glyphIndex_0;
    VertexStageOutput_0 _S10 = output_0;

#line 58
    gl_Position = _S10.position_1;

#line 58
    _S4 = _S10.uvCoords_0;

#line 58
    _S5 = _S10.glyphIndex_1;

#line 58
    return;
}

