#version 450
#extension GL_EXT_samplerless_texture_functions : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 11 0
struct CullingParams_0
{
    uvec3 numThreadGroups_0;
    uint pad0_0;
    uvec3 numThreads_0;
    uint pad1_0;
};


#line 35 1
layout(binding = 0, set = 1)
layout(std140) uniform _S1
{
    uvec3 numThreadGroups_0;
    uint pad0_0;
    uvec3 numThreads_0;
    uint pad1_0;
}gCullingParams_0;

#line 2113 2
layout(binding = 1, set = 1)
uniform texture2D gCullingParams_depthTextureVS_0;


#line 11 0
layout(std430, binding = 2, set = 1) buffer StructuredBuffer_uint_t_0 {
    uint _data[];
} gCullingParams_oLightIndexCounter_0;

#line 11
layout(std430, binding = 3, set = 1) buffer StructuredBuffer_uint_t_1 {
    uint _data[];
} gCullingParams_tLightIndexCounter_0;

#line 11
layout(std430, binding = 4, set = 1) buffer StructuredBuffer_uint_t_2 {
    uint _data[];
} gCullingParams_oLightIndexList_0;

#line 11
layout(std430, binding = 5, set = 1) buffer StructuredBuffer_uint_t_3 {
    uint _data[];
} gCullingParams_tLightIndexList_0;

#line 1572 2
layout(rg32ui)
layout(binding = 6, set = 1)
uniform uimage2D gCullingParams_oLightGrid_0;


#line 1572
layout(rg32ui)
layout(binding = 7, set = 1)
uniform uimage2D gCullingParams_tLightGrid_0;


#line 26 1
struct Plane_0
{
    vec3 n_0;
    float d_0;
};

struct Frustum_0
{
    Plane_0  sides_0[4];
    Plane_0 basePlane_0;
};


#line 11 0
layout(std430, binding = 8, set = 1) readonly buffer StructuredBuffer_Frustum_t_0 {
    Frustum_0 _data[];
} gCullingParams_frustums_0;

#line 64 3
struct LightEnv_0
{
    uint numDirectionalLights_0;
    uint numPointLights_0;
};


#line 25
layout(binding = 0, set = 2)
layout(std140) uniform _S2
{
    uint numDirectionalLights_0;
    uint numPointLights_0;
}gLightEnv_0;

#line 22
struct PointLight_0
{
    vec4 position_WS_0;
    vec4 colorRange_0;
};


#line 64
layout(std430, binding = 2, set = 2) readonly buffer StructuredBuffer_PointLight_t_0 {
    PointLight_0 _data[];
} gLightEnv_pointLights_0;

#line 5 1
struct ViewParameter_0
{
    mat4x4 viewMatrix_0;
    mat4x4 projectionMatrix_0;
    vec4 cameraPos_WS_0;
    vec2 screenDimensions_0;
};

layout(binding = 0)
layout(std140) uniform _S3
{
    mat4x4 viewMatrix_0;
    mat4x4 projectionMatrix_0;
    vec4 cameraPos_WS_0;
    vec2 screenDimensions_0;
}viewParams_0;

#line 39 0
shared uint uMinDepth_0;


#line 40
shared uint uMaxDepth_0;



shared uint oLightCount_0;



shared uint tLightCount_0;


#line 42
shared Frustum_0 groupFrustum_0;


#line 50
shared uint  tLightList_0[1024];


#line 63
void tAppendLight_0(uint lightIndex_0)
{
    uint index_0;
    ((index_0) = atomicAdd((tLightCount_0), (1U)));
    if(index_0 < 1024U)
    {
        tLightList_0[index_0] = lightIndex_0;

#line 67
    }



    return;
}


#line 58 3
vec3 PointLight_getViewPos_0(PointLight_0 this_0)
{
    return (((this_0.position_WS_0) * (viewParams_0.viewMatrix_0))).xyz;
}


#line 36
bool PointLight_insidePlane_0(PointLight_0 this_1, Plane_0 plane_0)
{
    return dot(plane_0.n_0, PointLight_getViewPos_0(this_1).xyz) - plane_0.d_0 < - this_1.colorRange_0.w;
}


#line 46 0
shared uint  oLightList_0[1024];


#line 53
void oAppendLight_0(uint lightIndex_1)
{
    uint index_1;
    ((index_1) = atomicAdd((oLightCount_0), (1U)));
    if(index_1 < 1024U)
    {
        oLightList_0[index_1] = lightIndex_1;

#line 57
    }



    return;
}


#line 45
shared uint oLightIndexStartOffset_0;



shared uint tLightIndexStartOffset_0;


#line 75
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
    ivec3 _S4 = ivec3(ivec2(gl_GlobalInvocationID.xy), 0);

    uint uDepth_0 = floatBitsToUint((texelFetch((gCullingParams_depthTextureVS_0), ((_S4)).xy, ((_S4)).z)).x);
    bool _S5 = gl_LocalInvocationIndex == 0U;

#line 81
    if(_S5)
    {
        uMinDepth_0 = 4294967295U;
        uMaxDepth_0 = 0U;
        oLightCount_0 = 0U;
        tLightCount_0 = 0U;
        groupFrustum_0 = gCullingParams_frustums_0._data[gl_WorkGroupID.x + gl_WorkGroupID.y * gCullingParams_0.numThreadGroups_0.x];

#line 81
    }

#line 90
    barrier();

    atomicMin((uMinDepth_0), (uDepth_0));
    atomicMax((uMaxDepth_0), (uDepth_0));

    barrier();

#line 104
    Plane_0 _S6 = { vec3(0.0, 0.0, -1.0), - uintBitsToFloat(uMinDepth_0) };

#line 125
    uvec2 _S7 = gl_WorkGroupID.xy;

#line 125
    uint i_0 = gl_LocalInvocationIndex;

#line 125
    for(;;)
    {

#line 106
        if(i_0 < gLightEnv_0.numPointLights_0)
        {
        }
        else
        {

#line 106
            break;
        }
        PointLight_0 light_0 = gLightEnv_pointLights_0._data[i_0];



        tAppendLight_0(i_0);
        if(!PointLight_insidePlane_0(light_0, _S6))
        {
            oAppendLight_0(i_0);

#line 113
        }

#line 106
        i_0 = i_0 + 1024U;

#line 106
    }

#line 120
    barrier();

    if(_S5)
    {
        ((oLightIndexStartOffset_0) = atomicAdd((gCullingParams_oLightIndexCounter_0._data[0U]), (oLightCount_0)));
        imageStore((gCullingParams_oLightGrid_0), ivec2((_S7)), uvec4(uvec2(oLightIndexStartOffset_0, oLightCount_0), uint(0), uint(0)));

        ((tLightIndexStartOffset_0) = atomicAdd((gCullingParams_tLightIndexCounter_0._data[0U]), (tLightCount_0)));
        imageStore((gCullingParams_tLightGrid_0), ivec2((_S7)), uvec4(uvec2(tLightIndexStartOffset_0, tLightCount_0), uint(0), uint(0)));

#line 122
    }

#line 130
    barrier();

#line 130
    uint k_0;

#line 130
    if(gl_LocalInvocationIndex < oLightCount_0)
    {

#line 130
        k_0 = gl_LocalInvocationIndex;

#line 130
        for(;;)
        {


            gCullingParams_oLightIndexList_0._data[oLightIndexStartOffset_0 + k_0] = oLightList_0[k_0];

#line 132
            uint j_0 = k_0 + 1024U;

#line 132
            if(j_0 < oLightCount_0)
            {
            }
            else
            {

#line 132
                break;
            }

#line 132
            k_0 = j_0;

#line 132
        }

#line 132
    }

#line 132
    if(gl_LocalInvocationIndex < tLightCount_0)
    {

#line 132
        k_0 = gl_LocalInvocationIndex;

#line 132
        for(;;)
        {

#line 140
            gCullingParams_tLightIndexList_0._data[tLightIndexStartOffset_0 + k_0] = tLightList_0[k_0];

#line 138
            uint k_1 = k_0 + 1024U;

#line 138
            if(k_1 < tLightCount_0)
            {
            }
            else
            {

#line 138
                break;
            }

#line 138
            k_0 = k_1;

#line 138
        }

#line 138
    }

#line 144
    return;
}

