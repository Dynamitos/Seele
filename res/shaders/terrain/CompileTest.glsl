#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 8 0
struct GeometryCB_std140_0
{
    uint totalNumElements_0;
    uint baseDepth_0;
    uint totalNumVertices_0;
};


#line 8
layout(binding = 0, set = 1)
layout(std140) uniform block_GeometryCB_std140_0
{
    uint totalNumElements_0;
    uint baseDepth_0;
    uint totalNumVertices_0;
}pParams_geometry_0;

#line 1334 1
struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    vec4  data_0[4];
};


#line 27 0
struct UpdateCB_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 viewProjectionMatrix_0;
    float triangleSize_0;
    uint maxSubdivisionDepth_0;
    float fov_0;
    float farPlaneDistance_0;
};


#line 27
layout(binding = 1, set = 1)
layout(std140) uniform block_UpdateCB_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 viewProjectionMatrix_0;
    float triangleSize_0;
    uint maxSubdivisionDepth_0;
    float fov_0;
    float farPlaneDistance_0;
}pParams_update_0;

#line 40
layout(std430, binding = 2, set = 1) buffer StructuredBuffer_vectorx3Cfloatx2C4x3E_t_0 {
    vec4 _data[];
} pParams_currentVertexBuffer_0;

#line 40
layout(std430, binding = 3, set = 1) readonly buffer StructuredBuffer_uint_t_0 {
    uint _data[];
} pParams_indexedBisectorBuffer_0;

#line 40
layout(std430, binding = 4, set = 1) buffer StructuredBuffer_uint_t_1 {
    uint _data[];
} pParams_indirectDrawBuffer_0;

#line 40
layout(std430, binding = 5, set = 1) buffer StructuredBuffer_uint64_t_0 {
    uint64_t _data[];
} pParams_heapIDBuffer_0;

#line 20 2
struct _Array_std430_uint3_0
{
    uint  data_1[3];
};


#line 35
struct BisectorData_std430_0
{
    _Array_std430_uint3_0 indices_0;
    uint subdivisionPattern_0;
    uint problematicNeighbor_0;
    uint bisectorState_0;
    uint flags_0;
    uint propagationID_0;
};


#line 35
layout(std430, binding = 6, set = 1) buffer StructuredBuffer_BisectorData_std430_t_0 {
    BisectorData_std430_0 _data[];
} pParams_bisectorDataBuffer_0;

#line 35
layout(std430, binding = 7, set = 1) buffer StructuredBuffer_uint_t_2 {
    uint _data[];
} pParams_classificationBuffer_0;

#line 102 3
struct Plane_std140_0
{
    vec3 n_0;
    float d_0;
};


#line 102
struct _Array_std140_Plane4_0
{
    Plane_std140_0  data_2[4];
};


#line 102
struct Frustum_std140_0
{
    _Array_std140_Plane4_0 sides_0;
};


#line 19
struct ViewParameter_std140_0
{
    Frustum_std140_0 viewFrustum_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 viewMatrix_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 inverseViewMatrix_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 projectionMatrix_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 inverseProjection_0;
    vec4 cameraPosition_WS_0;
    vec4 cameraForward_WS_0;
    vec2 screenDimensions_0;
    vec2 invScreenDimensions_0;
    uint frameIndex_0;
    float time_0;
};


#line 22
layout(binding = 0)
layout(std140) uniform block_ViewParameter_std140_0
{
    Frustum_std140_0 viewFrustum_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 viewMatrix_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 inverseViewMatrix_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 projectionMatrix_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 inverseProjection_0;
    vec4 cameraPosition_WS_0;
    vec4 cameraForward_WS_0;
    vec2 screenDimensions_0;
    vec2 invScreenDimensions_0;
    uint frameIndex_0;
    float time_0;
}pViewParams_0;

#line 22
mat4x4 unpackStorage_0(_MatrixStorage_float4x4_ColMajorstd140_0 _S1)
{

#line 22
    return mat4x4(_S1.data_0[0][0], _S1.data_0[1][0], _S1.data_0[2][0], _S1.data_0[3][0], _S1.data_0[0][1], _S1.data_0[1][1], _S1.data_0[2][1], _S1.data_0[3][1], _S1.data_0[0][2], _S1.data_0[1][2], _S1.data_0[2][2], _S1.data_0[3][2], _S1.data_0[0][3], _S1.data_0[1][3], _S1.data_0[2][3], _S1.data_0[3][3]);
}


#line 99
struct Plane_0
{
    vec3 n_0;
    float d_0;
};


#line 109
Plane_0 unpackStorage_1(Plane_std140_0 _S2)
{

#line 109
    Plane_0 _S3 = { _S2.n_0, _S2.d_0 };

#line 109
    return _S3;
}


#line 109
void unpackStorage_2(_Array_std140_Plane4_0 _S4, out Plane_0  _S5[4])
{

#line 109
    Plane_0 _S6 = unpackStorage_1(_S4.data_2[1]);

#line 109
    Plane_0 _S7 = unpackStorage_1(_S4.data_2[2]);

#line 109
    Plane_0 _S8 = unpackStorage_1(_S4.data_2[3]);

#line 109
    _S5[0] = unpackStorage_1(_S4.data_2[0]);

#line 109
    _S5[1] = _S6;

#line 109
    _S5[2] = _S7;

#line 109
    _S5[3] = _S8;

#line 109
    return;
}


#line 17 2
_Array_std430_uint3_0 packStorage_0(uint  _S9[3])
{

#line 17
    uint  _S10[3] = { _S9[0], _S9[1], _S9[2] };

#line 17
    _Array_std430_uint3_0 _S11 = { _S10 };

#line 17
    return _S11;
}


#line 17
void unpackStorage_3(_Array_std430_uint3_0 _S12, out uint  _S13[3])
{

#line 17
    _S13[0] = _S12.data_1[0];

#line 17
    _S13[1] = _S12.data_1[1];

#line 17
    _S13[2] = _S12.data_1[2];

#line 17
    return;
}


#line 17
struct BisectorData_0
{
    uint  indices_0[3];
    uint subdivisionPattern_0;
    uint problematicNeighbor_0;
    uint bisectorState_0;
    uint flags_0;
    uint propagationID_0;
};


#line 17
BisectorData_0 unpackStorage_4(BisectorData_std430_0 _S14)
{

#line 17
    uint  _S15[3];

#line 17
    unpackStorage_3(_S14.indices_0, _S15);

#line 17
    BisectorData_0 _S16 = { _S15, _S14.subdivisionPattern_0, _S14.problematicNeighbor_0, _S14.bisectorState_0, _S14.flags_0, _S14.propagationID_0 };

#line 17
    return _S16;
}


#line 17
BisectorData_std430_0 packStorage_1(BisectorData_0 _S17)
{

#line 17
    BisectorData_std430_0 _S18 = { packStorage_0(_S17.indices_0), _S17.subdivisionPattern_0, _S17.problematicNeighbor_0, _S17.bisectorState_0, _S17.flags_0, _S17.propagationID_0 };

#line 17
    return _S18;
}


#line 38
uint HeapIDDepth_0(uint64_t x_0)
{

#line 38
    uint64_t _S19 = x_0;

#line 38
    uint depth_0 = 0U;


    for(;;)
    {

#line 41
        if(_S19 > 0UL)
        {
        }
        else
        {

#line 41
            break;
        }

#line 42
        uint depth_1 = depth_0 + 1U;

#line 42
        _S19 = _S19 >> 1U;

#line 42
        depth_0 = depth_1;

#line 41
    }



    return depth_0;
}


#line 109 3
struct Frustum_0
{
    Plane_0  sides_0[4];
};


#line 109
Frustum_0 unpackStorage_5(Frustum_std140_0 _S20)
{

#line 109
    Plane_0  _S21[4];

#line 109
    unpackStorage_2(_S20.sides_0, _S21);

#line 109
    Frustum_0 _S22 = { _S21 };

#line 109
    return _S22;
}


#line 26 4
bool FrustumAABBIntersect_0(Frustum_0 frustum_0, vec3 aabbMin_0, vec3 aabbMax_0)
{
    vec3 _S23 = (aabbMax_0 + aabbMin_0) * 0.5;
    vec3 _S24 = (aabbMax_0 - aabbMin_0) * 0.5;

#line 29
    int i_0 = 0;
    for(;;)
    {

#line 30
        if(i_0 < 4)
        {
        }
        else
        {

#line 30
            break;
        }

#line 37
        if(dot(_S23 + _S24 * vec3((ivec3(sign((frustum_0.sides_0[i_0].n_0))))), frustum_0.sides_0[i_0].n_0) + frustum_0.sides_0[i_0].d_0 < 0.0)
        {

#line 38
            return false;
        }

#line 30
        i_0 = i_0 + 1;

#line 30
    }

#line 40
    return true;
}


#line 7
struct BisectorGeometry_0
{
    vec3  p_0[4];
};


#line 43
int ClassifyBisector_0(BisectorGeometry_0 tri_0, uint depth_2)
{



    vec3 viewDir_0 = normalize(- ((tri_0.p_0[0] + tri_0.p_0[1] + tri_0.p_0[2]) / 3.0));

    float VdotN_0 = dot(viewDir_0, normalize(cross(tri_0.p_0[2] - tri_0.p_0[1], tri_0.p_0[0] - tri_0.p_0[1])));

#line 50
    bool _S25;


    if(dot(viewDir_0, pViewParams_0.cameraForward_WS_0.xyz) < 0.0)
    {

#line 53
        _S25 = VdotN_0 < -0.00100000004749745;

#line 53
    }
    else
    {

#line 53
        _S25 = false;

#line 53
    }

#line 53
    if(_S25)
    {

#line 54
        return -3;
    }

    float _S26 = tri_0.p_0[0].x;

#line 57
    float _S27 = tri_0.p_0[1].x;

#line 57
    float _S28 = tri_0.p_0[2].x;

#line 57
    float _S29 = tri_0.p_0[0].y;

#line 57
    float _S30 = tri_0.p_0[1].y;

#line 57
    float _S31 = tri_0.p_0[2].y;

#line 57
    float _S32 = tri_0.p_0[0].z;

#line 57
    float _S33 = tri_0.p_0[1].z;

#line 57
    float _S34 = tri_0.p_0[2].z;



    if(!FrustumAABBIntersect_0(unpackStorage_5(pViewParams_0.viewFrustum_0), vec3(min(min(_S26, _S27), _S28), min(min(_S29, _S30), _S31), min(min(_S32, _S33), _S34)), vec3(max(max(_S26, _S27), _S28), max(max(_S29, _S30), _S31), max(max(_S32, _S33), _S34))))
    {

#line 62
        return -2;
    }

#line 62
    mat4x4 _S35 = unpackStorage_0(pParams_update_0.viewProjectionMatrix_0);



    vec4 _S36 = (((vec4(tri_0.p_0[0], 1.0)) * (_S35)));

#line 66
    vec4 p0P_0 = _S36;
    p0P_0.xy = _S36.xy / _S36.w;
    p0P_0.xy = p0P_0.xy * 0.5 + 0.5;

    vec4 _S37 = (((vec4(tri_0.p_0[1], 1.0)) * (_S35)));

#line 70
    vec4 p1P_0 = _S37;
    p1P_0.xy = _S37.xy / _S37.w;
    p1P_0.xy = p1P_0.xy * 0.5 + 0.5;

    vec4 _S38 = (((vec4(tri_0.p_0[2], 1.0)) * (_S35)));

#line 74
    vec4 p2P_0 = _S38;
    p2P_0.xy = _S38.xy / _S38.w;
    p2P_0.xy = p2P_0.xy * 0.5 + 0.5;

#line 85
    float areaOverestimation_0 = mix(2.0, 1.0, pow(VdotN_0, 0.20000000298023224));
    float area_0 = 0.5 * abs(p0P_0.x * (p2P_0.y - p1P_0.y) + p1P_0.x * (p0P_0.y - p2P_0.y) + p2P_0.x * (p1P_0.y - p0P_0.y)) * (pViewParams_0.screenDimensions_0.x * pViewParams_0.screenDimensions_0.y) * areaOverestimation_0;


    if(pParams_update_0.triangleSize_0 < area_0)
    {

#line 89
        _S25 = depth_2 < pParams_update_0.maxSubdivisionDepth_0;

#line 89
    }
    else
    {

#line 89
        _S25 = false;

#line 89
    }

#line 89
    if(_S25)
    {

        return 1;
    }
    else
    {

#line 94
        if(pParams_update_0.triangleSize_0 * 0.5 > area_0)
        {

#line 94
            _S25 = true;

#line 94
        }
        else
        {

#line 94
            _S25 = depth_2 > pParams_update_0.maxSubdivisionDepth_0;

#line 94
        }

#line 94
        if(_S25)
        {

            vec4 _S39 = (((vec4(tri_0.p_0[3], 1.0)) * (_S35)));

#line 97
            vec4 p3P_0 = _S39;
            p3P_0.xy = _S39.xy / _S39.w;
            p3P_0.xy = p3P_0.xy * 0.5 + 0.5;

#line 107
            if(pParams_update_0.triangleSize_0 >= 0.5 * abs(p0P_0.x * (p2P_0.y - p3P_0.y) + p3P_0.x * (p0P_0.y - p2P_0.y) + p2P_0.x * (p3P_0.y - p0P_0.y)) * (pViewParams_0.screenDimensions_0.x * pViewParams_0.screenDimensions_0.y) * areaOverestimation_0)
            {

#line 107
                _S25 = true;

#line 107
            }
            else
            {

#line 107
                _S25 = depth_2 > pParams_update_0.maxSubdivisionDepth_0;

#line 107
            }

#line 107
            int _S40;

#line 107
            if(_S25)
            {

#line 107
                _S40 = -1;

#line 107
            }
            else
            {

#line 107
                _S40 = 0;

#line 107
            }

#line 107
            return _S40;
        }

#line 89
    }

#line 109
    return 0;
}


#line 140
void ClassifyElement_0(uint currentID_0, BisectorGeometry_0 bis_0, uint totalNumElements_1, uint baseDepth_1)
{

    uint64_t heapID_0 = pParams_heapIDBuffer_0._data[uint(currentID_0)];
    uint depth_3 = HeapIDDepth_0(pParams_heapIDBuffer_0._data[uint(currentID_0)]);
    BisectorData_0 cbisectorData_0 = unpackStorage_4(pParams_bisectorDataBuffer_0._data[uint(currentID_0)]);


    cbisectorData_0.subdivisionPattern_0 = 0U;
    cbisectorData_0.bisectorState_0 = 0U;
    cbisectorData_0.problematicNeighbor_0 = 4294967295U;
    cbisectorData_0.flags_0 = 1U;


    int currentValidity_0 = ClassifyBisector_0(bis_0, depth_3);
    if(currentValidity_0 > 0)
    {


        cbisectorData_0.bisectorState_0 = 1U;
        uint targetSlot_0 = atomicAdd(pParams_classificationBuffer_0._data[uint(0UL)], 1U);
        pParams_classificationBuffer_0._data[uint(2UL + uint64_t(targetSlot_0))] = currentID_0;

#line 155
    }
    else
    {

#line 155
        int _S41;

#line 165
        if(currentValidity_0 >= -1)
        {

#line 165
            _S41 = 1;

#line 165
        }
        else
        {

#line 165
            _S41 = 0;

#line 165
        }

#line 165
        cbisectorData_0.flags_0 = uint(_S41);

#line 155
    }

#line 155
    bool _S42;

#line 168
    if(baseDepth_1 != depth_3)
    {

#line 168
        _S42 = currentValidity_0 < 0;

#line 168
    }
    else
    {

#line 168
        _S42 = false;

#line 168
    }

#line 168
    if(_S42)
    {

        cbisectorData_0.bisectorState_0 = 2U;


        if(heapID_0 % 2UL == 0UL)
        {

            uint targetSlot_1 = atomicAdd(pParams_classificationBuffer_0._data[uint(1UL)], 1U);
            pParams_classificationBuffer_0._data[uint(2UL + uint64_t(totalNumElements_1) + uint64_t(targetSlot_1))] = currentID_0;

#line 174
        }

#line 168
    }

#line 183
    pParams_bisectorDataBuffer_0._data[uint(currentID_0)] = packStorage_1(cbisectorData_0);
    return;
}


#line 14 5
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{

#line 14
    uint _S43 = gl_GlobalInvocationID.x;


    if(_S43 >= pParams_indirectDrawBuffer_0._data[uint(9)])
    {

#line 18
        return;
    }

    uint _S44 = pParams_indexedBisectorBuffer_0._data[uint(_S43)];


    BisectorGeometry_0 bis_1;
    uint _S45 = 3U * _S44;

#line 25
    bis_1.p_0[0] = pParams_currentVertexBuffer_0._data[uint(_S45)].xyz;
    bis_1.p_0[1] = pParams_currentVertexBuffer_0._data[uint(_S45 + 1U)].xyz;
    bis_1.p_0[2] = pParams_currentVertexBuffer_0._data[uint(_S45 + 2U)].xyz;
    bis_1.p_0[3] = pParams_currentVertexBuffer_0._data[uint(3U * pParams_geometry_0.totalNumElements_0 + _S44)].xyz;


    ClassifyElement_0(_S44, bis_1, pParams_geometry_0.totalNumElements_0, pParams_geometry_0.baseDepth_0);
    return;
}

