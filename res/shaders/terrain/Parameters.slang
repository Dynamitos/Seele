import Common;
import Bisector;

struct GeometryCB
{
    uint32_t totalNumElements;
    uint32_t baseDepth;
    uint32_t totalNumVertices;
};

struct DeformationCB
{
    float4 patchSize;
    float4 patchRoughness;
    float choppiness;
    int attenuation;
    float amplification;
    uint32_t patchFlags;
};

struct UpdateCB
{
    float triangleSize;
    uint32_t maxSubdivisionDepth;
    float fov;
    float farPlaneDistance;
}

struct HeapDebug
{
    uint id;
    uint real_heap_id;
    uint depth;
    uint level_first_element;
    uint id_in_level;
    uint first_bit;
};

struct DebugStruct
{
    HeapDebug heapValues[18];
};


struct ComputeParams
{
    // 0
    ConstantBuffer<GeometryCB> geometry;
    // 1
    ConstantBuffer<UpdateCB> update;

    // 2
	RWStructuredBuffer<float4> currentVertexBuffer;
    // 3
	StructuredBuffer<uint> indexedBisectorBuffer;
    // 4
    RWStructuredBuffer<uint> indirectDrawBuffer;
    // 5
    RWStructuredBuffer<uint64_t> heapIDBuffer;
    // 6
    RWStructuredBuffer<BisectorData> bisectorDataBuffer;
    // 7
    RWStructuredBuffer<uint> classificationBuffer;
    // 8
    RWStructuredBuffer<int> allocateBuffer;
    // 9
    RWStructuredBuffer<uint> indirectDispatchBuffer;
    // 10
    RWStructuredBuffer<uint4> neighboursBuffer;
    // 11
    RWStructuredBuffer<uint4> neighboursOutputBuffer;
    // 12
    RWStructuredBuffer<int> memoryBuffer;
    // 13
    RWStructuredBuffer<uint> cbtBuffer;
    // 14
    RWStructuredBuffer<uint64_t> bitFieldBuffer;
    // 15
    RWStructuredBuffer<int> propagateBuffer;
    // 16
    RWStructuredBuffer<uint> simplifyBuffer;
    // 17
    RWStructuredBuffer<uint> validationBuffer;
    // 18
    RWStructuredBuffer<uint> bisectorIndicesBuffer;
    // 19
    RWStructuredBuffer<uint> visibleBisectorIndices;
    // 20
    RWStructuredBuffer<uint> modifiedBisectorIndices;
    // 21
    RWStructuredBuffer<float4> lebPositionBuffer;
    // 22
    StructuredBuffer<float> lebMatrixCache;
    // 23
    RWStructuredBuffer<DebugStruct> debugBuffer;
    // 24
    SamplerState displacementSampler;
    // 25
    Texture2D<float4> displacementMap;
};
ParameterBlock<ComputeParams> pParams;
