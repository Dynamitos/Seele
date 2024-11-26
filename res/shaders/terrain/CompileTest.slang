
struct ComputeParams
{
    RWStructuredBuffer<uint64_t> bitFieldBuffer;
};
ParameterBlock<ComputeParams> pParams;

static const uint64_t OCBT_bit_mask[18] = { 
                            0xffffffffULL, // Root 17
                            0xffffffffULL, // Level 16
                            0xffffffffULL, // level 15
                            0xffffffffULL, // level 14
                            0xffffffffULL, // level 13
                            0xffffffffULL, // level 12
                            0xffffffffULL, // level 11

                            0xffffULL, // level 10
                            0xffffULL, // level 9
                            0xffffULL, // level 8
                            0xffULL, // level 8

                            0xffffffffffffffffULL, // level 7
                            0xffffffffULL, // Level 6
                            0xffffULL, // level 5
                            0xffULL, // level 4
                            0xfULL, // level 3
                            0x3ULL, // level 2
                            0x1ULL, // level 1
};

[numthreads(1, 1, 1)]
[shader("compute")]
void TestHeap(uint dispatchID: SV_DispatchThreadID)
{
    pParams.bitFieldBuffer[dispatchID] = OCBT_bit_mask[dispatchID];
}
