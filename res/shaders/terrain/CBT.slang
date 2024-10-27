import Parameters;

// The maximal size of the LDS is 16kbyte.
#ifndef WORKGROUP_SIZE
#define WORKGROUP_SIZE 64
#endif

/*
Level 0: 32 bit // [0, 131072] x 1, needs a minimum of 18 bits (rounded up to 32 for alignment and required for atomic operations)
Level 1: 32 bit // [0, 65536] x 2, needs a minimum of 17 bits (rounded up to 32 for alignment and required for atomic operations)
Level 2: 32 bit // [0, 32768] x 4, needs a minimum of 16 bits (bumped to 32 bits for atomic operations)
Level 3: 32 bit // [0, 16384] x 8, needs a minimum of 15 bits (rounded up to 16 for alignment and bumped to 32 bits for atomic operations)
Level 4: 32 bit // [0, 8192] x 16, needs a minimum of 14 bits (rounded up to 16 for alignment and bumped to 32 bits for atomic operations)
Level 5: 32 bit // [0, 4096] x 32, needs a minimum of 13 bits (rounded up to 16 for alignment and bumped to 32 bits for atomic operations)
Level 6: 32 bit // [0, 2048] x 64, needs a minimum of 12 bits (rounded up to 16 for alignment and bumped to 32 bits for atomic operations)

Level 7: 16 bit // [0, 1024] x 128, needs a minimum of 11 bits (rounded up to 16 for alignment)
Level 8: 16 bit // [0, 512] x 256, needs a minimum of 10 bits (rounded up to 16 for alignment)
Level 9: 16 bit // [0, 256] x 512, needs a minimum of 9 bits (rounded up to 16 for alignment)
Level 10: 8 bit // [0, 128] x 1024, needs a minimum of 8 bits

Level 11: Raw 64 bits representation
*/

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

// Define the remaining values
#define BUFFER_ELEMENT_PER_LANE ((OCBT_TREE_NUM_SLOTS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE)
#define BUFFER_ELEMENT_PER_LANE_NO_BITFIELD ((OCBT_TREE_NUM_SLOTS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE)
#define BITFIELD_ELEMENT_PER_LANE ((OCBT_BITFIELD_NUM_SLOTS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE)
#define WAVE_TREE_DEPTH uint(log2(OCBT_NUM_ELEMENTS))

uint32_t cbt_size()
{
    return OCBT_NUM_ELEMENTS;
}

uint32_t last_level_offset()
{
    return OCBT_depth_offset[TREE_LAST_LEVEL] / 32;
}

groupshared uint gs_cbtTree[OCBT_TREE_NUM_SLOTS];

// Function that sets a given bit
void set_bit(uint bitID, bool state)
{
    // Coordinates of the bit
    uint32_t slot = bitID / 64;
    uint32_t local_id = bitID % 64;

    if (state)
        pParams.bitFieldBuffer[slot] |= 1uLL << local_id;
    else
        pParams.bitFieldBuffer[slot] &= ~(1uLL << local_id);
}

void set_bit_atomic(uint bitID, bool state)
{
    // Coordinates of the bit
    uint32_t slot = bitID / 64;
    uint32_t local_id = bitID % 64;

    if (state)
        InterlockedOr(pParams.bitFieldBuffer[slot], 1uLL << local_id);
    else
        InterlockedAnd(pParams.bitFieldBuffer[slot], ~(1uLL << local_id));
}

uint get_bit(uint bitID)
{
    uint32_t slot = bitID / 64;
    uint32_t local_id = bitID % 64;
    return uint((pParams.bitFieldBuffer[slot] & (1uLL << local_id)) >> local_id);
}

uint get_heap_element(uint id, inout HeapDebug debug)
{
    // Figure out the location of the first bit of this element
    uint32_t real_heap_id = id - 1;
    uint32_t depth = uint32_t(log2(real_heap_id + 1));
    uint32_t level_first_element = (1u << depth) - 1;
    uint32_t id_in_level = real_heap_id - level_first_element;
    uint32_t first_bit = OCBT_depth_offset[depth] + OCBT_bit_count[depth] * id_in_level;
    debug.id = id;
    debug.real_heap_id = real_heap_id;
    debug.depth = depth;
    debug.level_first_element = level_first_element;
    debug.id_in_level = id_in_level;
    debug.first_bit = first_bit;
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
        return countbits(uint(target_bits >> 32)) + countbits(uint(target_bits));
    }
}

// Should not be called if depth > TREE_LAST_LEVEL
void set_heap_element(uint id, uint value)
{
    // Figure out the location of the first bit of this element
    uint real_heap_id = id - 1;
    uint depth = uint(log2(real_heap_id + 1));
    uint level_first_element = (1u << depth) - 1;
    uint first_bit = OCBT_depth_offset[depth] + OCBT_bit_count[depth] * (real_heap_id - level_first_element);

    // Find the slot and the local first bit
    uint slot = first_bit / 32;
    uint local_id = first_bit % 32;

    // Extract the relevant bits
    gs_cbtTree[slot] &= ~(uint32_t(OCBT_bit_mask[depth]) << local_id);
    gs_cbtTree[slot] |= ((uint32_t(OCBT_bit_mask[depth]) & value) << local_id);
}

// Should not be called if depth > TREE_LAST_LEVEL
void set_heap_element_atomic(uint id, uint value)
{
    // Figure out the location of the first bit of this element
    uint real_heap_id = id - 1;
    uint depth = uint(log2(real_heap_id + 1));
    uint level_first_element = (1u << depth) - 1;
    uint first_bit = OCBT_depth_offset[depth] + OCBT_bit_count[depth] * (real_heap_id - level_first_element);

    // Find the slot and the local first bit
    uint slot = first_bit / 32;
    uint local_id = first_bit % 32;

    // Extract the relevant bits
    InterlockedAnd(gs_cbtTree[slot], ~(uint32_t(OCBT_bit_mask[depth]) << local_id));
    InterlockedOr(gs_cbtTree[slot], ((uint32_t(OCBT_bit_mask[depth]) & value) << local_id));
}

// Function that returns the number of active bits
uint bit_count()
{
    return gs_cbtTree[0];
}

uint bit_count(uint depth, uint element)
{
    HeapDebug debug;
    return get_heap_element((1u << depth) + element, debug);
}

// decodes the position of the i-th zero in the bitfield
uint decode_bit_complement(uint handle, uint dispatchID)
{
    uint temp = handle;
    DebugStruct debug;
    uint x = 0;
    uint bitID = 1u;
    uint c = OCBT_NUM_ELEMENTS / 2u;

    while (bitID < OCBT_NUM_ELEMENTS) {
        uint heapValue = c - get_heap_element(2u * bitID, debug.heapValues[x++]);
        uint b = handle < heapValue ? 0u : 1u;

        bitID = 2u * bitID + b;
        handle -= heapValue * b;
        c /= 2u;
    }

    uint result = (bitID ^ OCBT_NUM_ELEMENTS);
    handle = temp;
    for(uint i = 0; i < OCBT_NUM_ELEMENTS; ++i)
    {
        if(get_bit(i) == 0)
        {
            if(handle == 0)
            {
                pParams.debugBuffer[dispatchID] = debug;
                return i;
            }
            handle--;
        }
    }
}

void reduce(uint groupIndex)
{
    HeapDebug debug;
	// First we do a reduction until each lane has exactly one element to process
	uint initial_pass_size = OCBT_NUM_ELEMENTS / WORKGROUP_SIZE;
    for (uint it = initial_pass_size / 64, offset = OCBT_NUM_ELEMENTS / 64; it > 0 ; it >>=1, offset >>=1)
    {
        uint minHeapID = offset + (groupIndex * it);
        uint maxHeapID = offset + ((groupIndex + 1) * it);

        for (uint heapID = minHeapID; heapID < maxHeapID; ++heapID)
        {
            set_heap_element(heapID, get_heap_element(heapID * 2, debug) + get_heap_element(heapID * 2 + 1, debug));
        }
    }
	GroupMemoryBarrierWithGroupSync();

	for(uint s = WORKGROUP_SIZE / 2; s > 0u; s >>= 1)
    {
        if (groupIndex < s)
        {
            uint v = s + groupIndex;
            set_heap_element(v, get_heap_element(v * 2, debug) + get_heap_element(v * 2 + 1, debug));
        }
        GroupMemoryBarrierWithGroupSync();
    }
}

void reduce_prepass(uint dispatchThreadID)
{
    // Initialize the packed sum
    uint packedSum = 0;

    // Loop through the 4 pairs to process
    for (uint pairIdx = 0; pairIdx < 4; ++pairIdx)
    {
        // First element of the pair
        uint64_t target_bits = pParams.bitFieldBuffer[dispatchThreadID * 8 + 2 * pairIdx];
        uint elementC = countbits(uint(target_bits >> 32)) + countbits(uint(target_bits));

        // Second element of the pair
        target_bits = pParams.bitFieldBuffer[dispatchThreadID * 8 + 2 * pairIdx + 1];
        elementC += countbits(uint(target_bits >> 32)) + countbits(uint(target_bits));

        // Store in the right bits
        packedSum |= (elementC << pairIdx * 8);
    }

    // Offset of the last level of the tree
    const uint bufferOffset = last_level_offset();

    // Store the result into the bitfield
    pParams.cbtBuffer[bufferOffset + dispatchThreadID] = packedSum;
}

void reduce_first_pass(uint dispatchThreadID, uint groupIndex)
{
    HeapDebug debug;
    // Load the lowest level (and only the last level)
    const uint level0Offset = OCBT_depth_offset[TREE_LAST_LEVEL] / 32;
    if (groupIndex % 2 == 0)
        gs_cbtTree[level0Offset + dispatchThreadID / 2] = pParams.cbtBuffer[level0Offset + dispatchThreadID / 2];

    GroupMemoryBarrierWithGroupSync();

    // First we do a reduction until each lane has exactly one element to process
    uint initial_pass_size = OCBT_LAST_LEVEL_SIZE / 2;
    uint it, offset;
    for (it = initial_pass_size / 512, offset = initial_pass_size; it > 1; it >>=1, offset >>=1)
    {
        uint minHeapID = offset + (dispatchThreadID * it);
        uint maxHeapID = offset + ((dispatchThreadID + 1) * it);

        for (uint heapID = minHeapID; heapID < maxHeapID; ++heapID)
        {
            set_heap_element(heapID, get_heap_element(heapID * 2, debug) + get_heap_element(heapID * 2 + 1, debug));
        }
    }

    // Last pass needs to be atomic
    uint heapID = offset + (dispatchThreadID * it);
    set_heap_element_atomic(heapID, get_heap_element(heapID * 2, debug) + get_heap_element(heapID * 2 + 1, debug));

    GroupMemoryBarrierWithGroupSync();

    const uint level2Offset = OCBT_depth_offset[TREE_LAST_LEVEL - 1] / 32;
    if (groupIndex % 2 == 0)
        pParams.cbtBuffer[level2Offset + dispatchThreadID / 2] = gs_cbtTree[level2Offset + dispatchThreadID / 2];

    const uint level3Offset = OCBT_depth_offset[TREE_LAST_LEVEL - 2] / 32;
    if (groupIndex % 4 == 0)
        pParams.cbtBuffer[level3Offset + dispatchThreadID / 4] = gs_cbtTree[level3Offset + dispatchThreadID / 4];
}

void reduce_second_pass(uint groupIndex)
{
    HeapDebug debug;
    // Load the lowest level (and only the last level)
    const uint level0Offset = OCBT_depth_offset[9] / 32;
    for (uint e = 0; e < 4; ++e)
    {
        uint target_element = 4 * groupIndex + e;
        gs_cbtTree[level0Offset + target_element] = pParams.cbtBuffer[level0Offset + target_element];
    }
    GroupMemoryBarrierWithGroupSync();

    // First we do a reduction until each lane has exactly one element to process
    uint initial_pass_size = 256;
    for (uint it = initial_pass_size / 64, offset = initial_pass_size; it > 0 ; it >>=1, offset >>=1)
    {
        uint minHeapID = offset + (groupIndex * it);
        uint maxHeapID = offset + ((groupIndex + 1) * it);

        for (uint heapID = minHeapID; heapID < maxHeapID; ++heapID)
        {
            set_heap_element(heapID, get_heap_element(heapID * 2, debug) + get_heap_element(heapID * 2 + 1, debug));
        }
    }
    GroupMemoryBarrierWithGroupSync();
    
    for(uint s = WORKGROUP_SIZE / 2; s > 0u; s >>= 1)
    {
        if (groupIndex < s)
        {
            uint v = s + groupIndex;
            set_heap_element(v, get_heap_element(v * 2, debug) + get_heap_element(v * 2 + 1, debug));
        }
        GroupMemoryBarrierWithGroupSync();
    }

    // Make sure all the previous operations are done
    GroupMemoryBarrierWithGroupSync();

    // Load the bitfield to the LDS
    for (uint e = 0; e < 5; ++e)
    {
        uint target_element = 5 * groupIndex + e;
        if (target_element < 319)
            pParams.cbtBuffer[target_element] = gs_cbtTree[target_element];
    }
}

void reduce_no_bitfield(uint groupIndex)
{
    HeapDebug debug;
    // First we do a reduction until each lane has exactly one element to process
    uint initial_pass_size = OCBT_NUM_ELEMENTS / WORKGROUP_SIZE;
    for (uint it = initial_pass_size / 128, offset = OCBT_NUM_ELEMENTS / 128; it > 0 ; it >>=1, offset >>=1)
    {
        uint minHeapID = offset + (groupIndex * it);
        uint maxHeapID = offset + ((groupIndex + 1) * it);

        for (uint heapID = minHeapID; heapID < maxHeapID; ++heapID)
        {
            set_heap_element(heapID, get_heap_element(heapID * 2, debug) + get_heap_element(heapID * 2 + 1, debug));
        }
    }
    GroupMemoryBarrierWithGroupSync();

    for(uint s = WORKGROUP_SIZE / 2; s > 0u; s >>= 1)
    {
        if (groupIndex < s)
        {
            uint v = s + groupIndex;
            set_heap_element(v, get_heap_element(v * 2, debug) + get_heap_element(v * 2 + 1, debug));
        }
        GroupMemoryBarrierWithGroupSync();
    }
}

void clear_cbt(uint groupIndex)
{
    for (uint v = 0; v < BUFFER_ELEMENT_PER_LANE; ++v)
    {
        uint target_element = BUFFER_ELEMENT_PER_LANE * groupIndex + v;
        if (target_element < OCBT_TREE_NUM_SLOTS)
            gs_cbtTree[target_element] = 0;
    }

    for (uint b = 0; b < BITFIELD_ELEMENT_PER_LANE; ++b)
    {
        uint target_element = BITFIELD_ELEMENT_PER_LANE * groupIndex + b;
        if (target_element < OCBT_BITFIELD_NUM_SLOTS)
            pParams.bitFieldBuffer[target_element] = 0;
    }
    GroupMemoryBarrierWithGroupSync();
}

// Importante note
// Depending on your target GPU architecture, the pattern used to load has a different performance behavior
// here is the best performant based on our tests:
// NVIDIA uint target_element = groupIndex + WORKGROUP_SIZE * e;
// AMD uint target_element = BUFFER_ELEMENT_PER_LANE * groupIndex + e;

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

void load_shared_memory_to_buffer(uint groupIndex)
{
    // Make sure all the previous operations are done
    GroupMemoryBarrierWithGroupSync();

    // Load the bitfield to the LDS
    for (uint e = 0; e < BUFFER_ELEMENT_PER_LANE; ++e)
    {
        uint target_element = BUFFER_ELEMENT_PER_LANE * groupIndex + e;
        if (target_element < OCBT_TREE_NUM_SLOTS)
            pParams.cbtBuffer[target_element] = gs_cbtTree[target_element];
    }
}

void set_bit_atomic_buffer(uint bitID, bool state)
{
    // Coordinates of the bit
    uint32_t slot = bitID / 64;
    uint32_t local_id = bitID % 64;

    if (state)
        InterlockedOr(pParams.bitFieldBuffer[slot], 1uLL << local_id);
    else
        InterlockedAnd(pParams.bitFieldBuffer[slot], ~(1uLL << local_id));
}

uint32_t bit_count_buffer()
{
    return pParams.cbtBuffer[0];
}
