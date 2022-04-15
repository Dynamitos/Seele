#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 18 0
layout(binding = 1, set = 1)
uniform texture2D  glyphTextures_0[];


#line 14
layout(binding = 1)
uniform sampler glyphSampler_0;


#line 14
layout(location = 0)
out vec4 _S1;


#line 62
layout(location = 0)
in vec2 _S2;


#line 62
flat layout(location = 1)
in uint _S3;


#line 62
void main()
{

#line 68
    vec4 _S4 = (texture(sampler2D(glyphTextures_0[_S3],glyphSampler_0), (_S2)));

#line 68
    _S1 = _S4;

#line 68
    return;
}

