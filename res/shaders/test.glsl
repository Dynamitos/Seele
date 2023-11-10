#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 5 0
struct InstanceData_0
{
    mat4x4 transformMatrix_0;
};


#line 22
layout(std430, binding = 0) readonly buffer StructuredBuffer_InstanceData_t_0 {
    InstanceData_0 _data[];
} pScene_instances_0;

#line 5 1
layout(std430, binding = 0, set = 2) readonly buffer StructuredBuffer_float3_t_0 {
    vec3 _data[];
} pVertexData_positions_0;

#line 5
layout(std430, binding = 1, set = 2) readonly buffer StructuredBuffer_float2_t_0 {
    vec2 _data[];
} pVertexData_texCoords_0;

#line 5
layout(std430, binding = 2, set = 2) readonly buffer StructuredBuffer_float3_t_1 {
    vec3 _data[];
} pVertexData_normals_0;

#line 5
layout(std430, binding = 3, set = 2) readonly buffer StructuredBuffer_float3_t_2 {
    vec3 _data[];
} pVertexData_tangents_0;

#line 5
layout(std430, binding = 4, set = 2) readonly buffer StructuredBuffer_float3_t_3 {
    vec3 _data[];
} pVertexData_biTangents_0;

#line 5 2
struct ViewParameter_0
{
    mat4x4 viewMatrix_0;
    mat4x4 projectionMatrix_0;
    vec4 cameraPos_WS_0;
    vec2 screenDimensions_0;
};


#line 12
layout(binding = 0, set = 1)
layout(std140) uniform _S1
{
    mat4x4 viewMatrix_0;
    mat4x4 projectionMatrix_0;
    vec4 cameraPos_WS_0;
    vec2 screenDimensions_0;
}pViewParams_0;

#line 4206 3
layout(location = 0)
out vec3 _S2;


#line 4206
layout(location = 1)
out vec3 _S3;


#line 4206
layout(location = 2)
out vec2 _S4;


#line 4206
layout(location = 3)
out vec3 _S5;


#line 4206
layout(location = 4)
out vec3 _S6;


#line 4206
layout(location = 5)
out vec3 _S7;


#line 4206
layout(location = 6)
out vec3 _S8;


#line 1 4
struct MaterialParameter_0
{
    vec3 position_TS_0;
    vec3 worldPosition_0;
    vec2 texCoords_0;
    vec3 normal_0;
    vec3 tangent_0;
    vec3 biTangent_0;
    vec3 viewDir_TS_0;
};

struct VertexAttributes_0
{
    MaterialParameter_0 parameter_0;
    vec4 clipPosition_0;
};


#line 9 1
VertexAttributes_0 StaticMeshVertexData_getAttributes_0(uint _S9, mat4x4 _S10)
{

    vec4 worldPos_0 = (((vec4(pVertexData_positions_0._data[_S9], 1.0)) * (_S10)));

    vec4 clipPos_0 = ((((((worldPos_0) * (pViewParams_0.viewMatrix_0)))) * (pViewParams_0.projectionMatrix_0)));

#line 10
    MaterialParameter_0 params_0;

#line 15
    params_0.worldPosition_0 = worldPos_0.xyz;
    params_0.texCoords_0 = pVertexData_texCoords_0._data[_S9];
    params_0.normal_0 = pVertexData_normals_0._data[_S9];
    params_0.tangent_0 = pVertexData_tangents_0._data[_S9];
    params_0.biTangent_0 = pVertexData_biTangents_0._data[_S9];

#line 9
    VertexAttributes_0 attributes_0;

#line 20
    attributes_0.parameter_0 = params_0;
    attributes_0.clipPosition_0 = clipPos_0;
    return attributes_0;
}


#line 18 0
void main()
{

#line 18
    VertexAttributes_0 _S11 = StaticMeshVertexData_getAttributes_0(uint(gl_VertexIndex), pScene_instances_0._data[uint(gl_InstanceIndex)].transformMatrix_0);

#line 18
    _S2 = _S11.parameter_0.position_TS_0;

#line 18
    _S3 = _S11.parameter_0.worldPosition_0;

#line 18
    _S4 = _S11.parameter_0.texCoords_0;

#line 18
    _S5 = _S11.parameter_0.normal_0;

#line 18
    _S6 = _S11.parameter_0.tangent_0;

#line 18
    _S7 = _S11.parameter_0.biTangent_0;

#line 18
    _S8 = _S11.parameter_0.viewDir_TS_0;

#line 18
    gl_Position = _S11.clipPosition_0;

#line 18
    return;
}

