#version 450
#extension GL_EXT_mesh_shader : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 3 0
struct InstanceData_0
{
    mat4x4 transformMatrix_0;
};


#line 25 1
layout(std430, binding = 0, set = 2) readonly buffer StructuredBuffer_InstanceData_t_0 {
    InstanceData_0 _data[];
} scene_instances_0;

#line 20 2
struct MeshData_0
{
    uint numMeshlets_0;
    uint meshletOffset_0;
};


#line 8 0
layout(std430, binding = 1, set = 2) readonly buffer StructuredBuffer_MeshData_t_0 {
    MeshData_0 _data[];
} scene_meshData_0;

#line 5 3
struct ViewParameter_0
{
    mat4x4 viewMatrix_0;
    mat4x4 projectionMatrix_0;
    vec4 cameraPos_WS_0;
    vec2 screenDimensions_0;
};

layout(binding = 0)
layout(std140) uniform _S1
{
    mat4x4 viewMatrix_0;
    mat4x4 projectionMatrix_0;
    vec4 cameraPos_WS_0;
    vec2 screenDimensions_0;
}viewParams_0;

#line 14 1
shared uint head_0;


#line 15
shared mat4x4 localToClip_0;


#line 26 3
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


#line 16 1
shared Frustum_0 viewFrustum_0;


#line 7
struct MeshPayload_0
{
    uint  instanceId_0[512];
    uint  meshletId_0[512];
};

taskPayloadSharedEXT MeshPayload_0 p_0;


#line 21
layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main()
{

#line 21
    uint _S2 = gl_WorkGroupID.x;



    InstanceData_0 instance_0 = scene_instances_0._data[_S2];
    if(gl_LocalInvocationIndex == 0U)
    {
        head_0 = 0U;
        localToClip_0 = ((((((instance_0.transformMatrix_0) * (viewParams_0.viewMatrix_0)))) * (viewParams_0.projectionMatrix_0)));

        viewFrustum_0.sides_0[0].n_0 = vec3(1.0, 0.0, 0.0);
        viewFrustum_0.sides_0[0].d_0 = -1.0;

        viewFrustum_0.sides_0[1].n_0 = vec3(-1.0, 0.0, 0.0);
        viewFrustum_0.sides_0[1].d_0 = 1.0;

        viewFrustum_0.sides_0[2].n_0 = vec3(0.0, -1.0, 0.0);
        viewFrustum_0.sides_0[2].d_0 = 1.0;

        viewFrustum_0.sides_0[1].n_0 = vec3(0.0, 1.0, 0.0);
        viewFrustum_0.sides_0[1].d_0 = -1.0;

        viewFrustum_0.basePlane_0.n_0 = vec3(0.0, 0.0, 1.0);
        viewFrustum_0.basePlane_0.d_0 = 0.0;

#line 26
    }

#line 46
    barrier();
    MeshData_0 _S3 = scene_meshData_0._data[_S2];

#line 47
    if(gl_LocalInvocationIndex < 512U)
    {

#line 47
        uint i_0 = gl_LocalInvocationIndex;

#line 47
        for(;;)
        {

            uint m_0 = _S3.meshletOffset_0 + min(_S3.numMeshlets_0, i_0);



            uint index_0;
            ((index_0) = atomicAdd((head_0), (1U)));
            p_0.meshletId_0[index_0] = m_0;
            p_0.instanceId_0[index_0] = _S2;

#line 48
            uint i_1 = i_0 + 128U;

#line 48
            if(i_1 < 512U)
            {
            }
            else
            {

#line 48
                break;
            }

#line 48
            i_0 = i_1;

#line 48
        }

#line 48
    }

#line 60
    barrier();
    EmitMeshTasksEXT((head_0), (1U), (1U));
    return;
}

