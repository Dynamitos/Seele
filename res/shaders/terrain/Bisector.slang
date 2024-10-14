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

struct BisectorData
{
    // Subvision that should be applied to this bisector
    uint32_t subdivisionPattern;

    // Allocated indices for this bisector
    uint32_t indices[3];

    // Neighbor that should be processed
    uint32_t problematicNeighbor;

    // State of this bisector (split, merge, etc)
    uint32_t bisectorState;

    // Visibility and modification flags of a bisector
    uint32_t flags;

    // ID used for the propagation
    uint32_t propagationID;
};

uint heapIDDepth(uint64_t x)
{
    uint depth = 0;
    while (x > 0u) {
        ++depth;
        x >>= 1u;
    }
    return depth;
}