#version 450
#extension GL_EXT_mesh_shader : require
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 1 0
struct InstanceData_0
{
    mat4x4 transformMatrix_0;
};


#line 45 1
layout(std430, binding = 0, set = 2) readonly buffer StructuredBuffer_InstanceData_t_0 {
    InstanceData_0 _data[];
} scene_instances_0;

#line 1 2
struct MeshletDescription_0
{
    uint vertexCount_0;
    uint primitiveCount_0;
    uint vertexOffset_0;
    uint primitiveOffset_0;
};


#line 46 1
layout(std430, binding = 0, set = 1) readonly buffer StructuredBuffer_MeshletDescription_t_0 {
    MeshletDescription_0 _data[];
} meshlets_meshletInfos_0;

#line 9 2
layout(std430, binding = 1, set = 1) readonly buffer StructuredBuffer_uint8_t_0 {
    uint8_t _data[];
} meshlets_primitiveIndices_0;

#line 9
layout(std430, binding = 2, set = 1) readonly buffer StructuredBuffer_uint_t_0 {
    uint _data[];
} meshlets_vertexIndices_0;

#line 35 3
layout(std430, binding = 0, set = 3) readonly buffer StructuredBuffer_float3_t_0 {
    vec3 _data[];
} vertexData_positions_0;

#line 35
layout(std430, binding = 1, set = 3) readonly buffer StructuredBuffer_float2_t_0 {
    vec2 _data[];
} vertexData_texCoords_0;

#line 35
layout(std430, binding = 2, set = 3) readonly buffer StructuredBuffer_float3_t_1 {
    vec3 _data[];
} vertexData_normals_0;

#line 35
layout(std430, binding = 3, set = 3) readonly buffer StructuredBuffer_float3_t_2 {
    vec3 _data[];
} vertexData_tangents_0;

#line 35
layout(std430, binding = 4, set = 3) readonly buffer StructuredBuffer_float3_t_3 {
    vec3 _data[];
} vertexData_biTangents_0;

#line 5 4
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

#line 1267 5
out gl_MeshPerVertexEXT
{
    vec4 gl_Position;
} gl_MeshVerticesEXT[64];


#line 24 1
shared uint gs_numVertices_0;


#line 5 3
struct StaticMeshVertexAttributes_0
{
    vec4 clipPosition_0;
    vec3 worldPosition_0;
    vec2 texCoords_0;
    vec3 normal_0;
    vec3 tangent_0;
    vec3 biTangent_0;
};


#line 22 1
shared StaticMeshVertexAttributes_0  gs_vertices_0[64];


shared uint gs_numPrimitives_0;


#line 23
shared uvec3  gs_indices_0[126];


#line 7910 6
layout(location = 0)
out vec3  _S2[64];


#line 7910
layout(location = 1)
out vec2  _S3[64];


#line 7910
layout(location = 2)
out vec3  _S4[64];


#line 7910
layout(location = 3)
out vec3  _S5[64];


#line 7910
layout(location = 4)
out vec3  _S6[64];


#line 7910
out uvec3  gl_PrimitiveTriangleIndicesEXT[126];


#line 900 7
StaticMeshVertexAttributes_0 StaticMeshVertexData_getAttributes_0(uint _S7, InstanceData_0 _S8)
{

#line 41 3
    vec4 worldPos_0 = (((vec4(vertexData_positions_0._data[_S7], 1.0)) * (_S8.transformMatrix_0)));

#line 39
    StaticMeshVertexAttributes_0 attr_0;

#line 44
    attr_0.clipPosition_0 = ((((((worldPos_0) * (viewParams_0.viewMatrix_0)))) * (viewParams_0.projectionMatrix_0)));
    attr_0.worldPosition_0 = worldPos_0.xyz;
    attr_0.texCoords_0 = vertexData_texCoords_0._data[_S7];
    attr_0.normal_0 = vertexData_normals_0._data[_S7];
    attr_0.tangent_0 = vertexData_tangents_0._data[_S7];
    attr_0.biTangent_0 = vertexData_biTangents_0._data[_S7];
    return attr_0;
}


#line 39 1
layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;
layout(max_vertices = 64) out;
layout(max_primitives = 126) out;
layout(triangles) out;
void main()
{
    InstanceData_0 _S9 = scene_instances_0._data[gl_LocalInvocationIndex];
    MeshletDescription_0 m_0 = meshlets_meshletInfos_0._data[gl_WorkGroupID.x];

#line 51
    uint _S10 = m_0.vertexCount_0 - 1U;

#line 63
    uint _S11 = m_0.primitiveCount_0 - 1U;

#line 82
    uint v_0 = min(gl_LocalInvocationIndex, _S10);



    uint v_1 = gl_LocalInvocationIndex + 32U;
    uint v_2 = min(v_1, _S10);

#line 92
    uint p_0 = min(gl_LocalInvocationIndex, _S11);

#line 98
    uint p_1 = min(v_1, _S11);

#line 104
    uint p_2 = min(gl_LocalInvocationIndex + 64U, _S11);

#line 110
    uint p_3 = min(gl_LocalInvocationIndex + 96U, _S11);

#line 110
    uint loop_0 = 0U;

#line 110
    for(;;)
    {

#line 51
        uint v_3 = min(gl_LocalInvocationIndex + loop_0 * 32U, _S10);
        atomicMax((gs_numVertices_0), (v_3 + 1U));


        gs_vertices_0[v_3] = StaticMeshVertexData_getAttributes_0(uint(int(meshlets_vertexIndices_0._data[m_0.vertexOffset_0 + v_3])), _S9);

#line 48
        uint loop_1 = loop_0 + 1U;

#line 48
        if(loop_1 < 2U)
        {
        }
        else
        {

#line 48
            break;
        }

#line 48
        loop_0 = loop_1;

#line 48
    }

#line 48
    loop_0 = 0U;

#line 48
    for(;;)
    {

#line 63
        uint p_4 = min(gl_LocalInvocationIndex + loop_0 * 32U, _S11);
        atomicMax((gs_numPrimitives_0), (p_4 + 1U));

        uint _S12 = p_4 * 3U;

#line 66
        uint _S13 = m_0.primitiveOffset_0 + _S12;



        uint idx1_0 = meshlets_vertexIndices_0._data[m_0.vertexOffset_0 + uint(meshlets_primitiveIndices_0._data[_S13 + 1U])];
        uint idx2_0 = meshlets_vertexIndices_0._data[m_0.vertexOffset_0 + uint(meshlets_primitiveIndices_0._data[_S13 + 2U])];
        gs_indices_0[_S12] = uvec3(meshlets_vertexIndices_0._data[m_0.vertexOffset_0 + uint(meshlets_primitiveIndices_0._data[_S13])]);
        gs_indices_0[_S12 + 1U] = uvec3(idx1_0);
        gs_indices_0[_S12 + 2U] = uvec3(idx2_0);

#line 60
        uint loop_2 = loop_0 + 1U;

#line 60
        if(loop_2 < 4U)
        {
        }
        else
        {

#line 60
            break;
        }

#line 60
        loop_0 = loop_2;

#line 60
    }

#line 77
    barrier();
    SetMeshOutputsEXT(gs_numVertices_0, gs_numPrimitives_0);
    barrier();



    StaticMeshVertexAttributes_0 _S14 = gs_vertices_0[v_0];

#line 83
    gl_MeshVerticesEXT[v_0].gl_Position = gs_vertices_0[v_0].clipPosition_0;

#line 83
    _S2[v_0] = _S14.worldPosition_0;

#line 83
    _S3[v_0] = _S14.texCoords_0;

#line 83
    _S4[v_0] = _S14.normal_0;

#line 83
    _S5[v_0] = _S14.tangent_0;

#line 83
    _S6[v_0] = _S14.biTangent_0;

#line 88
    StaticMeshVertexAttributes_0 _S15 = gs_vertices_0[v_2];

#line 88
    gl_MeshVerticesEXT[v_2].gl_Position = gs_vertices_0[v_2].clipPosition_0;

#line 88
    _S2[v_2] = _S15.worldPosition_0;

#line 88
    _S3[v_2] = _S15.texCoords_0;

#line 88
    _S4[v_2] = _S15.normal_0;

#line 88
    _S5[v_2] = _S15.tangent_0;

#line 88
    _S6[v_2] = _S15.biTangent_0;

#line 93
    gl_PrimitiveTriangleIndicesEXT[p_0] = gs_indices_0[p_0];

#line 99
    gl_PrimitiveTriangleIndicesEXT[p_1] = gs_indices_0[p_1];

#line 105
    gl_PrimitiveTriangleIndicesEXT[p_2] = gs_indices_0[p_2];

#line 111
    gl_PrimitiveTriangleIndicesEXT[p_3] = gs_indices_0[p_3];

    return;
}

