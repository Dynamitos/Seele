#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 1 0
struct RenderElementStyle_0
{
    vec3 position_0;
    uint backgroundImageIndex_0;
    vec3 backgroundColor_0;
    float opacity_0;
    vec4 borderBottomColor_0;
    vec4 borderLeftColor_0;
    vec4 borderRightColor_0;
    vec4 borderTopColor_0;
    float borderBottomLeftRadius_0;
    float borderBottomRightRadius_0;
    float borderTopLeftRadius_0;
    float borderTopRightRadius_0;
    vec2 dimensions_0;
};


#line 33
layout(std430, binding = 3) readonly buffer _S1 {
    RenderElementStyle_0 _data[];
} elements_0;

#line 33
layout(location = 0)
out vec2 _S2;


#line 33
layout(location = 1)
out uint _S3;



struct VertexStageOutput_0
{
    vec4 position_1;
    vec2 texCoords_0;
    uint elementId_0;
};


void main()
{

#line 46
    uint _S4 = uint(gl_VertexIndex);

#line 46
    uint _S5 = uint(gl_InstanceIndex);

    RenderElementStyle_0 style_0 = ((elements_0)._data[(_S5)]);
    float xMin_0 = style_0.position_0.x;
    float xMax_0 = xMin_0 + style_0.dimensions_0.x;
    float yMin_0 = style_0.position_0.y;
    float yMax_0 = yMin_0 + style_0.dimensions_0.y;
    vec4  coordinates_0[4] = { vec4(xMin_0, yMin_0, float(0), float(0)), vec4(xMin_0, yMax_0, float(0), float(1)), vec4(xMax_0, yMin_0, float(1), float(0)), vec4(xMax_0, yMax_0, float(1), float(1)) };

#line 59
    VertexStageOutput_0 output_0;
    output_0.position_1 = vec4(coordinates_0[_S4].xy, style_0.position_0.z, float(1));
    output_0.texCoords_0 = coordinates_0[_S4].zw;
    output_0.elementId_0 = _S5;
    VertexStageOutput_0 _S6 = output_0;

#line 63
    gl_Position = _S6.position_1;

#line 63
    _S2 = _S6.texCoords_0;

#line 63
    _S3 = _S6.elementId_0;

#line 63
    return;
}

