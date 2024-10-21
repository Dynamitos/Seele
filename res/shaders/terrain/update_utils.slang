import Common;
import Bisector;
import CBT;
import Parameters;

// Needs to be defined before including update_utilities
struct BisectorGeometry
{
    float3 p[4];
};

// Possible splits
static const uint64_t NO_SPLIT = 0x00;
static const uint64_t CENTER_SPLIT = 0x01;
static const uint64_t RIGHT_SPLIT = 0x02;
static const uint64_t LEFT_SPLIT = 0x04;
static const uint64_t RIGHT_DOUBLE_SPLIT = (CENTER_SPLIT | RIGHT_SPLIT);
static const uint64_t LEFT_DOUBLE_SPLIT = (CENTER_SPLIT | LEFT_SPLIT);
static const uint64_t TRIPLE_SPLIT = (CENTER_SPLIT | RIGHT_SPLIT | LEFT_SPLIT);

// Split buffer slots
static const uint32_t SPLIT_COUNTER = 0;
static const uint32_t SIMPLIFY_COUNTER = 1;
static const uint32_t CLASSIFY_COUNTER_OFFSET = 2;

bool FrustumAABBIntersect(in Frustum frustum, float3 aabbMin, float3 aabbMax)
{
    float3 center = (aabbMax + aabbMin) * 0.5;
    float3 extents = (aabbMax - aabbMin) * 0.5;
    for (int i = 0; i < 4; i++)
    {
        Plane plane = frustum.sides[i];
        float3 normal_sign = sign(plane.n);
        float3 test_point = center + extents * normal_sign;
 
        float dotProd = dot(test_point, plane.n);
        if (dotProd + plane.d < 0)
            return false;
    }
    return true;
}

int ClassifyBisector(in BisectorGeometry tri, uint depth)
{
    // Check the triangle's visibility
    float3 triNormal = normalize(cross(tri.p[2] - tri.p[1], tri.p[0] - tri.p[1]));
    float3 triCenter = (tri.p[0] + tri.p[1] + tri.p[2]) / 3.0;
    float3 viewDir = normalize(-triCenter);
    float FdotV = dot(viewDir, pViewParams.cameraForward_WS.xyz);
    float VdotN = dot(viewDir, triNormal);

    // Here we don't use 0 as it introduces stability issues at grazing angles
    if (FdotV < 0.0 && VdotN < -1e-3)
        return BACK_FACE_CULLED;

    // Compute the triangle's AABB
    float3 aabbMin = float3(min(min(tri.p[0].x, tri.p[1].x), tri.p[2].x), min(min(tri.p[0].y, tri.p[1].y), tri.p[2].y), min(min(tri.p[0].z, tri.p[1].z), tri.p[2].z));
    float3 aabbMax = float3(max(max(tri.p[0].x, tri.p[1].x), tri.p[2].x), max(max(tri.p[0].y, tri.p[1].y), tri.p[2].y), max(max(tri.p[0].z, tri.p[1].z), tri.p[2].z));

    // First we do a frustum culling pass
    if (!FrustumAABBIntersect(pViewParams.viewFrustum, aabbMin, aabbMax))
        return FRUSTUM_CULLED;

    // Project the points on screen
    float4x4 viewProjectionMatrix = pViewParams.viewProjectionMatrix;
    float4 p0P = mul(viewProjectionMatrix, float4(tri.p[0], 1.0));
    p0P.xy = p0P.xy / p0P.w;
    p0P.xy = (p0P.xy * 0.5 + 0.5);

    float4 p1P = mul(viewProjectionMatrix, float4(tri.p[1], 1.0));
    p1P.xy = p1P.xy / p1P.w;
    p1P.xy = (p1P.xy * 0.5 + 0.5);

    float4 p2P = mul(viewProjectionMatrix, float4(tri.p[2], 1.0));
    p2P.xy = p2P.xy / p2P.w;
    p2P.xy = (p2P.xy * 0.5 + 0.5);

    // 2D area of the triangle
    // here we didn't reverse the sign of the y coordinate at projection time, but we simply adapted the area evaluation
    // The same way we don't multiply by the screen size before, but after which is equivalent
    float area = 0.5 * abs(p0P.x * (p2P.y - p1P.y) + p1P.x * (p0P.y - p2P.y) + p2P.x * (p1P.y - p0P.y));
    area *= pViewParams.screenDimensions.x * pViewParams.screenDimensions.y;

    // We over estimate the area at grazing angles
    float areaOverestimation = lerp(2.0, 1.0, pow(VdotN, 0.2));
    area *= areaOverestimation;

    // If the triangle's area is bigger than the target size and the depth is not the maximal depth, subdivide
    if (pParams.update.triangleSize < area && depth < pParams.update.maxSubdivisionDepth)
    {
        // If the area is really big, put it in high priority.
        return BISECT_ELEMENT;
    }
    else if ((pParams.update.triangleSize * 0.5 > area) || (depth > pParams.update.maxSubdivisionDepth))
    {
        // Transform the parent's point
        float4 p3P = mul(viewProjectionMatrix, float4(tri.p[3], 1.0));
        p3P.xy = p3P.xy / p3P.w;
        p3P.xy = (p3P.xy * 0.5 + 0.5);

        // Evaluate the parent area
        float areaParent = 0.5 * abs(p0P.x * (p2P.y - p3P.y) + p3P.x * (p0P.y - p2P.y) + p2P.x * (p3P.y - p0P.y));
        areaParent *= pViewParams.screenDimensions.x * pViewParams.screenDimensions.y;
        areaParent *= areaOverestimation;

        // If the depth is too high (max depth changed) or the area is too 
        return ((pParams.update.triangleSize >= areaParent ) || (depth > pParams.update.maxSubdivisionDepth)) ? TOO_SMALL : UNCHANGED_ELEMENT;
    }
    return UNCHANGED_ELEMENT;
}

void ResetBuffers()
{
    pParams.memoryBuffer[0] = 0;
    pParams.memoryBuffer[1] = cbt_size() - bit_count_buffer();

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

void ClassifyElement(uint currentID, BisectorGeometry bis, uint totalNumElements, uint baseDepth)
{
    // Evaluate the depth of the element
    uint64_t heapID = pParams.heapIDBuffer[currentID];
    uint depth = HeapIDDepth(heapID);
    BisectorData cbisectorData = pParams.bisectorDataBuffer[currentID];

    // Reset some values
    cbisectorData.subdivisionPattern = 0;
    cbisectorData.bisectorState = UNCHANGED_ELEMENT;
    cbisectorData.problematicNeighbor = INVALID_POINTER;
    cbisectorData.flags = VISIBLE_BISECTOR;
    
    // Does this triangle intersect the circle?
    int currentValidity = ClassifyBisector(bis, depth);
    if (currentValidity > UNCHANGED_ELEMENT)
    {
        // This element should be bisected
        uint targetSlot = 0;
        cbisectorData.bisectorState = BISECT_ELEMENT;
        InterlockedAdd(pParams.classificationBuffer[SPLIT_COUNTER], 1, targetSlot);
        pParams.classificationBuffer[CLASSIFY_COUNTER_OFFSET + targetSlot] = currentID;

    }
    else
        cbisectorData.flags = currentValidity >= TOO_SMALL ? VISIBLE_BISECTOR : 0;

    // What's the validity of the father?
    if (baseDepth != depth && currentValidity < UNCHANGED_ELEMENT)
    {   
        // Mark that it requires simplification
        cbisectorData.bisectorState = SIMPLIFY_ELEMENT;

        // Only register it if it has an even heapID, the odd ones will be processed by the even ones
        if (heapID % 2 == 0)
        {
            uint targetSlot = 0;
            InterlockedAdd(pParams.classificationBuffer[SIMPLIFY_COUNTER], 1, targetSlot);
            pParams.classificationBuffer[CLASSIFY_COUNTER_OFFSET + totalNumElements + targetSlot] = currentID;
        }
    }

    // Update the bisector data
    pParams.bisectorDataBuffer[currentID] = cbisectorData;
}

void SplitElement(uint currentID, uint baseDepth)
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
    int usedMemory = 1;
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
    // Add back the unused memory (in case)
    InterlockedAdd(pParams.memoryBuffer[1], maxRequiredMemory - usedMemory, remainingMemory);
}

void AllocateElement(uint currentID)
{
    // Load the bisector for this element
    BisectorData bisectorData = pParams.bisectorDataBuffer[currentID];

    // Does this guy need to be subdivided
    if (bisectorData.subdivisionPattern != 0)
    {
        // How many bits do we need?
        int numSlots = countbits(bisectorData.subdivisionPattern);

        // Request the number of bits we need using an interlock add
        int firstBitIndex = 0;
        InterlockedAdd(pParams.memoryBuffer[0], numSlots, firstBitIndex);

        // llocate the bits we need
        for (uint bitId = 0; bitId < numSlots; ++bitId)
        {
            bisectorData.indices[bitId] = decode_bit_complement(firstBitIndex + bitId);
        }

        // Output 
        pParams.bisectorDataBuffer[currentID] = bisectorData;
    }
}

#define SUBLING0_ID 0
#define SUBLING1_ID 1
#define SUBLING2_ID 2
void evaluate_neighbors(uint currentID, uint bisectorID, out uint resX, out uint resY)
{
    BisectorData nBisectorData = pParams.bisectorDataBuffer[bisectorID];
    uint4 nNeighbors = pParams.neighboursBuffer[bisectorID];
    if (nBisectorData.subdivisionPattern == 0x01)
    {
        resX = nBisectorData.indices[SUBLING0_ID];
        resY = bisectorID;
    }
    else if (nBisectorData.subdivisionPattern == 0x03)
    {
        if (nNeighbors[0] == currentID)
        {
            resX = nBisectorData.indices[SUBLING1_ID];
            resY = bisectorID;
        }
        else
        {
            resX = nBisectorData.indices[SUBLING0_ID];
            resY = nBisectorData.indices[SUBLING1_ID];
        }
    }
    else if (nBisectorData.subdivisionPattern == 0x05)
    {
        if (nNeighbors[1] == currentID)
        {
            resX = nBisectorData.indices[SUBLING1_ID];
            resY = nBisectorData.indices[SUBLING0_ID];
        }
        else
        {
            resX = nBisectorData.indices[SUBLING0_ID];
            resY = bisectorID;
        }
    }
    else
    {
        if (nNeighbors[0] == currentID)
        {
            resX = nBisectorData.indices[SUBLING1_ID];
            resY = bisectorID;
        }
        else if (nNeighbors[1] == currentID)
        {
            resX = nBisectorData.indices[SUBLING2_ID];
            resY = nBisectorData.indices[SUBLING0_ID];
        }
        else
        {
            resX = nBisectorData.indices[SUBLING0_ID];
            resY = nBisectorData.indices[SUBLING1_ID];
        }
    }
}

void BisectElement(uint currentID)
{
    // If this bisector is not allocated or not subdivided, stop right away
    uint64_t baseHeapID = pParams.heapIDBuffer[currentID];
    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];
    if (baseHeapID == 0 || cBisectorData.subdivisionPattern == NO_SPLIT)
        return;

    // Load the bisector data of the target triangle
    uint currentSubdiv = cBisectorData.subdivisionPattern;

    // neighbors of the parent
    uint4 cNeighbors = pParams.neighboursBuffer[currentID];
    uint p_n0 = cNeighbors[0];
    uint p_n1 = cNeighbors[1];
    uint p_n2 = cNeighbors[2];

    // Get the main axis subdiv
    uint siblingID0 = cBisectorData.indices[0];
    uint siblingID1 = cBisectorData.indices[1];
    uint siblingID2 = cBisectorData.indices[2];

    // Simple subdivision (along the main axis)
    if (currentSubdiv == CENTER_SPLIT)
    {   
        uint resX = INVALID_POINTER, resY = INVALID_POINTER;
        if (p_n2 != INVALID_POINTER)
            evaluate_neighbors(currentID, p_n2, resX, resY);

        // Set the heap IDs
        pParams.heapIDBuffer[currentID] = 2 * baseHeapID;
        pParams.heapIDBuffer[siblingID0] = 2 * baseHeapID + 1;

        // Update the neighbors
        uint4 modifiedNeighbors;
        modifiedNeighbors[0] = siblingID0;
        modifiedNeighbors[1] = resX;
        modifiedNeighbors[2] = p_n0;
        pParams.neighboursOutputBuffer[currentID] = modifiedNeighbors;
        modifiedNeighbors[0] = resY;
        modifiedNeighbors[1] = currentID;
        modifiedNeighbors[2] = p_n1;
        pParams.neighboursOutputBuffer[siblingID0] = modifiedNeighbors;

        // Keep track of the parent
        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.indices[0] = cBisectorData.indices[0];
        modifiedBisector.indices[1] = cBisectorData.indices[1];
        modifiedBisector.indices[2] = cBisectorData.indices[2];
        modifiedBisector.subdivisionPattern = cBisectorData.subdivisionPattern;
        modifiedBisector.bisectorState = cBisectorData.bisectorState;
        modifiedBisector.propagationID = currentID;

        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[currentID] = modifiedBisector;

        modifiedBisector.problematicNeighbor = p_n1;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID0] = modifiedBisector;

        // Mark this for propagation
        uint targetLocation = 0;
        InterlockedAdd(pParams.propagateBuffer[0], 1, targetLocation);
        pParams.propagateBuffer[2 + targetLocation] = siblingID0;
    }
    else if (currentSubdiv == RIGHT_DOUBLE_SPLIT)
    {
        // Grab the bisector of the twin
        uint res0X = INVALID_POINTER, res0Y = INVALID_POINTER;
        evaluate_neighbors(currentID, p_n0, res0X, res0Y);

        uint res1X = INVALID_POINTER, res1Y = INVALID_POINTER;
        if (p_n2 != INVALID_POINTER)
            evaluate_neighbors(currentID, p_n2, res1X, res1Y);

        // Set the heap IDs
        pParams.heapIDBuffer[currentID] = 4 * baseHeapID;
        pParams.heapIDBuffer[siblingID0] = 2 * baseHeapID + 1;
        pParams.heapIDBuffer[siblingID1] = 4 * baseHeapID + 1;

        uint4 modifiedNeighbors;
        modifiedNeighbors[0] = siblingID1;
        modifiedNeighbors[1] = res0X;
        modifiedNeighbors[2] = siblingID0;
        pParams.neighboursOutputBuffer[currentID] = modifiedNeighbors;
        modifiedNeighbors[0] = res1Y;
        modifiedNeighbors[1] = currentID;
        modifiedNeighbors[2] = p_n1;
        pParams.neighboursOutputBuffer[siblingID0] = modifiedNeighbors;
        modifiedNeighbors[0] = res0Y;
        modifiedNeighbors[1] = currentID;
        modifiedNeighbors[2] = res1X;
        pParams.neighboursOutputBuffer[siblingID1] = modifiedNeighbors;

        // Keep track of the parent
        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.indices[0] = cBisectorData.indices[0];
        modifiedBisector.indices[1] = cBisectorData.indices[1];
        modifiedBisector.indices[2] = cBisectorData.indices[2];
        modifiedBisector.subdivisionPattern = cBisectorData.subdivisionPattern;
        modifiedBisector.bisectorState = cBisectorData.bisectorState;
        modifiedBisector.propagationID = currentID;

        // Lower the element down the tree and update it's sibling
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[currentID] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = p_n1;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID0] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID1] = modifiedBisector;

        // Mark this for propagation
        uint targetLocation = 0;
        InterlockedAdd(pParams.propagateBuffer[0], 1, targetLocation);
        pParams.propagateBuffer[2 + targetLocation] = siblingID0;
    }
    else if (currentSubdiv == LEFT_DOUBLE_SPLIT)
    {
        // Grab the bisector of the twin
        uint res0X = INVALID_POINTER, res0Y = INVALID_POINTER;
        evaluate_neighbors(currentID, p_n1, res0X, res0Y);

        uint res1X = INVALID_POINTER, res1Y = INVALID_POINTER;
        if (p_n2 != INVALID_POINTER)
            evaluate_neighbors(currentID, p_n2, res1X, res1Y);

        // Set the heap IDs
        pParams.heapIDBuffer[currentID] = 2 * baseHeapID;
        pParams.heapIDBuffer[siblingID0] = 4 * baseHeapID + 2;
        pParams.heapIDBuffer[siblingID1] = 4 * baseHeapID + 3;

        uint4 modifiedNeighbors;
        modifiedNeighbors[0] = siblingID1;
        modifiedNeighbors[1] = res1X;
        modifiedNeighbors[2] = p_n0;
        pParams.neighboursOutputBuffer[currentID] = modifiedNeighbors;
        modifiedNeighbors[0] = siblingID1;
        modifiedNeighbors[1] = res0X;
        modifiedNeighbors[2] = res1Y;
        pParams.neighboursOutputBuffer[siblingID0] = modifiedNeighbors;
        modifiedNeighbors[0] = res0Y;
        modifiedNeighbors[1] = siblingID0;
        modifiedNeighbors[2] = currentID;
        pParams.neighboursOutputBuffer[siblingID1] = modifiedNeighbors;

        // Keep track of the parent
        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.indices[0] = cBisectorData.indices[0];
        modifiedBisector.indices[1] = cBisectorData.indices[1];
        modifiedBisector.indices[2] = cBisectorData.indices[2];
        modifiedBisector.subdivisionPattern = cBisectorData.subdivisionPattern;
        modifiedBisector.bisectorState = cBisectorData.bisectorState;
        modifiedBisector.propagationID = currentID;

        // Lower the element down the tree and update it's sibling
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[currentID] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID0] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID1] = modifiedBisector;
    }
    else if (currentSubdiv == TRIPLE_SPLIT)
    {
        // Grab the bisector of the twin
        uint res0X = INVALID_POINTER, res0Y = INVALID_POINTER;
        evaluate_neighbors(currentID, p_n0, res0X, res0Y);

        uint res1X = INVALID_POINTER, res1Y = INVALID_POINTER;
        evaluate_neighbors(currentID, p_n1, res1X, res1Y);

        uint res2X = INVALID_POINTER, res2Y = INVALID_POINTER;
        if (p_n2 != INVALID_POINTER)
            evaluate_neighbors(currentID, p_n2, res2X, res2Y);

        // Set the heap IDs
        pParams.heapIDBuffer[currentID] = 4 * baseHeapID;
        pParams.heapIDBuffer[siblingID0] = 4 * baseHeapID + 2;
        pParams.heapIDBuffer[siblingID1] = 4 * baseHeapID + 1;
        pParams.heapIDBuffer[siblingID2] = 4 * baseHeapID + 3;

        uint4 modifiedNeighbors;
        modifiedNeighbors[0] = siblingID1;
        modifiedNeighbors[1] = res0X;
        modifiedNeighbors[2] = siblingID2;
        pParams.neighboursOutputBuffer[currentID] = modifiedNeighbors;
        modifiedNeighbors[0] = siblingID2;
        modifiedNeighbors[1] = res1X;
        modifiedNeighbors[2] = res2Y;
        pParams.neighboursOutputBuffer[siblingID0] = modifiedNeighbors;
        modifiedNeighbors[0] = res0Y;
        modifiedNeighbors[1] = currentID;
        modifiedNeighbors[2] = res2X;
        pParams.neighboursOutputBuffer[siblingID1] = modifiedNeighbors;
        modifiedNeighbors[0] = res1Y;
        modifiedNeighbors[1] = siblingID0;
        modifiedNeighbors[2] = currentID;
        pParams.neighboursOutputBuffer[siblingID2] = modifiedNeighbors;

        // Keep track of the parent
        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.indices[0] = cBisectorData.indices[0];
        modifiedBisector.indices[1] = cBisectorData.indices[1];
        modifiedBisector.indices[2] = cBisectorData.indices[2];
        modifiedBisector.subdivisionPattern = cBisectorData.subdivisionPattern;
        modifiedBisector.bisectorState = cBisectorData.bisectorState;
        modifiedBisector.propagationID = currentID;

        // Lower the element down the tree and update it's sibling
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[currentID] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID0] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID1] = modifiedBisector;

        // Create the sibling of the current element
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID2] = modifiedBisector;
    }

    // How many bits do we need to raise
    uint numSiblings = countbits(currentSubdiv);
    for (uint siblingIdx = 0; siblingIdx < numSiblings; ++siblingIdx)
    {
        set_bit_atomic(cBisectorData.indices[siblingIdx], true);
    }
}

void PropagateBisectElement(uint currentID)
{
    // Load the bisector data of the target triangle
    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];

    // neighbors of the parent
    uint parentID = cBisectorData.propagationID;
    uint problematicNeighbor = cBisectorData.problematicNeighbor;

    // Read the neighbor that may have changed
    BisectorData tBisectorData = pParams.bisectorDataBuffer[problematicNeighbor];
    uint3 tNeighbors = pParams.neighboursBuffer[problematicNeighbor].xyz;
    uint targetID = problematicNeighbor;
    uint sibling1 = tBisectorData.indices[1];

    if (tBisectorData.subdivisionPattern == NO_SPLIT)
    {
        if (tNeighbors[0] == parentID)
            pParams.neighboursBuffer[targetID][0] = currentID;
        if (tNeighbors[1] == parentID)
            pParams.neighboursBuffer[targetID][1] = currentID;
        if (tNeighbors[2] == parentID)
            pParams.neighboursBuffer[targetID][2] = currentID;
    }
    else if (tBisectorData.subdivisionPattern == CENTER_SPLIT)
    {
        if (pParams.neighboursBuffer[targetID][2] == parentID)
            pParams.neighboursBuffer[targetID][2] = currentID;
        if (pParams.neighboursBuffer[tBisectorData.propagationID][2] == parentID)
            pParams.neighboursBuffer[tBisectorData.propagationID][2] = currentID;
    }
    else if (tBisectorData.subdivisionPattern == RIGHT_DOUBLE_SPLIT)
    {
        pParams.neighboursBuffer[sibling1][2] = currentID;
    } 
    else if (tBisectorData.subdivisionPattern == LEFT_DOUBLE_SPLIT)
    {
        pParams.neighboursBuffer[targetID][2] = currentID;
    }

    // Reset the problematic neighbor and the bisection state
    cBisectorData.problematicNeighbor = INVALID_POINTER;
    cBisectorData.bisectorState = UNCHANGED_ELEMENT;
    pParams.bisectorDataBuffer[currentID] = cBisectorData;
}

void PrepareSimplifyElement(uint currentID)
{
    // Get the bisector
    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];

    // Grab the current bisector
    uint64_t cHeapID = pParams.heapIDBuffer[currentID];

    // If this is not an even heap number it will be handeled by it's pair, the twin or the twin's pair

    // Neighbors of this element
    uint3 cNeighbors = pParams.neighboursBuffer[currentID].xyz;

    // Evaluate the depth of this bisector
    uint currentDepth = HeapIDDepth(cHeapID);

    // Grab the pair neighbor (it has to exist)
    uint pairID = cNeighbors[0];
    uint64_t pHeapID = pParams.heapIDBuffer[pairID];
    BisectorData pBisectorData = pParams.bisectorDataBuffer[pairID];
    uint3 pNeighbors = pParams.neighboursBuffer[pairID].xyz;

    // Evaluate the depth of the pair
    uint pairDepth = HeapIDDepth(pHeapID);

    // If they are not at the same depth or the pair is not to be simplified, we're done
    if (pairDepth != currentDepth || pBisectorData.bisectorState != SIMPLIFY_ELEMENT)
        return;

    // We need to identify our twin pair
    uint twinLowID = pNeighbors[0];
    uint twinHighID = cNeighbors[1];
    if (twinLowID != INVALID_POINTER)
    {
        // Grab the two bisectors
        uint64_t twinLowHeapID = pParams.heapIDBuffer[twinLowID];
        uint64_t twinHighHeapID = pParams.heapIDBuffer[twinHighID];
        
        // The current bisector is not the smallest element of the neighborhood, he will be handeled by twinLowBisect if needed
        if (cHeapID > twinLowHeapID)
            return;

        // Compute the depth of both neighbors
        uint lowFacingDepth = HeapIDDepth(twinLowHeapID);
        uint highFacingDepth = HeapIDDepth(twinHighHeapID);

        // If all four elements are not on the same
        if (lowFacingDepth != currentDepth || highFacingDepth != currentDepth)
            return;

        // Grab the two bisectors
        BisectorData twinLowBisectData = pParams.bisectorDataBuffer[twinLowID];
        BisectorData twinHighBisectData = pParams.bisectorDataBuffer[twinHighID];

        // This element should not be doing the simplifications if:
        // - One of the four elements doesn't have the same depth
        // - One of the four elements isn't flagged for simplification
        if (twinLowBisectData.bisectorState != SIMPLIFY_ELEMENT
            || twinHighBisectData.bisectorState != SIMPLIFY_ELEMENT)
            return;
    }

    // This element will simplify itself, it's pair and possibilty it's twin and twin-pair.
    uint bisectorSlot;
    InterlockedAdd(pParams.simplifyBuffer[0], 1, bisectorSlot);

    // Log the bisector ID
    pParams.simplifyBuffer[1 + bisectorSlot] = currentID;
}

void SimplifyElement(uint currentID)
{
    // Grab the current bisector
    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];
    uint3 cNeighbors = pParams.neighboursBuffer[currentID].xyz;

    // Grab the pair neighbor (it has to exist)
    uint pairID = cNeighbors[0];
    BisectorData pBisectorData = pParams.bisectorDataBuffer[pairID];
    uint3 pNeighbors = pParams.neighboursBuffer[pairID].xyz;

    // We need to indentify our twin pair
    uint twinLowID = pNeighbors[0];
    uint twinHighID = cNeighbors[1];

    // Set the heap IDs
    pParams.heapIDBuffer[currentID] = pParams.heapIDBuffer[currentID] / 2;
    pParams.heapIDBuffer[pairID] = 0;

    // All conditions are met for us to simplify these triangles
    uint3 newNeighbors;
    newNeighbors[0] = cNeighbors[2];
    newNeighbors[1] = pNeighbors[2];
    newNeighbors[2] = twinLowID;
    pParams.neighboursBuffer[currentID] = uint4(newNeighbors, 0);

    // Update the bisector data
    cBisectorData.propagationID = pairID;
    cBisectorData.problematicNeighbor = pNeighbors[2];
    cBisectorData.bisectorState = MERGED_ELEMENT;
    cBisectorData.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
    pParams.bisectorDataBuffer[currentID] = cBisectorData;

    // Mark this for propagation
    if (cBisectorData.problematicNeighbor != INVALID_POINTER)
    {
        // Mark this for propagation
        uint targetLocation = 0;
        InterlockedAdd(pParams.propagateBuffer[1], 1, targetLocation);
        pParams.propagateBuffer[2 + targetLocation] = currentID;
    }

    // Clear the pair's heap for identification
    pBisectorData.bisectorState = MERGED_ELEMENT;
    pBisectorData.flags = 0;
    pParams.bisectorDataBuffer[pairID] = pBisectorData;

    // Don't forget to free the bit
    set_bit_atomic_buffer(pairID, false);

    // If there was a facing pair, simplify it aswell
    if (twinLowID != INVALID_POINTER)
    {
        // Set the heap IDs
        pParams.heapIDBuffer[twinLowID] = pParams.heapIDBuffer[twinLowID] / 2;
        pParams.heapIDBuffer[twinHighID] = 0;

        // Read both bisectors
        BisectorData lowFacingBst = pParams.bisectorDataBuffer[twinLowID];
        uint3 lfNeighbors = pParams.neighboursBuffer[twinLowID].xyz;
        BisectorData highFacingBst = pParams.bisectorDataBuffer[twinHighID];
        uint3 hfNeighbors = pParams.neighboursBuffer[twinHighID].xyz;

        // Update the lowest ID
        newNeighbors[0] = lfNeighbors[2];
        newNeighbors[1] = hfNeighbors[2];
        newNeighbors[2] = currentID;
        pParams.neighboursBuffer[twinLowID] = uint4(newNeighbors, 0);

        // Update the twin bisector data
        lowFacingBst.propagationID = twinHighID;
        lowFacingBst.problematicNeighbor = hfNeighbors[2];
        lowFacingBst.bisectorState = MERGED_ELEMENT;
        lowFacingBst.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[twinLowID] = lowFacingBst;

        if (lowFacingBst.problematicNeighbor != INVALID_POINTER)
        {
            // Mark this for propagation
            uint targetLocation = 0;
            InterlockedAdd(pParams.propagateBuffer[1], 1, targetLocation);
            pParams.propagateBuffer[2 + targetLocation] = twinLowID;
        }

        // Clear the pair's heap for identification 
        highFacingBst.bisectorState = MERGED_ELEMENT;
        highFacingBst.flags = 0;
        pParams.bisectorDataBuffer[twinHighID] = highFacingBst;

        // Don't forget to free the bit
        set_bit_atomic_buffer(twinHighID, false);
    }
}

void PropagateElementSimplify(uint currentID)
{
    // Load the bisector data of the target element
    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];

    // Id of the element before the simplification
    uint deletedPair = cBisectorData.propagationID;

    // neighbors of the parent
    uint neighborID = cBisectorData.problematicNeighbor;

    // Read the neighbor that may have changed
    BisectorData nBisectorData = pParams.bisectorDataBuffer[neighborID];
    uint3 nNeighbors = pParams.neighboursBuffer[neighborID].xyz;

    // The neighbor has not changed, so we just need to make it point on currentID instead of the pair that was deleted
    if (nBisectorData.bisectorState != MERGED_ELEMENT)
    {
        for (uint i = 0; i < 3; ++i)
        {
            if (nNeighbors[i] == deletedPair)
                pParams.neighboursBuffer[neighborID][i] = currentID;
        }
    }
    // The neighbor has had a simplification, so we need to update a different neighbor based on if it went up one depth in the tree or was deleted.
    else if (nBisectorData.bisectorState == MERGED_ELEMENT)
    {
        // He still exist, but was simplified
        if (pParams.heapIDBuffer[neighborID] != 0)
        {
            for (uint i = 0; i < 3; ++i)
            {
                if (nNeighbors[i] == deletedPair)
                    pParams.neighboursBuffer[neighborID][i] = currentID;
            }
        }
        // He is gone, we need to update his pair instead of him.
        else
        {
            uint neighborPair = nNeighbors[1];
            for (uint i = 0; i < 3; ++i)
            {
                if (pParams.neighboursBuffer[neighborPair][i] == deletedPair)
                    pParams.neighboursBuffer[neighborPair][i] = currentID;
            }
        }
    }

    // Reset the problematic neighbor
    pParams.bisectorDataBuffer[currentID].problematicNeighbor = INVALID_POINTER;
}

void BisectorElementIndexation(uint currentID)
{
    // Grab the current heap ID
    uint64_t cHeapID = pParams.heapIDBuffer[currentID];

    // Deallocated element, we don't care
    if (cHeapID == 0)
        return;

    // Reserve a slot for this bisector
    uint bisectorSlot;
    InterlockedAdd(pParams.indirectDrawBuffer[0], 3, bisectorSlot);

    // Keep track of it's global ID
    pParams.bisectorIndicesBuffer[bisectorSlot / 3] = currentID;

    // Load the bisector data of the target element
    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];
    
    // Is it visible?
    if ((cBisectorData.flags & VISIBLE_BISECTOR) == 0)
        return;

    // Reserve a slot for this visible bisector
    InterlockedAdd(pParams.indirectDrawBuffer[4], 3, bisectorSlot);

    // Keep track of it's global ID
    pParams.visibleBisectorIndices[bisectorSlot / 3] = currentID;

    // Is it visible?
    if ((cBisectorData.flags & MODIFIED_BISECTOR) == 0)
        return;

    // Reserve a slot for this visible bisector
    InterlockedAdd(pParams.indirectDrawBuffer[8], 4, bisectorSlot);

    // Keep track of it's global ID
    pParams.modifiedBisectorIndices[bisectorSlot / 4] = currentID;
}

void ValidateBisector(uint currentID)
{
    // Grab the current heap ID
    uint64_t cHeapID = pParams.heapIDBuffer[currentID];

    // Deallocated element, we don't care
    if (cHeapID == 0)
        return;

    // Load the bisector data of the target element
    uint4 cNeighbors = pParams.neighboursBuffer[currentID];
    uint4 tempnNeighbours[3];

    bool failed = false;
    uint targetNeighbor = INVALID_POINTER;
    uint targetIdx = INVALID_POINTER;
    for (uint i = 0; i < 3; ++i)
    {
        uint neighborID = cNeighbors[i];
        if (neighborID != INVALID_POINTER)
        {
            bool found = false;
            uint4 nNeighbors = pParams.neighboursBuffer[neighborID];
            tempnNeighbours[i] = nNeighbors;
            for (uint j = 0; j < 3; ++j)
            {
                if (nNeighbors[j] == currentID)
                    found = true;
            }
            if (!found)
            {
                failed = true;
                targetNeighbor = neighborID;
                targetIdx = i;
                break;
            }
        }
    }
    
    // Notify the failure
    if (failed)
    {
        uint prevValue;
        InterlockedAdd(pParams.validationBuffer[0], 1, prevValue);
    }    
}