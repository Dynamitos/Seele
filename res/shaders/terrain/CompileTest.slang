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
    RWStructuredBuffer<uint32_t> bitFieldBuffer;
};
ParameterBlock<ComputeParams> pParams;

void SplitElement(uint currentID, uint baseDepth, uint dispatchID)
{
    // Get the neighbors information
    uint4 cNeighbors = pParams.neighboursBuffer[currentID];

    // If there is a neighbor X
    if (cNeighbors.x != INVALID_POINTER)
    {
        // This is on the path of it's neighbor X
        uint4 xNeighbors = pParams.neighboursBuffer[cNeighbors.x];
        if (xNeighbors.z == currentID && pParams.bisectorDataBuffer[cNeighbors.x].bisectorState != UNCHANGED_ELEMENT)
            return;
    }

    // If there is a neighbor Y
    if (cNeighbors.y != INVALID_POINTER)
    {
        // This is on the path of it's neighbor Y
        uint4 yNeighbors = pParams.neighboursBuffer[cNeighbors.y];
        if (yNeighbors.z == currentID && pParams.bisectorDataBuffer[cNeighbors.y].bisectorState != UNCHANGED_ELEMENT)
            return;
    }

    // Depth of the current triangle
    uint64_t heapID = pParams.heapIDBuffer[currentID];
    uint currentDepth = HeapIDDepth(heapID);

    // Compute the maximal required memory for this subdivision
    int maxRequiredMemory = 2 * (currentDepth - baseDepth) - 1;

    // Get the twin information
    uint twinID = cNeighbors.z;

    // This avoid the massive over-reservation and saves a bunch of artifacts
    if (twinID == INVALID_POINTER)
        maxRequiredMemory = 1;
    else if (pParams.neighboursBuffer[twinID].z == currentID)
        maxRequiredMemory = 2;

    // Try to reserve
    int remainingMemory;
    InterlockedAdd(pParams.memoryBuffer[1], -maxRequiredMemory, remainingMemory);
    // Did someone manage to sneak-in while we were trying to pick the memory, add it back and try again
    if (remainingMemory < maxRequiredMemory)
    {
        // Then add back the required memory and stop
        InterlockedAdd(pParams.memoryBuffer[1], maxRequiredMemory, remainingMemory);
        return;
    }

    // Let's actually count the memory that we will be using
    uint usedMemory = 1;
    uint prevPattern;
    InterlockedOr(pParams.bisectorDataBuffer[currentID].subdivisionPattern, CENTER_SPLIT, prevPattern);

    // If this is not zero, it means an other neighbor went faster than us, we restore the memory and leave.
    if (prevPattern != 0)
    {
        InterlockedAdd(pParams.memoryBuffer[1], maxRequiredMemory, remainingMemory);
        return;
    }

    // Mark this for allocation
    uint targetLocation = 0;
    InterlockedAdd(pParams.allocateBuffer[0], 1, targetLocation);
    pParams.allocateBuffer[1 + targetLocation] = currentID;

    // While we're not done (up the tree or everything is subdivided properly)
    bool done = false;
    while (!done)
    {
        // If this neighbor is not allocated, we're done.
        if (twinID == INVALID_POINTER)
            break;

        // Grab the bisector of the neighbor
        uint64_t nHeapID = pParams.heapIDBuffer[twinID];
        BisectorData nBisectorData = pParams.bisectorDataBuffer[twinID];
        uint nDepth = HeapIDDepth(nHeapID);
        uint4 nNeighbors = pParams.neighboursBuffer[twinID];

        // If both triangles have the same depth
        if (nDepth == currentDepth)
        {
            // Raised the center split
            InterlockedOr(pParams.bisectorDataBuffer[twinID].subdivisionPattern, CENTER_SPLIT, prevPattern);

            // Only account for it if it was not raised before.
            if (prevPattern == 0)
            {
                // Mark this for allocation
                uint targetLocation = 0;
                InterlockedAdd(pParams.allocateBuffer[0], 1, targetLocation);
                pParams.allocateBuffer[1 + targetLocation] = twinID;
                usedMemory++;
            }

            // And we're done
            done = true;
        }
        // If this node has already been subdivided, it means that we need to add the third subdivision and we're done
        else
        {
            if (nNeighbors[0] == currentID)
                InterlockedOr(pParams.bisectorDataBuffer[twinID].subdivisionPattern, RIGHT_DOUBLE_SPLIT, prevPattern);
            else // if (nNeighbors[1] == currentID)
                InterlockedOr(pParams.bisectorDataBuffer[twinID].subdivisionPattern, LEFT_DOUBLE_SPLIT, prevPattern);

            if (prevPattern != 0)
            {
                usedMemory++;
                done = true;
            }
            else
            {
                // Mark this for allocation
                uint targetLocation = 0;
                InterlockedAdd(pParams.allocateBuffer[0], 1, targetLocation);
                pParams.allocateBuffer[1 + targetLocation] = twinID;
                
                // Account for two splits
                usedMemory += 2;

                // the new bisector that needs to be propagated
                currentID = twinID;
                currentDepth = nDepth;
                twinID = pParams.neighboursBuffer[currentID].z;
            }
        }
    }
    int change = maxRequiredMemory - usedMemory;
    if(change > 0)
    {
        // Add back the unused memory (in case)
        InterlockedAdd(pParams.memoryBuffer[1], change, remainingMemory);
    }
}

[numthreads(64, 1, 1)]
void Split(uint dispatchID : SV_DispatchThreadID)
{
    if (dispatchID >= pParams.classificationBuffer[SPLIT_COUNTER])
        return;

    // Grab the real elementID
    uint currentID = pParams.classificationBuffer[CLASSIFY_COUNTER_OFFSET + dispatchID];

    // Split the element
    SplitElement(currentID, 7, dispatchID);
}
