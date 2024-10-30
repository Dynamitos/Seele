
struct ComputeParams
{
    RWStructuredBuffer<uint64_t> bitFieldBuffer;
};
ParameterBlock<ComputeParams> pParams;

static const uint32_t OCBT_bit_mask[18] = { 
                            0xffffffff, // Root 17
                            0xffffffff, // Level 16
                            0xffffffff, // level 15
                            0xffffffff, // level 14
                            0xffffffff, // level 13
                            0xffffffff, // level 12
                            0xffffffff, // level 11

                            0xffff, // level 10
                            0xffff, // level 9
                            0xffff, // level 8
                            0xff, // level 8

                            0xffffffff, // level 7
                            0xffffffff, // Level 6
                            0xffff, // level 5
                            0xff, // level 4
                            0xf, // level 3
                            0x3, // level 2
                            0x1, // level 1
};

[numthreads(1, 1, 1)]
[shader("compute")]
void TestHeap(uint dispatchID: SV_DispatchThreadID)
{
    pParams.bitFieldBuffer[dispatchID] = OCBT_bit_mask[dispatchID];
}
