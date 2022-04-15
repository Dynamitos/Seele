#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 7 0
struct GlyphData_0
{
    vec2 bearing_0;
    vec2 size_0;
};


layout(std430, binding = 1) readonly buffer _S1 {
    GlyphData_0 _data[];
} glyphData_0;

#line 1
struct ViewParameter_0
{
    mat4x4 projectionMatrix_0;
};


#line 5
layout(binding = 0)
layout(std140) uniform _S2
{
    ViewParameter_0 _data;
} gViewParams_0;

#line 5
layout(location = 0)
out vec2 _S3;


#line 5
layout(location = 1)
out uint _S4;


#line 5
layout(location = 0)
in uint _S5;


#line 5
layout(location = 1)
in vec2 _S6;


#line 5
layout(location = 2)
in float _S7;


#line 20
struct VertexStageInput_0
{
    uint glyphIndex_0;
    vec2 position_0;
    float scale_0;
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

#line 36
    VertexStageInput_0 _S8 = VertexStageInput_0(_S5, _S6, _S7, uint(gl_VertexIndex));

    GlyphData_0 glyph_0 = ((glyphData_0)._data[(_S8.glyphIndex_0)]);

    float xpos_0 = _S8.position_0.x + glyph_0.bearing_0.x * _S8.scale_0;
    float ypos_0 = _S8.position_0.y - (glyph_0.size_0.y - glyph_0.bearing_0.y) * _S8.scale_0;

    float w_0 = glyph_0.size_0.x * _S8.scale_0;


    vec4  coordinates_0[4] = { vec4(xpos_0, ypos_0 + glyph_0.size_0.y * _S8.scale_0, float(0), float(0)), vec4(xpos_0, ypos_0, float(0), float(1)), vec4(xpos_0 + w_0, ypos_0, float(1), float(0)), vec4(xpos_0 + w_0, ypos_0 + w_0, float(1), float(1)) };

#line 46
    vec4 vertex_0 = coordinates_0[_S8.vertexId_0];

#line 53
    VertexStageOutput_0 output_0;
    output_0.uvCoords_0 = vertex_0.zw;
    vec4 _S9 = (((vec4(vertex_0.xy, float(0), float(1))) * (gViewParams_0._data.projectionMatrix_0)));

#line 55
    output_0.position_1 = _S9;
    output_0.glyphIndex_1 = _S8.glyphIndex_0;
    VertexStageOutput_0 _S10 = output_0;

#line 57
    gl_Position = _S10.position_1;

#line 57
    _S3 = _S10.uvCoords_0;

#line 57
    _S4 = _S10.glyphIndex_1;

#line 57
    return;
}

