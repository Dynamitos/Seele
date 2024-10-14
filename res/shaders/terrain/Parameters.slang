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

struct DebugStruct
{
    float4 sourceP[3];
    float2 areaP[3];
    float area;
    int validity;
    float fDotV;
    float vDotN;
}

struct ComputeParams
{
    ConstantBuffer<GeometryCB> geometry;
    ConstantBuffer<UpdateCB> update;

	RWStructuredBuffer<float3> currentVertexBuffer;
	StructuredBuffer<uint> indexedBisectorBuffer;
    RWStructuredBuffer<uint> indirectDrawBuffer;
    RWStructuredBuffer<uint64_t> heapIDBuffer;
    RWStructuredBuffer<BisectorData> bisectorDataBuffer;
    RWStructuredBuffer<uint> classificationBuffer;
    RWStructuredBuffer<int> allocateBuffer;
    RWStructuredBuffer<uint> indirectDispatchBuffer;
    RWStructuredBuffer<uint3> neighboursBuffer;
    RWStructuredBuffer<uint3> neighboursOutputBuffer;
    RWStructuredBuffer<int> memoryBuffer;
    RWStructuredBuffer<uint> cbtBuffer;
    RWStructuredBuffer<uint64_t> bitFieldBuffer;
    RWStructuredBuffer<int> propagateBuffer;
    RWStructuredBuffer<uint> simplifyBuffer;
    RWStructuredBuffer<uint> validationBuffer;
    RWStructuredBuffer<uint> bisectorIndicesBuffer;
    RWStructuredBuffer<uint> visibleBisectorIndices;
    RWStructuredBuffer<uint> modifiedBisectorIndices;
    RWStructuredBuffer<float3> lebPositionBuffer;
    StructuredBuffer<float3x3> lebMatrixCache;
    RWStructuredBuffer<DebugStruct> debugBuffer;
};
ParameterBlock<ComputeParams> pParams;
