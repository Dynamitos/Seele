import Parameters;
import CBT;
import Bisector;

static const uint32_t WORKGROUP_SIZE = 64;

    // Resolution of the cache
    #define LEB_TABLE_DEPTH 5ULL

    // Cache in shared memory
    groupshared float3x3 g_MatrixCache[2ULL << LEB_TABLE_DEPTH];

    void load_leb_matrix_cache_to_shared_memory(uint groupIndex)
    {
        if (groupIndex < (2ULL << LEB_TABLE_DEPTH))
                g_MatrixCache[groupIndex] = pParams.lebMatrixCache[groupIndex];
        GroupMemoryBarrierWithGroupSync();
    }
uint leb_depth(uint64_t heapID)
{
    uint depth = 0;
    while (heapID > 0u)
    {
        ++depth;
        heapID >>= 1u;
    }
    return depth - 1;
}

/*******************************************************************************
 * GetBitValue -- Returns the value of a bit stored in a 64-bit word
 *
 */

uint64_t leb__GetBitValue(uint64_t bitField, int64_t bitID)
{
    return ((bitField >> bitID) & 1L);
}

/*******************************************************************************
 * IdentityMatrix3x3 -- Sets a 3x3 matrix to identity
 *
 */

void leb__IdentityMatrix3x3(out float3x3 m)
{
    m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
}

/*******************************************************************************
 * SplittingMatrix -- Computes a LEB splitting matrix from a split bit
 *
 */

void leb__SplittingMatrix(inout float3x3 mat, uint64_t bitValue)
{
    float b = (float)bitValue;
    float c = 1.0f - b;
    const float3x3 splitMatrix = {
        {0.0f, b, c},
        {0.5f, 0.0f, 0.5f},
        {b, c, 0.0f}
    };
    mat = mul(splitMatrix, mat);
}

/*******************************************************************************
 * SplittingMatrix -- Computes a LEB splitting matrix from a split bit
 *
 */

float3x3 leb__SplittingMatrix_out(float3x3 mat, uint64_t bitValue)
{
    float b = (float)bitValue;
    float c = 1.0 - b;
    float3x3 splitMatrix = {
        {0.0, b, c},
        {0.5, 0.0, 0.5},
        {b, c, 0.0}
    };
    return mul(splitMatrix, mat);
}

/*******************************************************************************
 * DecodeTransformationMatrix -- Computes the matrix associated to a LEB
 * node
 *
*/

void leb__DecodeTransformationMatrix(uint64_t heapID, out float3x3 mat)
{
    int depth = leb_depth(heapID);
    leb__IdentityMatrix3x3(mat);
    for (int bitID = depth - 1; bitID >= 0; --bitID)
        leb__SplittingMatrix(mat, leb__GetBitValue(heapID, bitID));
}

#if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
void leb__DecodeTransformationMatrix_Tabulated(uint64_t heapID, out float3x3 mat)
{
    leb__IdentityMatrix3x3(mat);
    const uint64_t msb = (1ULL << LEB_TABLE_DEPTH);
    const uint64_t mask = ~(~0ULL << LEB_TABLE_DEPTH);
    while (heapID > mask)
    {
        uint32_t index = uint32_t((heapID & mask) | msb);
        mat = mul(mat, g_MatrixCache[index]);
        heapID >>= LEB_TABLE_DEPTH;
    }
    mat = mul(mat, g_MatrixCache[uint32_t(heapID)]);
}
#endif

/*******************************************************************************
 * DecodeTransformationMatrix -- Computes the matrix associated to a LEB
 * node
 *
*/

void leb__DecodeTransformationMatrix_parent_child(uint64_t heapID, out float3x3 parent, out float3x3 child)
{
    int depth = leb_depth(heapID);
    leb__IdentityMatrix3x3(parent);

    // Evaluate the parent matrix
    int bitID;
    for (bitID = depth - 1; bitID > 0; --bitID)
        leb__SplittingMatrix(parent, leb__GetBitValue(heapID, bitID));

    // Evaluate the child
    if (depth > 0)
        child = leb__SplittingMatrix_out(parent, leb__GetBitValue(heapID, bitID));
    else
        child = parent;
}

#if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
void leb__DecodeTransformationMatrix_parent_child_Tabulated(uint64_t heapID, out float3x3 parent, out float3x3 child)
{
    int depth = leb_depth(heapID);
    leb__IdentityMatrix3x3(parent);
    const uint64_t msb = (1ULL << LEB_TABLE_DEPTH);
    const uint64_t mask = ~(~0ULL << LEB_TABLE_DEPTH);
    uint64_t parentHeapID = heapID / 2;
    while (parentHeapID > mask)
    {
        uint32_t index = uint32_t((parentHeapID & mask) | msb);
        parent = mul(parent, g_MatrixCache[index]);
        parentHeapID >>= LEB_TABLE_DEPTH;
    }
    if (parentHeapID != 0)
        parent = mul(parent, g_MatrixCache[uint32_t(parentHeapID)]);

    // Evaluate the child
    if (depth > 0)
        child = leb__SplittingMatrix_out(parent, leb__GetBitValue(heapID, 0));
    else
        child = parent;
}

#endif

/*******************************************************************************
 * DecodeNodeAttributeArray -- Compute the triangle attributes at the input node
 *
 */

void leb_DecodeNodeAttributeArray(uint64_t heapID, inout float3 attributeArray[2])
{
    float3x3 m;
    leb__DecodeTransformationMatrix(heapID, m);
    for (int i = 0; i < 2; ++i)
    {
        float3 attributeVector = attributeArray[i];
        attributeArray[i][0] = dot(m[0], attributeVector);
        attributeArray[i][1] = dot(m[1], attributeVector);
        attributeArray[i][2] = dot(m[2], attributeVector);
    }
}

void leb_DecodeNodeAttributeArray(uint64_t heapID, inout float3 attributeArray[3])
{
    float3x3 m;
#if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
    leb__DecodeTransformationMatrix_Tabulated(heapID, m);
#else
    leb__DecodeTransformationMatrix(heapID, m);
#endif
    for (int i = 0; i < 3; ++i)
    {
        float3 attributeVector = attributeArray[i];
        attributeArray[i][0] = dot(m[0], attributeVector);
        attributeArray[i][1] = dot(m[1], attributeVector);
        attributeArray[i][2] = dot(m[2], attributeVector);
    }
}

/*******************************************************************************
 * DecodeNodeAttributeArray -- Compute the triangle attributes at the input node
 *
 */

void leb_DecodeNodeAttributeArray_parent_child(uint64_t heapID, inout float3 childAttribute[3], out float3 parentAttribute[3])
{
    float3x3 child, parent;
#if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
    leb__DecodeTransformationMatrix_parent_child_Tabulated(heapID, parent, child);
#else
    leb__DecodeTransformationMatrix_parent_child(heapID, parent, child);
#endif
    int i;
    for (i = 0; i < 3; ++i)
    {
        float3 attributeVector = childAttribute[i];
        parentAttribute[i][0] = dot(parent[0], attributeVector);
        parentAttribute[i][1] = dot(parent[1], attributeVector);
        parentAttribute[i][2] = dot(parent[2], attributeVector);
    }

    for (i = 0; i < 3; ++i)
    {
        float3 attributeVector = childAttribute[i];
        childAttribute[i][0] = dot(child[0], attributeVector);
        childAttribute[i][1] = dot(child[1], attributeVector);
        childAttribute[i][2] = dot(child[2], attributeVector);
    }
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void ClearBuffer(uint currentID : SV_DispatchThreadID)
{
    // This thread doesn't have any work to do, we're done
    if (currentID >= pParams.geometry.totalNumVertices)
        return;
    pParams.lebPositionBuffer[currentID] = float4(0.0, 0.0, 0.0, 1.0);
}

[[vk::push_constant]]
ConstantBuffer<uint> preRendering;
struct Triangle
{
    float3 p[3];
};

void EvaluateElementPosition(uint64_t heapID, uint32_t vertexDataOffset, uint minDepth, RWStructuredBuffer<float4> vertexBuffer, out Triangle parentTri, out Triangle childTri)
{
    // Get the depth of the element
    uint depth = HeapIDDepth(heapID);

    // Compute the required shift to find the original vertices
    uint64_t subTreeDepth = depth - minDepth;

    // Compute the base heapID
    uint64_t baseHeapID = 1u << (minDepth - 1);
    uint primitiveID = uint((heapID >> subTreeDepth) - baseHeapID);

    // Grab the base positions of the element
    float3 p0 = float3(vertexBuffer[3 * primitiveID + vertexDataOffset].xyz);
    float3 p1 = float3(vertexBuffer[3 * primitiveID + 1 + vertexDataOffset].xyz);
    float3 p2 = float3(vertexBuffer[3 * primitiveID + 2 + vertexDataOffset].xyz);

    // Heap ID in the sub triangle
    uint64_t mask = subTreeDepth != 0uL ? 0xFFFFFFFFFFFFFFFFull >> (64ull - subTreeDepth) : 0ull;
    uint64_t baseHeap = (1ull << subTreeDepth);
    uint64_t baseMask = (mask & heapID);
    uint64_t subHeapID = baseMask + baseHeap;

    // Generate the triangle positions
    float3 childArray[3] = {{p0.x, p1.x, p2.x}, {p0.y, p1.y, p2.y}, {p0.z, p1.z, p2.z}};
    float3 parentArray[3];

    // Decode
    leb_DecodeNodeAttributeArray_parent_child(subHeapID, childArray, parentArray);

    // Fill the parent triangle
    parentTri.p[0] = float3(parentArray[0][0], parentArray[1][0], parentArray[2][0]);
    parentTri.p[1] = float3(parentArray[0][1], parentArray[1][1], parentArray[2][1]);
    parentTri.p[2] = float3(parentArray[0][2], parentArray[1][2], parentArray[2][2]);

    // Fill the child triangle
    Triangle child;
    childTri.p[0] = float3(childArray[0][0], childArray[1][0], childArray[2][0]);
    childTri.p[1] = float3(childArray[0][1], childArray[1][1], childArray[2][1]);
    childTri.p[2] = float3(childArray[0][2], childArray[1][2], childArray[2][2]);
}
[numthreads(WORKGROUP_SIZE, 1, 1)]
void EvaluateLEB(uint currentID : SV_DispatchThreadID, uint groupIndex: SV_GroupIndex)
{
    #if defined(LEB_MATRIX_CACHE_BINDING_SLOT)
    // Make sure these are loaded to the shared memory
    load_leb_matrix_cache_to_shared_memory(groupIndex);
    #endif

    // This thread doesn't have any work to do, we're done
    uint32_t numBisectors = bool(preRendering) ? (pParams.indirectDrawBuffer[9]) : (pParams.indirectDrawBuffer[8] / 4);
    if (currentID >= numBisectors)
        return;

    // Load the bisector for this element
    currentID = pParams.indexedBisectorBuffer[currentID];

    // Evaluate the depth of the element
    uint64_t cHeapID = pParams.heapIDBuffer[currentID];
    uint depth = HeapIDDepth(cHeapID);

    // Evaluate the positions of the current element
    Triangle parentTri, childTri;
    EvaluateElementPosition(cHeapID, 0, pParams.geometry.baseDepth, pParams.currentVertexBuffer, parentTri, childTri);

    // Export the child
    pParams.lebPositionBuffer[3 * currentID + 0] = float4(normalize(childTri.p[0]), 1.0f);
    pParams.lebPositionBuffer[3 * currentID + 1] = float4(normalize(childTri.p[1]), 1.0f);
    pParams.lebPositionBuffer[3 * currentID + 2] = float4(normalize(childTri.p[2]), 1.0f);

    // Export the fourth element
    if (pParams.geometry.baseDepth < depth)
    {
        // Offset for the parent buffer
        const uint parentOffset = 3 * pParams.geometry.totalNumElements;

        // Transform the coordinate to planet space
        pParams.lebPositionBuffer[parentOffset + currentID] = float4(normalize(cHeapID % 2 == 0 ? parentTri.p[0] : parentTri.p[2]), 1.0f);
    }
}
