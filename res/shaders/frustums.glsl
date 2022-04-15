#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 5 0
struct ViewParameter_0
{
    mat4x4 viewMatrix_0;
    mat4x4 projectionMatrix_0;
    mat4x4 inverseProjection_0;
    vec4 cameraPos_WS_0;
    vec2 screenDimensions_0;
};

layout(binding = 0)
layout(std140) uniform _S1
{
    ViewParameter_0 _data;
} gViewParams_0;

#line 17
vec4 clipToView_0(vec4 clip_0)
{

    vec4 view_0 = (((clip_0) * (gViewParams_0._data.inverseProjection_0)));

    vec4 view_1 = view_0 / view_0.w;

    return view_1;
}


vec4 screenToView_0(vec4 screen_0)
{

    vec2 texCoord_0 = screen_0.xy / gViewParams_0._data.screenDimensions_0;

#line 36
    vec4 _S2 = clipToView_0(vec4(vec2(texCoord_0.x, 1.00000000000000000000 - texCoord_0.y) * 2.00000000000000000000 - 1.00000000000000000000, screen_0.z, screen_0.w));

#line 36
    return _S2;
}

struct Plane_0
{
    vec3 n_0;
    float d_0;
};


#line 49
Plane_0 computePlane_0(vec3 p0_0, vec3 p1_0, vec3 p2_0)
{
    Plane_0 plane_0;

#line 56
    vec3 _S3 = cross(p1_0 - p0_0, p2_0 - p0_0);

#line 56
    vec3 _S4 = normalize(_S3);

#line 56
    plane_0.n_0 = _S4;

    float _S5 = dot(plane_0.n_0, p0_0);

#line 58
    plane_0.d_0 = _S5;

    return plane_0;
}


#line 12 1
struct SLANG_ParameterGroup_DispatchParams_0
{
    uvec3 numThreadGroups_0;
    uint pad0_0;
    uvec3 numThreads_0;
    uint pad1_0;
};


#line 12
layout(binding = 1)
layout(std140) uniform _S6
{
    SLANG_ParameterGroup_DispatchParams_0 _data;
} DispatchParams_0;

#line 45 0
struct Frustum_0
{
    Plane_0  planes_0[4];
};


#line 20 1
layout(std430, binding = 2) buffer _S7 {
    Frustum_0 _data[];
} out_Frustums_0;

#line 3
struct ComputeShaderInput_0
{
    uvec3 groupID_0;
    uvec3 groupThreadID_0;
    uvec3 dispatchThreadID_0;
    uint groupIndex_0;
};


#line 25
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
    int i_0;

#line 25
    ComputeShaderInput_0 _S8 = ComputeShaderInput_0(gl_WorkGroupID, gl_LocalInvocationID, gl_GlobalInvocationID, gl_LocalInvocationIndex);

    const vec3 eyePos_0 = vec3(float(0), float(0), float(0));

    vec4  screenSpace_0[4];

    screenSpace_0[0] = vec4(vec2(_S8.dispatchThreadID_0.xy * uint(32)), -1.00000000000000000000, 1.00000000000000000000);
    screenSpace_0[1] = vec4(vec2(float(_S8.dispatchThreadID_0.x + uint(1)), float(_S8.dispatchThreadID_0.y)) * float(uint(32)), -1.00000000000000000000, 1.00000000000000000000);
    screenSpace_0[2] = vec4(vec2(float(_S8.dispatchThreadID_0.x), float(_S8.dispatchThreadID_0.y + uint(1))) * float(uint(32)), -1.00000000000000000000, 1.00000000000000000000);
    screenSpace_0[3] = vec4(vec2(float(_S8.dispatchThreadID_0.x + uint(1)), float(_S8.dispatchThreadID_0.y + uint(1))) * float(uint(32)), -1.00000000000000000000, 1.00000000000000000000);


    vec3  viewSpace_0[4];

    i_0 = 0;
    for(;;)
    {

#line 39
        if(i_0 < 4)
        {
        }
        else
        {
            break;
        }

#line 41
        vec4 _S9 = screenToView_0(screenSpace_0[i_0]);

#line 41
        viewSpace_0[i_0] = _S9.xyz;

#line 39
        int _S10 = i_0 + int(1);

#line 39
        i_0 = _S10;
    }

#line 45
    Frustum_0 frustum_0;

    Plane_0 _S11 = computePlane_0(eyePos_0, viewSpace_0[2], viewSpace_0[0]);

#line 47
    frustum_0.planes_0[0] = _S11;
    Plane_0 _S12 = computePlane_0(eyePos_0, viewSpace_0[1], viewSpace_0[3]);

#line 48
    frustum_0.planes_0[1] = _S12;
    Plane_0 _S13 = computePlane_0(eyePos_0, viewSpace_0[0], viewSpace_0[1]);

#line 49
    frustum_0.planes_0[2] = _S13;
    Plane_0 _S14 = computePlane_0(eyePos_0, viewSpace_0[3], viewSpace_0[2]);

#line 50
    frustum_0.planes_0[3] = _S14;

    if(_S8.dispatchThreadID_0.x < DispatchParams_0._data.numThreads_0.x && _S8.dispatchThreadID_0.y < DispatchParams_0._data.numThreads_0.y)
    {

        ((out_Frustums_0)._data[(_S8.dispatchThreadID_0.x + _S8.dispatchThreadID_0.y * DispatchParams_0._data.numThreads_0.x)]) = frustum_0;

#line 52
    }
    else
    {

#line 52
    }

#line 57
    return;
}

