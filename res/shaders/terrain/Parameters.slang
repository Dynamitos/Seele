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
    float4x4 viewProjectionMatrix;
    float triangleSize;
    uint32_t maxSubdivisionDepth;
    float fov;
    float farPlaneDistance;
}

struct DebugStruct
{
    int maxRequiredMemory;
    int usedMemory;
    uint memoryChange;
    uint twinID;
};


struct ComputeParams
{
    ConstantBuffer<GeometryCB> geometry;
    ConstantBuffer<UpdateCB> update;

	RWStructuredBuffer<float4> currentVertexBuffer;
	StructuredBuffer<uint> indexedBisectorBuffer;
    RWStructuredBuffer<uint> indirectDrawBuffer;
    RWStructuredBuffer<uint64_t> heapIDBuffer;
    RWStructuredBuffer<BisectorData> bisectorDataBuffer;
    RWStructuredBuffer<uint> classificationBuffer;
    RWStructuredBuffer<int> allocateBuffer;
    RWStructuredBuffer<uint> indirectDispatchBuffer;
    RWStructuredBuffer<uint4> neighboursBuffer;
    RWStructuredBuffer<uint4> neighboursOutputBuffer;
    RWStructuredBuffer<int> memoryBuffer;
    RWStructuredBuffer<uint> cbtBuffer;
    RWStructuredBuffer<uint64_t> bitFieldBuffer;
    RWStructuredBuffer<int> propagateBuffer;
    RWStructuredBuffer<uint> simplifyBuffer;
    RWStructuredBuffer<uint> validationBuffer;
    RWStructuredBuffer<uint> bisectorIndicesBuffer;
    RWStructuredBuffer<uint> visibleBisectorIndices;
    RWStructuredBuffer<uint> modifiedBisectorIndices;
    RWStructuredBuffer<float4> lebPositionBuffer;
    StructuredBuffer<float3x3> lebMatrixCache;
    globallycoherent RWStructuredBuffer<DebugStruct> debugBuffer;
};
ParameterBlock<ComputeParams> pParams;
