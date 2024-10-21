#pragma once
#include "Containers/Array.h"
#include "Graphics/RenderPass/RenderPass.h"
#include "MinimalEngine.h"
#include <bit>

namespace Seele {
constexpr auto GEOMETRY_CB = 0;
constexpr auto UPDATE_CB = 1;

constexpr auto CURRENT_VERTEX_BUFFER = 2;
constexpr auto INDEXED_BISECTOR_BUFFER = 3;
constexpr auto INDIRECT_DRAW_BUFFER = 4;
constexpr auto HEAP_ID_BUFFER = 5;
constexpr auto BISECTOR_DATA_BUFFER = 6;
constexpr auto CLASSIFICATION_BUFFER = 7;
constexpr auto ALLOCATE_BUFFER = 8;
constexpr auto INDIRECT_DISPATCH_BUFFER = 9;
constexpr auto NEIGHBOURS_BUFFER = 10;
constexpr auto NEIGHBOURS_OUTPUT_BUFFER = 11;
constexpr auto MEMORY_BUFFER = 12;
constexpr auto CBT_BUFFER0 = 13;
constexpr auto CBT_BUFFER1 = 14;
constexpr auto PROPAGATE_BUFFER = 15;
constexpr auto SIMPLIFICATION_BUFFER = 16;
constexpr auto VALIDATION_BUFFER = 17;
constexpr auto BISECTOR_INDICES = 18;
constexpr auto VISIBLE_BISECTOR_INDICES = 19;
constexpr auto MODIFIED_BISECTOR_INDICES = 20;
constexpr auto LEB_POSITION_BUFFER = 21;
constexpr auto LEB_MATRIX_CACHE = 22;
constexpr auto DEBUG_BUFFER = 23;

constexpr uint64 WORKGROUP_SIZE = 64;
template <size_t Power> class CBT {
  private:
    struct OCBTTree {
        uint64 numElements = 0;
        uint64 treeSizeBits = 0;
        uint64 numSlots = 0;
        uint64 bitFieldNumSlots = 0;
        uint64 lastLevelSize = 0;
        uint64 lastLevel = 0;
        uint64 firstVirtualLevel = 0;
        uint64 leafLevel = 0;
        StaticArray<uint32, Power> depthOffset;
        StaticArray<uint64, Power> bitMask;
        StaticArray<uint32, Power> bitCount;
    };
    OCBTTree tree;
    constexpr static OCBTTree generateVirtualLevel(OCBTTree tree, uint64 index, uint64 bits) {
        tree.depthOffset[index] = 0;
        tree.bitCount[index] = bits;
        uint64 bitmask = 0;
        for (uint32 i = 0; i < bits; ++i) {
            bitmask |= 1ull << i;
        }
        tree.bitMask[index] = bitmask;
        if (bits == 1) {
            tree.numSlots = tree.treeSizeBits / 32;
            tree.bitFieldNumSlots = tree.numElements / 64;
            return tree;
        }
        return generateVirtualLevel(tree, index++, bits / 2);
    }
    constexpr static OCBTTree generateLevel(OCBTTree tree, uint64 remaining, uint64 index, uint64 levelElements, uint64 levelDimensions) {
        if (remaining == 7) {
            tree.depthOffset[index] = tree.treeSizeBits;
            tree.bitMask[index] = (1ull << 8) - 1;
            tree.bitCount[index] = 8;
            tree.treeSizeBits += levelDimensions * 8;
            tree.lastLevelSize = levelDimensions;
            tree.firstVirtualLevel = std::bit_width(levelDimensions);
            tree.lastLevel = tree.firstVirtualLevel - 1;
            return generateVirtualLevel(tree, index++, 64);
        }
        uint32 levelBits;
        if (levelDimensions < 128) {
            levelBits = 32;
        } else {
            levelBits = 16;
        }
        tree.depthOffset[index] = tree.treeSizeBits;
        tree.bitMask[index] = (1ull << levelBits) - 1;
        tree.bitCount[index] = levelBits;
        tree.treeSizeBits += levelDimensions * levelBits;
        return generateLevel(tree, --remaining, index++, levelElements / 2, levelDimensions * 2);
    }
    constexpr static OCBTTree createTree(uint64 power) {
        uint64 numElements = (1ull << (power - 1));
        return generateLevel(
            CBT::OCBTTree{
                .numElements = numElements,
                .leafLevel = power - 1,
            },
            power - 1, 0, numElements, 1);
    }

  public:
    constexpr CBT() : tree(createTree(Power)) {
        rawMemory = new uint32[tree.numSlots + tree.numElements / 32];
        packedHeap = rawMemory;
        bitfield = (uint64*)(rawMemory + tree.numSlots);
        clear();
    }
    ~CBT() {}

    uint32 numElements() { return tree.numElements; }

    uint32 lastLevelSize() { return tree.lastLevelSize; }

    uint32 maxDepth() { return Power; }

    uint32 numInternalBuffers() { return 2; }

    char* rawBuffer(uint32 bufferIdx = 0) { return bufferIdx == 0 ? (char*)packedHeap : (char*)bitfield; }
    const char* rawBuffer(uint32 bufferIdx = 0) const { return bufferIdx == 0 ? (const char*)packedHeap : (const char*)bitfield; }

    uint32 bufferSize(uint32 bufferIdx = 0) const { return bufferIdx == 0 ? treeMemoryFootprint() : bitFieldMemoryFootprint(); }
    uint32 elementSize(uint32 bufferIdx) const { return bufferIdx == 0 ? sizeof(uint32) : sizeof(uint64); }

    uint32 memoryFootPrint() const { return treeMemoryFootprint() + bitFieldMemoryFootprint(); }
    uint32 treeMemoryFootprint() const { return tree.numSlots * sizeof(uint32); }
    uint32 bitFieldMemoryFootprint() const { return (tree.numElements / 64) * sizeof(uint64); }

    void setBit(uint32 bitID, bool state) {
        // Coordinates of the bit
        uint32_t slot = bitID / 64;
        uint32_t local_id = bitID % 64;

        if (state)
            bitfield[slot] |= 1ull << local_id;
        else
            bitfield[slot] &= ~(1ull << local_id);
    }
    uint32 getBit(uint32 bitID) const {
        uint32_t slot = bitID / 64;
        uint32_t local_id = bitID % 64;
        return (bitfield[slot] & (1ull << local_id)) >> local_id;
    }

    uint32 bitcount() const { return packedHeap[0]; }
    uint32 bitCount(uint32 depth, uint32 element) { return getHeapElement((1 << depth) + element); }

    uint32 decodeBit(uint32 handle) const {
        uint32_t currentDepth = 0;
        uint32_t heapElementID = 1u;
        for (currentDepth = 0; currentDepth < tree.firstVirtualLevel; ++currentDepth) {
            // Read the left element
            uint32_t heapValue = getHeapElement(2u * heapElementID);

            // Does it fall in the right or left subtree?
            uint32_t b = handle < heapValue ? 0u : 1u;

            // Pick a subtree
            heapElementID = 2u * heapElementID + b;

            // Move the iterator to exclude the right subtree if required
            handle -= heapValue * b;
        }

        // Align with the internal depth
        currentDepth++;

        // Ok we have our subtree, now we need to pick the right bit
        uint64_t heapValue = bitfield[heapElementID - tree.lastLevel * 2];

        for (; currentDepth < (tree.leafLevel + 1); ++currentDepth) {
            // Figure out the location of the first bit of this element
            uint32_t real_heap_id = 2 * heapElementID - 1;
            uint32_t level_first_element = (1 << currentDepth) - 1;
            uint32_t id_in_level = real_heap_id - level_first_element;
            uint32_t first_bit = tree.depthOffset[currentDepth] + tree.bitCount[currentDepth] * id_in_level;
            uint32_t local_id = first_bit % 64;
            uint64_t target_bits = (heapValue >> local_id) & tree.bitMask[currentDepth];
            uint32_t heapValue = std::popcount(target_bits);

            // Does it fall in the right or left subtree?
            uint32_t b = handle < heapValue ? 0u : 1u;

            // Pick a subtree
            heapElementID = 2u * heapElementID + b;

            // Move the iterator to exclude the right subtree if required
            handle -= heapValue * b;
        }
        return (heapElementID ^ tree.numElements);
    }
    uint32 decodeBitComplement(uint32 handle) const {
        uint32_t heapElementID = 1u;
        uint32_t c = tree.numElements / 2u;
        uint32_t currentDepth = 0;

        for (currentDepth = 0; currentDepth < tree.firstVirtualLevel; ++currentDepth) {
            uint32_t heapValue = c - getHeapElement(2u * heapElementID);
            uint32_t b = handle < heapValue ? 0u : 1u;

            heapElementID = 2u * heapElementID + b;
            handle -= heapValue * b;
            c /= 2u;
        }

        // Align with the internal depth
        currentDepth++;

        // Ok we have our subtree, now we need to pick the right bit
        uint64_t heapValue = bitfield[heapElementID - tree.lastLevelSize * 2];

        for (; currentDepth < (tree.leafLevel + 1); ++currentDepth) {
            // Figure out the location of the first bit of this element
            uint32_t real_heap_id = 2 * heapElementID - 1;
            uint32_t level_first_element = (1 << currentDepth) - 1;
            uint32_t id_in_level = real_heap_id - level_first_element;
            uint32_t first_bit = tree.depthOffset[currentDepth] + tree.bitCount[currentDepth] * id_in_level;
            uint32_t local_id = first_bit % 64;
            uint64_t target_bits = (heapValue >> local_id) & tree.bitMask[currentDepth];
            uint32_t heapValue = c - countbits(target_bits);

            uint32_t b = handle < heapValue ? 0u : 1u;

            heapElementID = 2u * heapElementID + b;
            handle -= heapValue * b;
            c /= 2u;
        }

        return (heapElementID ^ tree.numElements);
    }

    uint32 getHeapElement(uint32 id) const {
        // Figure out the location of the first bit of this element
        uint32_t real_heap_id = id - 1;
        uint32_t depth = uint32_t(log2(real_heap_id + 1));
        uint32_t level_first_element = (1 << depth) - 1;
        uint32_t id_in_level = real_heap_id - level_first_element;
        uint32_t first_bit = tree.depthOffset[depth] + tree.bitCount[depth] * id_in_level;
        if (depth < tree.firstVirtualLevel) {
            uint32_t slot = first_bit / 32;
            uint32_t local_id = first_bit % 32;
            uint32_t target_bits = (packedHeap[slot] >> local_id) & tree.bitMask[depth];
            return (packedHeap[slot] >> local_id) & tree.bitMask[depth];
        } else {
            uint32_t slot = first_bit / 64;
            uint32_t local_id = first_bit % 64;
            uint64_t target_bits = (bitfield[slot] >> local_id) & tree.bitMask[depth];
            return std::popcount(target_bits);
        }
    }
    uint32 setHeapElement(uint32 id, uint32 value) {
        // Figure out the location of the first bit of this element
        uint32_t real_heap_id = id - 1;
        uint32_t depth = uint32_t(log2(real_heap_id + 1));
        uint32_t level_first_element = (1 << depth) - 1;
        uint32_t id_in_level = real_heap_id - level_first_element;

        // If this is the tree representation
        if (depth < tree.firstVirtualLevel) {
            // Find the slot and the local first bit
            uint32_t first_bit = tree.depthOffset[depth] + tree.bitCount[depth] * id_in_level;
            uint32_t slot = first_bit / 32;
            uint32_t local_id = first_bit % 32;

            // Extract the relevant bits
            uint32_t& target = packedHeap[slot];
            target &= ~(tree.bitMask[depth] << (local_id));
            target |= (tree.bitMask[depth] & value) << (local_id);
        }
        // Should be avoided, but is supported
        else if (depth == tree.leafLevel) {
            setBit(id_in_level, value);
        }
        // Doesn't make sense
        else {
            assert(false);
        }
    }

    void reduce() {
        // First reduce the last level using countbits
        for (uint32_t threadID = 0; threadID < (tree.lastLevelSize / 4); ++threadID) {
            // Initialize the packed sum
            uint32_t packedSum = 0;

            // Loop through the 2 pairs to process
            for (uint32_t pairIdx = 0; pairIdx < 4; ++pairIdx) {
                // First element of the pair
                uint32_t elementC = std::popcount(bitfield[threadID * 8 + 2 * pairIdx]);

                // Second element of the pair
                elementC += std::popcount(bitfield[threadID * 8 + 2 * pairIdx + 1]);

                // Store in the right bits
                packedSum |= (elementC << pairIdx * 8);
            }

            // Offset of the last level of the tree
            const uint32_t bufferOffset = tree.depthOffset[11] / 32;

            // Store the result into the bitfield
            packedHeap[bufferOffset + threadID] = packedSum;
        }

        // Then operate the reduction on the tree only (not the bitfield)
        for (uint32_t size = tree.numElements / 128u; size > 0u; size /= 2u) {
            uint32_t minHeapID = size;
            uint32_t maxHeapID = size * 2u;

            for (uint32_t heapID = minHeapID; heapID < maxHeapID; ++heapID) {
                uint32_t value = getHeapElement(2u * heapID) + getHeapElement(2u * heapID + 1u);
                setHeapElement(heapID, value);
            }
        }
    }
    void clear() {
        memset(packedHeap, 0, tree.numSlots * sizeof(uint32_t));
        memset(bitfield, 0, tree.bitFieldNumSlots * sizeof(uint64_t));
    }

  private:
    uint32* rawMemory;
    uint32* packedHeap;
    uint64* bitfield;
};

struct GeometryCB {
    uint32 totalNumElements;
    uint32 baseDepth;
    uint32 totalNumVertices;
};
struct UpdateCB {
    float triangleSize;
    uint32_t maxSubdivisionDepth;
    float fov;
    float farPlaneDistance;
};
struct CPUMesh {
    uint32 totalNumElements = 0;

    uint32 minimalDepth = 0;

    Array<uint64> heapIDArray;
    Array<UVector4> neighborsArray;

    Array<Vector4> basePoints;
};
CPUMesh generateCPUMesh(uint32 cbtNumElements);
// Pointer to an invalid neighbor or index
constexpr uint64 INVALID_POINTER = UINT32_MAX;

// Possible culling state
constexpr int64 BACK_FACE_CULLED = -3;
constexpr int64 FRUSTUM_CULLED = -2;
constexpr int64 TOO_SMALL = -1;
constexpr int64 UNCHANGED_ELEMENT = 0;
constexpr int64 BISECT_ELEMENT = 1;
constexpr int64 SIMPLIFY_ELEMENT = 2;
constexpr int64 MERGED_ELEMENT = 3;
struct BisectorData {
    UVector indices;

    uint32 subdivisionPattern;
    
    uint32 problematicNeighbor;

    uint32 bisectorState;

    uint32 flags;

    uint32 propagationID;
};
struct LebMatrixCache {
    void init(Gfx::PGraphics graphics, uint32 cacheDepth);
    void release();

    Gfx::PShaderBuffer getLebMatrixBuffer() const { return lebMatrixBuffer; }

  private:
    Gfx::OShaderBuffer lebMatrixBuffer;
    uint32 cacheDepth;
};
struct GPU_CBT {
    uint32 numElements;

    uint32 lastLevelSize = 0;

    uint32 bufferCount = 2;

    Gfx::OShaderBuffer bufferArray[2];
};
struct BaseMesh {
    uint32 numVertices;

    Gfx::OShaderBuffer vertexBuffer;

    uint32 numElements;

    Gfx::OIndexBuffer indexBuffer;
};
struct CBTMesh {
    uint32 totalNumElements;

    uint32 numBaseVertices;

    uint32 baseDepth;

    Gfx::OShaderBuffer heapIDBuffer;
    uint32 currentNeighborsBufferIdx;

    Gfx::OShaderBuffer neighborsBuffers[2];

    Gfx::OShaderBuffer updateBuffer;
    Gfx::OShaderBuffer classificationBuffer;
    Gfx::OShaderBuffer simplificationBuffer;
    Gfx::OShaderBuffer allocateBuffer;
    Gfx::OShaderBuffer propagateBuffer;

    Gfx::OShaderBuffer indirectDrawBuffer;
    Gfx::OShaderBuffer indirectDispatchBuffer;
    Gfx::OShaderBuffer indexedBisectorBuffer;
    Gfx::OShaderBuffer visibleIndexedBisectorBuffer;
    Gfx::OShaderBuffer modifiedIndexedBisectorBuffer;

    Gfx::OShaderBuffer lebVertexBuffer;
    Gfx::OShaderBuffer currentVertexBuffer;
    Gfx::OShaderBuffer currentDisplacementBuffer;

    GPU_CBT gpuCBT;
};
struct MeshUpdater {
    void init(Gfx::PGraphics gfx, Gfx::PDescriptorLayout viewParamsLayout);
    void release();

    void evaluateLeb(const BaseMesh& baseMesh, CBTMesh& mesh, Gfx::PDescriptorSet viewParamsSet,
                     Gfx::PUniformBuffer geometryBuffer, Gfx::PUniformBuffer updateBuffer, Gfx::PShaderBuffer lebMatrixCache, bool clear,
                     bool complete);

    void update(CBTMesh& mesh, Gfx::PDescriptorSet viewParamsSet, Gfx::PUniformBuffer geometryCB, Gfx::PUniformBuffer updateCB);

    void validation(CBTMesh& mesh, Gfx::PUniformBuffer geometryCB);

    void resetBuffers(CBTMesh& mesh);

    void prepareIndirection(CBTMesh& mesh, Gfx::PUniformBuffer geometryCB);

    bool checkIfValid();

    void queryOccupancy(const CBTMesh& mesh);

    uint32 getOccupancy();

    Gfx::PGraphics graphics;

    Gfx::ODescriptorLayout layout;
    Gfx::OPipelineLayout pipelineLayout;

    Gfx::OShaderBuffer indirectBuffer;
    Gfx::OShaderBuffer memoryBuffer;
    Gfx::OShaderBuffer validationBuffer;
    Gfx::OShaderBuffer validationBufferRB;
    Gfx::OShaderBuffer occupancyBufferRB;

    // Main update
    Gfx::OComputeShader resetCS;
    Gfx::PComputePipeline reset;
    Gfx::OComputeShader classifyCS;
    Gfx::PComputePipeline classify;
    Gfx::OComputeShader splitCS;
    Gfx::PComputePipeline split;
    Gfx::OComputeShader prepareIndirectCS;
    Gfx::PComputePipeline prepareIndirect;
    Gfx::OComputeShader allocateCS;
    Gfx::PComputePipeline allocate;
    Gfx::OComputeShader bisectCS;
    Gfx::PComputePipeline bisect;
    Gfx::OComputeShader propagateBisectCS;
    Gfx::PComputePipeline propagateBisect;
    Gfx::OComputeShader prepareSimplifyCS;
    Gfx::PComputePipeline prepareSimplify;
    Gfx::OComputeShader simplifyCS;
    Gfx::PComputePipeline simplify;
    Gfx::OComputeShader propagateSimplifyCS;
    Gfx::PComputePipeline propagateSimplify;

    // Reduction
    Gfx::OComputeShader reducePrePassCS;
    Gfx::PComputePipeline reducePrePass;
    Gfx::OComputeShader reduceFirstPassCS;
    Gfx::PComputePipeline reduceFirstPass;
    Gfx::OComputeShader reduceSecondPassCS;
    Gfx::PComputePipeline reduceSecondPass;

    // Indexation
    Gfx::OComputeShader bisectorIndexationCS;
    Gfx::PComputePipeline bisectorIndexation;
    Gfx::OComputeShader prepareBisectorIndirectCS;
    Gfx::PComputePipeline prepareBisectorIndirect;

    // Debug
    Gfx::OComputeShader validateCS;
    Gfx::PComputePipeline validate;
    Gfx::OShaderBuffer debugBuffer;

    // LEB
    Gfx::OComputeShader lebClearCS;
    Gfx::PComputePipeline lebClear;
    Gfx::OComputeShader lebEvaluateCS;
    Gfx::PComputePipeline lebEvaluate;
};
}; // namespace Seele