
struct ComputeParams
{
    RWStructuredBuffer<uint> indirectDrawBuffer;
    RWStructuredBuffer<uint64_t> heapIDBuffer;
    RWStructuredBuffer<uint> classificationBuffer;
    RWStructuredBuffer<int> allocateBuffer;
    RWStructuredBuffer<int> memoryBuffer;
    RWStructuredBuffer<int> propagateBuffer;
    RWStructuredBuffer<uint> simplifyBuffer;
};
ParameterBlock<ComputeParams> pParams;

// Split buffer slots
static const uint64_t SPLIT_COUNTER = 0;
static const uint64_t SIMPLIFY_COUNTER = 1;
static const uint64_t CLASSIFY_COUNTER_OFFSET = 2;
void ResetBuffers()
{
    pParams.memoryBuffer[0] = 0;
    pParams.memoryBuffer[1] = 0;

    pParams.classificationBuffer[SPLIT_COUNTER] = 0;
    pParams.classificationBuffer[SIMPLIFY_COUNTER] = 0;

    pParams.allocateBuffer[0] = 0;

    pParams.propagateBuffer[0] = 0;
    pParams.propagateBuffer[1] = 0;

    pParams.simplifyBuffer[0] = 0;

    pParams.indirectDrawBuffer[0] = 0;
    pParams.indirectDrawBuffer[1] = 1;
    pParams.indirectDrawBuffer[2] = 0;
    pParams.indirectDrawBuffer[3] = 0;

    pParams.indirectDrawBuffer[4] = 0;
    pParams.indirectDrawBuffer[5] = 1;
    pParams.indirectDrawBuffer[6] = 0;
    pParams.indirectDrawBuffer[7] = 0;

    pParams.indirectDrawBuffer[8] = 0;
}

[numthreads(1, 1, 1)]
void Reset()
{
    ResetBuffers();
}