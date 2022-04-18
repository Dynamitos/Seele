#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 30 0
layout(binding = 2)
layout(std140) uniform _S1
{
    uint _data;
} numBackgroundTextures_0;

#line 33
layout(binding = 3)
uniform texture2D  backgroundTextures_0[];


#line 27
layout(binding = 1)
uniform sampler backgroundSampler_0;


#line 27
layout(location = 0)
out vec4 _S2;


#line 63
layout(location = 0)
in vec2 _S3;


#line 1
layout(location = 1)
in vec3 _S4;


#line 1
flat layout(location = 2)
in uint _S5;


#line 1
layout(location = 3)
in vec3 _S6;


#line 1
layout(location = 4)
in float _S7;


#line 1
layout(location = 5)
in vec2 _S8;


#line 1
struct RenderElementStyle_0
{
    vec3 position_0;
    uint backgroundImageIndex_0;
    vec3 backgroundColor_0;
    float opacity_0;
    vec2 dimensions_0;
};


#line 63
void main()
{
    vec4 bgTextureColor_0;

#line 63
    RenderElementStyle_0 _S9 = RenderElementStyle_0(_S4, _S5, _S6, _S7, _S8);

#line 69
    const vec4 _S10 = vec4(float(1), float(1), float(1), float(1));

#line 69
    uint imageIndex_0 = _S9.backgroundImageIndex_0;

    if(imageIndex_0 < numBackgroundTextures_0._data)
    {
        vec4 _S11 = (texture(sampler2D(backgroundTextures_0[imageIndex_0],backgroundSampler_0), (_S3)));

#line 71
        bgTextureColor_0 = _S11;
    }
    else
    {

#line 71
        bgTextureColor_0 = _S10;
    }

#line 71
    _S2 = vec4(_S9.backgroundColor_0, _S9.opacity_0) * bgTextureColor_0;

#line 71
    return;
}

