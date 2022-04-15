#version 450
#extension GL_EXT_samplerless_texture_functions : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 21 0
layout(binding = 2)
uniform texture2D depthTextureVS_0;


#line 46
shared uint uMinDepth_0;


#line 47
shared uint uMaxDepth_0;



shared uint oLightCount_0;



shared uint tLightCount_0;


#line 12
struct SLANG_ParameterGroup_DispatchParams_0
{
    uvec3 numThreadGroups_0;
    uint pad0_0;
    uvec3 numThreads_0;
    uint pad1_0;
};


#line 12
layout(binding = 1)
layout(std140) uniform _S1
{
    SLANG_ParameterGroup_DispatchParams_0 _data;
} DispatchParams_0;

#line 39 1
struct Plane_0
{
    vec3 n_0;
    float d_0;
};

struct Frustum_0
{
    Plane_0  planes_0[4];
};


#line 23 0
layout(std430, binding = 3) readonly buffer _S2 {
    Frustum_0 _data[];
} frustums_0;

#line 49
shared Frustum_0 groupFrustum_0;


#line 5 1
struct ViewParameter_0
{
    mat4x4 viewMatrix_0;
    mat4x4 projectionMatrix_0;
    mat4x4 inverseProjection_0;
    vec4 cameraPos_WS_0;
    vec2 screenDimensions_0;
};

layout(binding = 0)
layout(std140) uniform _S3
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


#line 71 2
layout(binding = 3, set = 1)
layout(std140) uniform _S4
{
    uint _data;
} numPointLights_0;

#line 22
struct PointLight_0
{
    vec4 positionWS_0;
    vec4 colorRange_0;
};


#line 69
layout(std430, binding = 2, set = 1) readonly buffer _S5 {
    PointLight_0 _data[];
} pointLights_0;

#line 57 0
shared uint  tLightList_0[1024];


#line 70
void tAppendLight_0(uint lightIndex_0)
{
    uint index_0;
    ((index_0) = atomicAdd((tLightCount_0), (uint(1))));
    if(index_0 < uint(1024))
    {
        tLightList_0[index_0] = lightIndex_0;

#line 74
    }
    else
    {

#line 74
    }



    return;
}


#line 58 2
vec3 PointLight_getViewPos_0(PointLight_0 this_0)
{
    vec4 _S6 = (((this_0.positionWS_0) * (gViewParams_0._data.viewMatrix_0)));

#line 60
    return _S6.xyz;
}


#line 36
bool PointLight_insidePlane_0(PointLight_0 this_1, Plane_0 plane_0)
{

#line 36
    vec3 _S7 = plane_0.n_0;

    vec3 _S8 = PointLight_getViewPos_0(this_1);

#line 38
    float _S9 = dot(_S7, _S8.xyz);

#line 38
    return _S9 - plane_0.d_0 < - this_1.colorRange_0.w;
}


#line 53 0
shared uint  oLightList_0[1024];


#line 60
void oAppendLight_0(uint lightIndex_1)
{
    uint index_1;
    ((index_1) = atomicAdd((oLightCount_0), (uint(1))));
    if(index_1 < uint(1024))
    {
        oLightList_0[index_1] = lightIndex_1;

#line 64
    }
    else
    {

#line 64
    }



    return;
}


#line 26
layout(std430, binding = 4) buffer _S10 {
    uint _data[];
} oLightIndexCounter_0;

#line 52
shared uint oLightIndexStartOffset_0;


#line 36
layout(rg32ui)
layout(binding = 8)
uniform uimage2D oLightGrid_0;


#line 28
layout(std430, binding = 5) buffer _S11 {
    uint _data[];
} tLightIndexCounter_0;

#line 56
shared uint tLightIndexStartOffset_0;


#line 38
layout(rg32ui)
layout(binding = 9)
uniform uimage2D tLightGrid_0;


#line 31
layout(std430, binding = 6) buffer _S12 {
    uint _data[];
} oLightIndexList_0;

#line 33
layout(std430, binding = 7) buffer _S13 {
    uint _data[];
} tLightIndexList_0;

#line 4
struct ComputeShaderInput_0
{
    uvec3 groupID_0;
    uvec3 groupThreadID_0;
    uvec3 dispatchThreadID_0;
    uint groupIndex_0;
};


#line 82
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
    uint i_0;
    uint j_0;
    uint k_0;

#line 82
    ComputeShaderInput_0 _S14 = ComputeShaderInput_0(gl_WorkGroupID, gl_LocalInvocationID, gl_GlobalInvocationID, gl_LocalInvocationIndex);


    ivec3 _S15 = ivec3(ivec2(_S14.dispatchThreadID_0.xy), 0);

#line 85
    vec4 _S16 = (texelFetch((depthTextureVS_0), ((_S15)).xy, ((_S15)).z));

    uint uDepth_0 = floatBitsToUint(_S16.x);
    if(_S14.groupIndex_0 == uint(0))
    {
        uMinDepth_0 = uint(-1);
        uMaxDepth_0 = uint(0);
        oLightCount_0 = uint(0);
        tLightCount_0 = uint(0);
        Frustum_0 _S17 = ((frustums_0)._data[(_S14.groupID_0.x + _S14.groupID_0.y * DispatchParams_0._data.numThreadGroups_0.x)]);

#line 94
        groupFrustum_0 = _S17;

#line 88
    }
    else
    {

#line 88
    }

#line 97
    groupMemoryBarrier(), barrier();

    atomicMin((uMinDepth_0), (uDepth_0));
    atomicMax((uMaxDepth_0), (uDepth_0));

    groupMemoryBarrier(), barrier();

    float fMinDepth_0 = uintBitsToFloat(uMinDepth_0);
    float fMaxDepth_0 = uintBitsToFloat(uMaxDepth_0);

    vec4 _S18 = clipToView_0(vec4(float(0), float(0), fMinDepth_0, float(1)));

#line 107
    float minDepthVS_0 = _S18.z;
    vec4 _S19 = clipToView_0(vec4(float(0), float(0), fMaxDepth_0, float(1)));
    vec4 _S20 = clipToView_0(vec4(float(0), float(0), float(0), 1.00000000000000000000));

    Plane_0 _S21 = { vec3(float(0), float(0), float(-1)), - minDepthVS_0 };

#line 111
    uint _S22 = _S14.groupIndex_0;

    i_0 = _S22;
    for(;;)
    {

#line 113
        if(i_0 < numPointLights_0._data)
        {
        }
        else
        {
            break;
        }

#line 115
        PointLight_0 light_0 = ((pointLights_0)._data[(i_0)]);



        tAppendLight_0(i_0);
        bool _S23 = PointLight_insidePlane_0(light_0, _S21);

#line 120
        if(!_S23)
        {
            oAppendLight_0(i_0);

#line 120
        }
        else
        {

#line 120
        }

#line 113
        uint i_1 = i_0 + uint(32) * uint(32);

#line 113
        i_0 = i_1;
    }

#line 127
    groupMemoryBarrier(), barrier();

    if(_S14.groupIndex_0 == uint(0))
    {
        ((oLightIndexStartOffset_0) = atomicAdd((((oLightIndexCounter_0)._data[(uint(0))])), (oLightCount_0)));
        imageStore((oLightGrid_0), ivec2((_S14.groupID_0.xy)), uvec4(uvec2(oLightIndexStartOffset_0, oLightCount_0), uint(0), uint(0)));

        ((tLightIndexStartOffset_0) = atomicAdd((((tLightIndexCounter_0)._data[(uint(0))])), (tLightCount_0)));
        imageStore((tLightGrid_0), ivec2((_S14.groupID_0.xy)), uvec4(uvec2(tLightIndexStartOffset_0, tLightCount_0), uint(0), uint(0)));

#line 129
    }
    else
    {

#line 129
    }

#line 137
    groupMemoryBarrier(), barrier();

#line 137
    uint _S24 = _S14.groupIndex_0;

    j_0 = _S24;
    for(;;)
    {

#line 139
        if(j_0 < oLightCount_0)
        {
        }
        else
        {
            break;
        }

#line 141
        ((oLightIndexList_0)._data[(oLightIndexStartOffset_0 + j_0)]) = oLightList_0[j_0];

#line 139
        uint j_1 = j_0 + uint(32) * uint(32);

#line 139
        j_0 = j_1;
    }

#line 139
    uint _S25 = _S14.groupIndex_0;

#line 145
    k_0 = _S25;
    for(;;)
    {

#line 145
        if(k_0 < tLightCount_0)
        {
        }
        else
        {
            break;
        }

#line 147
        ((tLightIndexList_0)._data[(tLightIndexStartOffset_0 + k_0)]) = tLightList_0[k_0];

#line 145
        uint k_1 = k_0 + uint(32) * uint(32);

#line 145
        k_0 = k_1;
    }

#line 151
    return;
}

