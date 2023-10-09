#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#pragma warning(disable: 3557)


#line 1 "lib/Scene.slang"
struct InstanceData_0
{
    matrix<float,int(4),int(4)>  transformMatrix_0;
};


#line 45 "test.slang"
StructuredBuffer<InstanceData_0 > scene_instances_0 : register(t0, space3);


#line 1 "lib/Meshlet.slang"
struct MeshletDescription_0
{
    uint vertexCount_0;
    uint primitiveCount_0;
    uint vertexOffset_0;
    uint primitiveOffset_0;
};


#line 46 "test.slang"
StructuredBuffer<MeshletDescription_0 > meshlets_meshletInfos_0 : register(t0, space2);


#line 9 "lib/Meshlet.slang"
StructuredBuffer<uint8_t > meshlets_primitiveIndices_0 : register(t1, space2);


#line 9
StructuredBuffer<uint > meshlets_vertexIndices_0 : register(t2, space2);


#line 35 "lib/StaticMeshVertexData.slang"
StructuredBuffer<float3 > vertexData_positions_0 : register(t0, space4);


#line 35
StructuredBuffer<float2 > vertexData_texCoords_0 : register(t1, space4);


#line 35
StructuredBuffer<float3 > vertexData_normals_0 : register(t2, space4);


#line 35
StructuredBuffer<float3 > vertexData_tangents_0 : register(t3, space4);


#line 35
StructuredBuffer<float3 > vertexData_biTangents_0 : register(t4, space4);


#line 5 "lib/Common.slang"
struct ViewParameter_0
{
    matrix<float,int(4),int(4)>  viewMatrix_0;
    matrix<float,int(4),int(4)>  projectionMatrix_0;
    float4 cameraPos_WS_0;
    float2 screenDimensions_0;
};

cbuffer viewParams_0 : register(b0, space1)
{
    ViewParameter_0 viewParams_0;
}

#line 24 "test.slang"
static groupshared uint gs_numVertices_0;


#line 5 "lib/StaticMeshVertexData.slang"
struct StaticMeshVertexAttributes_0
{
    float4 clipPosition_0 : SV_Position;
    float3 worldPosition_0 : POSITION0;
    float2 texCoords_0 : TEXCOORD0;
    float3 normal_0 : NORMAL0;
    float3 tangent_0 : TANGENT0;
    float3 biTangent_0 : BITANGENT0;
};


#line 22 "test.slang"
static groupshared StaticMeshVertexAttributes_0  gs_vertices_0[int(64)];


#line 37 "lib/StaticMeshVertexData.slang"
StaticMeshVertexAttributes_0 StaticMeshVertexData_getAttributes_0(StructuredBuffer<float3 > this_positions_0, StructuredBuffer<float2 > this_texCoords_0, StructuredBuffer<float3 > this_normals_0, StructuredBuffer<float3 > this_tangents_0, StructuredBuffer<float3 > this_biTangents_0, uint index_0, InstanceData_0 inst_0)
{


    float4 worldPos_0 = mul(inst_0.transformMatrix_0, float4(this_positions_0.Load(index_0), 1.0));

#line 39
    StaticMeshVertexAttributes_0 attr_0;

#line 44
    attr_0.clipPosition_0 = mul(viewParams_0.projectionMatrix_0, mul(viewParams_0.viewMatrix_0, worldPos_0));
    attr_0.worldPosition_0 = worldPos_0.xyz;
    attr_0.texCoords_0 = this_texCoords_0.Load(index_0);
    attr_0.normal_0 = this_normals_0.Load(index_0);
    attr_0.tangent_0 = this_tangents_0.Load(index_0);
    attr_0.biTangent_0 = this_biTangents_0.Load(index_0);
    return attr_0;
}


#line 25 "test.slang"
static groupshared uint gs_numPrimitives_0;


#line 23
static groupshared uint3  gs_indices_0[int(126)];


#line 39
[shader("mesh")][numthreads(32, 1, 1)]
[outputtopology("triangle")]
void meshMain(uint threadID_0 : SV_GROUPINDEX, uint3 groupID_0 : SV_GROUPID, vertices out StaticMeshVertexAttributes_0  vertices_0[int(64)], indices out uint3  indices_0[int(126)])
{


    InstanceData_0 _S1 = scene_instances_0.Load(threadID_0);
    MeshletDescription_0 _S2 = meshlets_meshletInfos_0.Load(groupID_0.x);

#line 51
    uint _S3 = _S2.vertexCount_0 - 1U;

#line 63
    uint _S4 = _S2.primitiveCount_0 - 1U;

#line 63
    uint loop_0 = 0U;

#line 63
    for(;;)
    {

#line 51
        uint v_0 = min(threadID_0 + loop_0 * 32U, _S3);
        InterlockedMax(gs_numVertices_0, v_0 + 1U);


        gs_vertices_0[v_0] = StaticMeshVertexData_getAttributes_0(vertexData_positions_0, vertexData_texCoords_0, vertexData_normals_0, vertexData_tangents_0, vertexData_biTangents_0, uint(int(meshlets_vertexIndices_0.Load(_S2.vertexOffset_0 + v_0))), _S1);

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
        uint p_0 = min(threadID_0 + loop_0 * 32U, _S4);
        InterlockedMax(gs_numPrimitives_0, p_0 + 1U);

        uint _S5 = p_0 * 3U;

#line 66
        uint _S6 = _S2.primitiveOffset_0 + _S5;



        uint idx1_0 = meshlets_vertexIndices_0.Load(_S2.vertexOffset_0 + uint(meshlets_primitiveIndices_0.Load(_S6 + 1U)));
        uint idx2_0 = meshlets_vertexIndices_0.Load(_S2.vertexOffset_0 + uint(meshlets_primitiveIndices_0.Load(_S6 + 2U)));
        gs_indices_0[_S5] = (uint3)meshlets_vertexIndices_0.Load(_S2.vertexOffset_0 + uint(meshlets_primitiveIndices_0.Load(_S6)));
        gs_indices_0[_S5 + 1U] = (uint3)idx1_0;
        gs_indices_0[_S5 + 2U] = (uint3)idx2_0;

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
    GroupMemoryBarrierWithGroupSync();
    SetMeshOutputCounts(gs_numVertices_0, gs_numPrimitives_0);
    GroupMemoryBarrierWithGroupSync();
    vertices_0[threadID_0] = gs_vertices_0[threadID_0];
    indices_0[threadID_0] = gs_indices_0[threadID_0];
    return;
}

