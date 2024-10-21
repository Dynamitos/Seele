// Pointer to an invalid neighbor or index
const static int INVALID_POINTER = 4294967295;

// Possible culling state
const static int BACK_FACE_CULLED = -3;
const static int FRUSTUM_CULLED = -2;
const static int TOO_SMALL = -1;
const static int UNCHANGED_ELEMENT = 0;
const static int BISECT_ELEMENT = 1;
const static int SIMPLIFY_ELEMENT = 2;
const static int MERGED_ELEMENT = 3;

// Bisector flags
const static int VISIBLE_BISECTOR = 0x1;
const static int MODIFIED_BISECTOR = 0x2;


// Possible splits
static const uint64_t NO_SPLIT = 0x00;
static const uint64_t CENTER_SPLIT = 0x01;
static const uint64_t RIGHT_SPLIT = 0x02;
static const uint64_t LEFT_SPLIT = 0x04;
static const uint64_t RIGHT_DOUBLE_SPLIT = (CENTER_SPLIT | RIGHT_SPLIT);
static const uint64_t LEFT_DOUBLE_SPLIT = (CENTER_SPLIT | LEFT_SPLIT);
static const uint64_t TRIPLE_SPLIT = (CENTER_SPLIT | RIGHT_SPLIT | LEFT_SPLIT);

// Split buffer slots
static const uint64_t SPLIT_COUNTER = 0;
static const uint64_t SIMPLIFY_COUNTER = 1;
static const uint64_t CLASSIFY_COUNTER_OFFSET = 2;


struct BisectorData
{
    // Allocated indices for this bisector
    uint32_t indices[3];

    // Subvision that should be applied to this bisector
    uint32_t subdivisionPattern;

    // Neighbor that should be processed
    uint32_t problematicNeighbor;

    // State of this bisector (split, merge, etc)
    uint32_t bisectorState;

    // Visibility and modification flags of a bisector
    uint32_t flags;

    // ID used for the propagation
    uint32_t propagationID;
};

uint HeapIDDepth(uint64_t x)
{
    uint depth = 0;
    while (x > 0u) {
        ++depth;
        x >>= 1u;
    }
    return depth;
}
struct ComputeParams
{
    RWStructuredBuffer<uint> indirectDrawBuffer;
    RWStructuredBuffer<uint64_t> heapIDBuffer;
    RWStructuredBuffer<uint> classificationBuffer;
    RWStructuredBuffer<int> allocateBuffer;
    RWStructuredBuffer<int> memoryBuffer;
    RWStructuredBuffer<uint4> neighboursBuffer;
    RWStructuredBuffer<BisectorData> bisectorDataBuffer;
    RWStructuredBuffer<int> propagateBuffer;
    RWStructuredBuffer<uint> simplifyBuffer;
    RWStructuredBuffer<uint> cbtBuffer;
    RWStructuredBuffer<uint64_t> bitFieldBuffer;
};
ParameterBlock<ComputeParams> pParams;

#define WORKGROUP_SIZE 64

// Num elements
#define OCBT_NUM_ELEMENTS 131072
// Tree sizes
#define OCBT_TREE_SIZE_BITS (32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64 + 16 * 128 + 16 * 256 + 16 * 512 + 8 * 1024)
#define OCBT_TREE_NUM_SLOTS (OCBT_TREE_SIZE_BITS / 32)
#define OCBT_BITFIELD_NUM_SLOTS (OCBT_NUM_ELEMENTS / 64)
#define OCBT_LAST_LEVEL_SIZE 1024

// Tree last level
#define TREE_LAST_LEVEL 10
// First virtual level
#define FIRST_VIRTUAL_LEVEL 11
// Leaf level
#define LEAF_LEVEL 17

// per level offset
static const uint32_t OCBT_depth_offset[18] = { 0, // Level 0
                                        32 * 1, // level 1
                                        32 * 1 + 32 * 2, // level 2
                                        32 * 1 + 32 * 2 + 32 * 4, // level 3
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8, // Level 4
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16, // Level 5
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32, // Level 6
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64, // Level 7

                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64 + 16 * 128, // Level 8
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64 + 16 * 128 + 16 * 256, // Level 9
                                        32 * 1 + 32 * 2 + 32 * 4 + 32 * 8 + 32 * 16 + 32 * 32 + 32 * 64 + 16 * 128 + 16 * 256 + 16 * 512, // Level 10

                                        0, // Level 12
                                        0, // Level 13
                                        0, // Level 14
                                        0, // Level 15
                                        0, // Level 16
                                        0, // Level 17
                                        0, // Level 18
};

static const uint64_t OCBT_bit_mask[18] = { 0xffffffff, // Root 17
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

                            0xffffffffffffffff, // level 7
                            0xffffffff, // Level 6
                            0xffff, // level 5
                            0xff, // level 4
                            0xf, // level 3
                            0x3, // level 2
                            0x1, // level 1
};

static const uint32_t OCBT_bit_count[18] = { 32, // Root 17
                            32, // Level 16
                            32, // level 15
                            32, // level 14
                            32, // level 13
                            32, // level 12
                            32, // level 11

                            16, // level 10
                            16, // level 9
                            16, // level 8
                            8, // level 8

                            64, // Level 5
                            32, // Level 5
                            16, // Level 4
                            8, // level 3
                            4, // level 2
                            2, // level 1
                            1, // level 0
};

groupshared uint gs_cbtTree[OCBT_TREE_NUM_SLOTS];

// Define the remaining values
#define BUFFER_ELEMENT_PER_LANE ((OCBT_TREE_NUM_SLOTS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE)
#define BUFFER_ELEMENT_PER_LANE_NO_BITFIELD ((OCBT_TREE_NUM_SLOTS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE)
#define BITFIELD_ELEMENT_PER_LANE ((OCBT_BITFIELD_NUM_SLOTS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE)
#define WAVE_TREE_DEPTH uint(log2(OCBT_NUM_ELEMENTS))

void load_buffer_to_shared_memory(uint groupIndex)
{
    // Load the bitfield to the LDS
    for (uint e = 0; e < BUFFER_ELEMENT_PER_LANE; ++e)
    {
        uint target_element = BUFFER_ELEMENT_PER_LANE * groupIndex + e;
        if (target_element < OCBT_TREE_NUM_SLOTS)
            gs_cbtTree[target_element] = pParams.cbtBuffer[target_element];
    }
    GroupMemoryBarrierWithGroupSync();
}

uint get_heap_element(uint id)
{
    // Figure out the location of the first bit of this element
    uint32_t real_heap_id = id - 1;
    uint32_t depth = uint32_t(log2(real_heap_id + 1));
    uint32_t level_first_element = (1u << depth) - 1;
    uint32_t id_in_level = real_heap_id - level_first_element;
    uint32_t first_bit = OCBT_depth_offset[depth] + OCBT_bit_count[depth] * id_in_level;
    if (depth < FIRST_VIRTUAL_LEVEL)
    {
        uint32_t slot = first_bit / 32;
        uint32_t local_id = first_bit % 32;
        uint32_t target_bits = (gs_cbtTree[slot] >> local_id) & uint32_t(OCBT_bit_mask[depth]);
        return (gs_cbtTree[slot] >> local_id) & uint32_t(OCBT_bit_mask[depth]);
    }
    else
    {
        uint32_t slot = first_bit / 64;
        uint32_t local_id = first_bit % 64;
        uint64_t target_bits = (pParams.bitFieldBuffer[slot] >> local_id) & OCBT_bit_mask[depth];
        uint32_t high = uint(target_bits >> 32);
        uint32_t low = uint(target_bits);
        return countbits(high) + countbits(low);
    }
}

[numthreads(64, 1, 1)]
void GetHeap(uint groupIndex : SV_GroupIndex, uint dispatchID : SV_DispatchThreadID)
{
    load_buffer_to_shared_memory(groupIndex);
    pParams.classificationBuffer[dispatchID] = get_heap_element(dispatchID);
}