import Common;
import Parameters;
import Bisector;
import Bounding;
import CBT;

#ifndef WORKGROUP_SIZE
#define WORKGROUP_SIZE 64
#endif


struct BisectorGeometry
{
	float3 p[4];
}

// global
// update
int classifyBisector(in BisectorGeometry tri, uint depth, inout DebugStruct debug)
{
	float3 triNormal = normalize(cross(tri.p[2] - tri.p[1], tri.p[0] - tri.p[1]));
	float3 triCenter = (tri.p[0] + tri.p[1] + tri.p[2]) / 3.0;
	float3 viewDir = normalize(-triCenter);
	float fDotV = dot(viewDir, pViewParams.cameraForward_WS.xyz);
	float vDotN = dot(viewDir, triNormal);

    debug.fDotV = fDotV;
    debug.vDotN = vDotN;
	if(fDotV < 0.0 && vDotN < -1e-3)
    {
        debug.area = 420;
		return BACK_FACE_CULLED;
    }


    AABB aabb;
	aabb.minCorner = float3(min(min(tri.p[0].x, tri.p[1].x), tri.p[2].x), min(min(tri.p[0].y, tri.p[1].y), tri.p[2].y), min(min(tri.p[0].z, tri.p[1].z), tri.p[2].z));
    aabb.maxCorner = float3(max(max(tri.p[0].x, tri.p[1].x), tri.p[2].x), max(max(tri.p[0].y, tri.p[1].y), tri.p[2].y), max(max(tri.p[0].z, tri.p[1].z), tri.p[2].z));
	
	//if(aabb.insideFrustum(pViewParams.viewFrustum))
    //    return FRUSTUM_CULLED;

    float4x4 viewProjectionMatrix = mul(pViewParams.projectionMatrix, pViewParams.viewMatrix);

    float4 p0P = clipToScreen(mul(viewProjectionMatrix, float4(tri.p[0], 1.0)));

    float4 p1P = clipToScreen(mul(viewProjectionMatrix, float4(tri.p[1], 1.0)));
    
    float4 p2P = clipToScreen(mul(viewProjectionMatrix, float4(tri.p[2], 1.0)));
    
    float area = 0.5 * abs(p0P.x * (p1P.y - p2P.y) + p1P.x * (p2P.y - p0P.y) + p2P.x * (p0P.y - p1P.y));

    float areaOverestimation = lerp(2.0, 1.0, pow(vDotN, 0.2));
    area *= areaOverestimation;

    debug.areaP[0] = p0P.xy;
    debug.areaP[1] = p1P.xy;
    debug.areaP[2] = p2P.xy;
    debug.area = area;

    if(pParams.update.triangleSize < area && depth < pParams.update.maxSubdivisionDepth)
    {
        return BISECT_ELEMENT;
    }
    else if((pParams.update.triangleSize * 0.5 > area) || (depth > pParams.update.maxSubdivisionDepth))
    {
        float4 p3P = clipToScreen(mul(viewProjectionMatrix, float4(tri.p[3], 1.0)));

        float areaParent = 0.5 * abs(p0P.x * (p3P.y - p2P.y) + p3P.x * (p2P.y - p0P.y) + p2P.x * (p0P.y - p3P.y));
        areaParent *= areaOverestimation;
        return ((pParams.update.triangleSize >= areaParent ) || (depth > pParams.update.maxSubdivisionDepth)) ? TOO_SMALL : UNCHANGED_ELEMENT;
    }
    return UNCHANGED_ELEMENT;
}


// Possible splits
const static uint64_t NO_SPLIT = 0x00;
const static uint64_t CENTER_SPLIT = 0x01;
const static uint64_t RIGHT_SPLIT = 0x02;
const static uint64_t LEFT_SPLIT = 0x04;
const static uint64_t RIGHT_DOUBLE_SPLIT = (CENTER_SPLIT | RIGHT_SPLIT);
const static uint64_t LEFT_DOUBLE_SPLIT = (CENTER_SPLIT | LEFT_SPLIT);
const static uint64_t TRIPLE_SPLIT = (CENTER_SPLIT | RIGHT_SPLIT | LEFT_SPLIT);


const static uint32_t SPLIT_COUNTER = 0;
const static uint32_t SIMPLIFY_COUNTER = 1;
const static uint32_t CLASSIFY_COUNTER_OFFSET = 2;

// memory RW
// CBT
// classify RW
// allocate RW
// propagate RW
// simplify RW
// indirectDraw RW
[numthreads(1, 1, 1)]
void reset()
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

// global
// geometry 
// update
// indirectDraw R
// indexedBisector R
// currentVertex R
// headID R
// bisectorData RW
// classification RW
[numthreads(WORKGROUP_SIZE, 1, 1)]
void classify(uint dispatchID : SV_DispatchThreadID)
{
    if(dispatchID >= pParams.indirectDrawBuffer[9])
        return;

    uint currentID = pParams.indexedBisectorBuffer[dispatchID];
    BisectorGeometry bis;
    bis.p[0] = pParams.currentVertexBuffer[3 * currentID];
    bis.p[1] = pParams.currentVertexBuffer[3 * currentID + 1];
    bis.p[2] = pParams.currentVertexBuffer[3 * currentID + 2];
    bis.p[3] = pParams.currentVertexBuffer[3 * pParams.geometry.totalNumElements + currentID];

    uint64_t heapID = pParams.heapIDBuffer[currentID];
    uint depth = heapIDDepth(heapID);
    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];

    cBisectorData.subdivisionPattern = 0;
    cBisectorData.bisectorState = UNCHANGED_ELEMENT;
    cBisectorData.problematicNeighbor = INVALID_POINTER;
    cBisectorData.flags = VISIBLE_BISECTOR;

    DebugStruct debug;
    debug.sourceP[0] = float4(bis.p[0], 1);
    debug.sourceP[1] = float4(bis.p[1], 1);
    debug.sourceP[2] = float4(bis.p[2], 1);
    int currentValidity = classifyBisector(bis, depth, debug);
    debug.validity = currentValidity;
    pParams.debugBuffer[dispatchID] = debug;
    if(currentValidity > UNCHANGED_ELEMENT)
    {
        uint targetSlot;
        cBisectorData.bisectorState = BISECT_ELEMENT;
        InterlockedAdd(pParams.classificationBuffer[SPLIT_COUNTER], 1, targetSlot);
        pParams.classificationBuffer[CLASSIFY_COUNTER_OFFSET + targetSlot] = currentID;
    }
    else
    {
        cBisectorData.flags = currentValidity >= TOO_SMALL ? VISIBLE_BISECTOR : 0;
    }
    if(pParams.geometry.baseDepth != depth && currentValidity < UNCHANGED_ELEMENT)
    {
        cBisectorData.bisectorState = SIMPLIFY_ELEMENT;

        if(heapID % 2 == 0)
        {
            uint targetSlot;
            InterlockedAdd(pParams.classificationBuffer[SIMPLIFY_COUNTER], 1, targetSlot);
            pParams.classificationBuffer[CLASSIFY_COUNTER_OFFSET + pParams.geometry.totalNumElements + targetSlot] = currentID;
        }
    }
    pParams.bisectorDataBuffer[currentID] = cBisectorData;
}

// classification R
// neighbours R
// bisectorData Rw
// heapID R
// memory RW
// allocate RW
[numthreads(WORKGROUP_SIZE, 1, 1)]
void split(uint dispatchID : SV_DispatchThreadID)
{
    if(dispatchID >= pParams.classificationBuffer[SPLIT_COUNTER])
        return;

    uint currentID = pParams.classificationBuffer[CLASSIFY_COUNTER_OFFSET + dispatchID];

    uint3 cNeighbours = pParams.neighboursBuffer[currentID];

    if(cNeighbours.x != INVALID_POINTER)
    {
        uint3 xNeighbours = pParams.neighboursBuffer[cNeighbours.x];
        if(xNeighbours.z == currentID && pParams.bisectorDataBuffer[cNeighbours.x].bisectorState != UNCHANGED_ELEMENT)
            return;
    }

    if(cNeighbours.y != INVALID_POINTER)
    {
        uint3 yNeighbours = pParams.neighboursBuffer[cNeighbours.y];
        if(yNeighbours.z == currentID && pParams.bisectorDataBuffer[cNeighbours.y].bisectorState != UNCHANGED_ELEMENT)
            return;
    }

    uint64_t heapID = pParams.heapIDBuffer[currentID];
    uint currentDepth = heapIDDepth(heapID);

    int maxRequiredMemory = 2 * (currentDepth - pParams.geometry.baseDepth) - 1;

    uint twinID = cNeighbours.z;

    if(twinID == INVALID_POINTER)
        maxRequiredMemory = 1;
    else if (pParams.neighboursBuffer[twinID].z == currentID)
        maxRequiredMemory = 2;
    
    int remainingMemory;
    InterlockedAdd(pParams.memoryBuffer[1], -maxRequiredMemory, remainingMemory);

    if(remainingMemory < maxRequiredMemory)
    {
        InterlockedAdd(pParams.memoryBuffer[1], maxRequiredMemory, remainingMemory);
        return;
    }

    uint usedMemory = 1;
    uint prevPattern;
    InterlockedOr(pParams.bisectorDataBuffer[currentID].subdivisionPattern, CENTER_SPLIT, prevPattern);

    if(prevPattern != 0)
    {
        InterlockedAdd(pParams.memoryBuffer[1], maxRequiredMemory, remainingMemory);
        return;
    }

    int targetLocation;
    InterlockedAdd(pParams.allocateBuffer[0], 1, targetLocation);
    pParams.allocateBuffer[1 + targetLocation] = currentID;

    bool done = false;
    while(!done)
    {
        if(twinID == INVALID_POINTER)
            break;

        uint64_t nHeapID = pParams.heapIDBuffer[twinID];
        BisectorData nBisectorData = pParams.bisectorDataBuffer[twinID];
        uint nDepth = heapIDDepth(nHeapID);
        uint3 nNeighbours = pParams.neighboursBuffer[twinID];

        if(nDepth == currentDepth)
        {
            InterlockedOr(pParams.bisectorDataBuffer[twinID].subdivisionPattern, CENTER_SPLIT, prevPattern);

            if(prevPattern == 0)
            {
                int targetLocation;
                InterlockedAdd(pParams.allocateBuffer[0], 1, targetLocation);
                pParams.allocateBuffer[1 + targetLocation] = twinID;
                usedMemory++;
            }

            done = true;
        }
        else
        {
            if(nNeighbours[0] == currentID)
                InterlockedOr(pParams.bisectorDataBuffer[twinID].subdivisionPattern, RIGHT_DOUBLE_SPLIT, prevPattern);
            else
                InterlockedOr(pParams.bisectorDataBuffer[twinID].subdivisionPattern, LEFT_DOUBLE_SPLIT, prevPattern);
            
            if(prevPattern != 0)
            {
                usedMemory++;
                done = true;
            }
            else
            {
                int targetLocation;
                InterlockedAdd(pParams.allocateBuffer[0], 1, targetLocation);
                pParams.allocateBuffer[1 + targetLocation] = twinID;

                usedMemory += 2;

                currentID = twinID;
                currentDepth = nDepth;
                twinID = pParams.neighboursBuffer[currentID].z;
            }
        }
    }

    InterlockedAdd(pParams.memoryBuffer[1], max(maxRequiredMemory - usedMemory, 0), remainingMemory);
}

// indirectDispatch RW
// allocate R
[numthreads(1, 1, 1)]
void prepareIndirect(uint currentID: SV_DispatchThreadID)
{
    pParams.indirectDispatchBuffer[currentID * 3 + 0] = (pParams.allocateBuffer[currentID] + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    pParams.indirectDispatchBuffer[currentID * 3 + 1] = 1;
    pParams.indirectDispatchBuffer[currentID * 3 + 2] = 1;
}

// cbtBuffer R
// allocateBuffer R
// memory RW
// bisectorData RW
[numthreads(WORKGROUP_SIZE, 1, 1)]
void allocate(uint groupIndex: SV_GroupIndex, uint dispatchID: SV_DispatchThreadID)
{
    load_buffer_to_shared_memory(groupIndex);

    if(dispatchID >= pParams.allocateBuffer[0])
        return;

    uint currentID = pParams.allocateBuffer[1 + dispatchID];

    BisectorData bisectorData = pParams.bisectorDataBuffer[currentID];

    if(bisectorData.subdivisionPattern != 0)
    {
        uint numSlots = countbits(bisectorData.subdivisionPattern);

        int firstBitIndex = 0;
        InterlockedAdd(pParams.memoryBuffer[0], numSlots, firstBitIndex);

        for(uint bitId = 0; bitId < numSlots; ++bitId)
        {
            uint index = decode_bit_complement(firstBitIndex + bitId);
            bisectorData.indices[bitId] = index;
        }

        pParams.bisectorDataBuffer[currentID] = bisectorData;
    }
}

// bisectorData R
// neightbours R
void evaluateNeighbours(uint currentID, uint bisectorID, out uint resX, out uint resY)
{
    BisectorData nBisectorData = pParams.bisectorDataBuffer[bisectorID];
    uint3 nNeighbours = pParams.neighboursBuffer[bisectorID];
    if(nBisectorData.subdivisionPattern == CENTER_SPLIT)
    {
        resX = nBisectorData.indices[0];
        resY = bisectorID;
    }
    else if(nBisectorData.subdivisionPattern == RIGHT_DOUBLE_SPLIT)
    {
        if(nNeighbours[0] == currentID)
        {
            resX = nBisectorData.indices[1];
            resY = bisectorID;
        }
        else
        {
            resX = nBisectorData.indices[0];
            resY = nBisectorData.indices[1];
        }
    }
    else if(nBisectorData.subdivisionPattern == LEFT_DOUBLE_SPLIT)
    {
        if(nNeighbours[1] == currentID)
        {
            resX = nBisectorData.indices[1];
            resY = nBisectorData.indices[0];
        }
        else
        {
            resX = nBisectorData.indices[0];
            resY = bisectorID;
        }
    }
    else
    {
        if(nNeighbours[0] == currentID)
        {
            resX = nBisectorData.indices[1];
            resY = bisectorID;
        }
        else if (nNeighbours[1] == currentID)
        {
            resX = nBisectorData.indices[2];
            resY = nBisectorData.indices[0];
        }
        else
        {
            resX = nBisectorData.indices[0];
            resY = nBisectorData.indices[1];
        }
    }
}

// allocate R
// heapID RW
// bisectorData RW
// neighbours R
// neighboursOutput RW
[numthreads(WORKGROUP_SIZE, 1, 1)]
void bisect(uint groupIndex : SV_GroupIndex, uint dispatchID : SV_DispatchThreadID)
{
    if(dispatchID >= pParams.allocateBuffer[0])
        return;

    uint currentID = pParams.allocateBuffer[1 + dispatchID];

    uint64_t baseHeapID = pParams.heapIDBuffer[currentID];
    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];
    if(baseHeapID == 0 || cBisectorData.subdivisionPattern == NO_SPLIT)
        return;

    uint currentSubdiv = cBisectorData.subdivisionPattern;

    uint3 cNeighbours = pParams.neighboursBuffer[currentID];
    uint p_n0 = cNeighbours[0];
    uint p_n1 = cNeighbours[1];
    uint p_n2 = cNeighbours[2];

    uint siblingID0 = cBisectorData.indices[0];
    uint siblingID1 = cBisectorData.indices[1];
    uint siblingID2 = cBisectorData.indices[2];

    if(currentSubdiv == CENTER_SPLIT)
    {
        uint resX = INVALID_POINTER;
        uint resY = INVALID_POINTER;
        if(p_n2 != INVALID_POINTER)
            evaluateNeighbours(currentID, p_n2, resX, resY);

        pParams.heapIDBuffer[currentID] = 2 * baseHeapID;
        pParams.heapIDBuffer[siblingID0] = 2 * baseHeapID + 1;

        uint3 modifiedNeighbours;
        modifiedNeighbours[0] = siblingID0;
        modifiedNeighbours[1] = resX;
        modifiedNeighbours[2] = p_n0;
        pParams.neighboursOutputBuffer[currentID] = modifiedNeighbours;
        modifiedNeighbours[0] = resY;
        modifiedNeighbours[1] = currentID;
        modifiedNeighbours[2] = p_n1;
        pParams.neighboursOutputBuffer[siblingID0] = modifiedNeighbours;

        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.propagationID = currentID;

        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[currentID] = modifiedBisector;

        modifiedBisector.problematicNeighbor = p_n1;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID0] = modifiedBisector;

        uint targetLocation = 0;
        InterlockedAdd(pParams.propagateBuffer[0], 1, targetLocation);
        pParams.propagateBuffer[2 + targetLocation] = siblingID0;
    }
    else if (currentSubdiv == RIGHT_DOUBLE_SPLIT)
    {
        uint res0X = INVALID_POINTER;
        uint res0Y = INVALID_POINTER;
        evaluateNeighbours(currentID, p_n0, res0X, res0Y);

        uint res1X = INVALID_POINTER;
        uint res1Y = INVALID_POINTER;
        if(p_n2 != INVALID_POINTER)
            evaluateNeighbours(currentID, p_n2, res1X, res1Y);

        pParams.heapIDBuffer[currentID] = 4 * baseHeapID;
        pParams.heapIDBuffer[siblingID0] = 2 * baseHeapID + 1;
        pParams.heapIDBuffer[siblingID1] = 4 * baseHeapID + 1;

        uint3 modifiedNeighbors;
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

        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.propagationID = currentID;

        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[currentID] = modifiedBisector;

        modifiedBisector.problematicNeighbor = p_n1;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID0] = modifiedBisector;

        
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID1] = modifiedBisector;

        uint targetLocation = 0;
        InterlockedAdd(pParams.propagateBuffer[0], 1, targetLocation);
        pParams.propagateBuffer[2 + targetLocation] = siblingID0;
    }
    else if(currentSubdiv == LEFT_DOUBLE_SPLIT)
    {
        uint res0X = INVALID_POINTER;
        uint res0Y = INVALID_POINTER;
        evaluateNeighbours(currentID, p_n1, res0X, res0Y);

        uint res1X = INVALID_POINTER;
        uint res1Y = INVALID_POINTER;
        if(p_n2 != INVALID_POINTER)
            evaluateNeighbours(currentID, p_n2, res1X, res1Y);

        pParams.heapIDBuffer[currentID] = 2 * baseHeapID;
        pParams.heapIDBuffer[siblingID0] = 4 * baseHeapID + 2;
        pParams.heapIDBuffer[siblingID1] = 4 * baseHeapID + 3;

        
        uint3 modifiedNeighbors;
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

        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.propagationID = currentID;

        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[currentID] = modifiedBisector;

        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID0] = modifiedBisector;

        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID1] = modifiedBisector;
    }
    else if(currentSubdiv == TRIPLE_SPLIT)
    {
        
        uint res0X = INVALID_POINTER;
        uint res0Y = INVALID_POINTER;
        evaluateNeighbours(currentID, p_n0, res0X, res0Y);

        uint res1X = INVALID_POINTER;
        uint res1Y = INVALID_POINTER;
        evaluateNeighbours(currentID, p_n1, res1X, res1Y);
        
        uint res2X = INVALID_POINTER;
        uint res2Y = INVALID_POINTER;
        if(p_n2 != INVALID_POINTER)
            evaluateNeighbours(currentID, p_n2, res2X, res2Y);

        pParams.heapIDBuffer[currentID] = 4 * baseHeapID;
        pParams.heapIDBuffer[siblingID0] = 4 * baseHeapID + 2;
        pParams.heapIDBuffer[siblingID1] = 4 * baseHeapID + 1;
        pParams.heapIDBuffer[siblingID2] = 4 * baseHeapID + 3;

        uint3 modifiedNeighbors;
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

        BisectorData modifiedBisector = cBisectorData;
        modifiedBisector.propagationID = currentID;

        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[currentID] = modifiedBisector;

        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID0] = modifiedBisector;

        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID1] = modifiedBisector;
        
        modifiedBisector.problematicNeighbor = INVALID_POINTER;
        modifiedBisector.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[siblingID2] = modifiedBisector;
    }

    uint numSiblings = countbits(currentSubdiv);
    for(uint siblingIdx = 0; siblingIdx < numSiblings; ++siblingIdx)
    {
        set_bit_atomic_buffer(cBisectorData.indices[siblingIdx], true);
    }
}

// bisectorData RW
// neighbours RW
[numthreads(WORKGROUP_SIZE, 1, 1)]
void propagateBisect(uint dispatchID : SV_DispatchThreadID)
{
    if(dispatchID >= pParams.propagateBuffer[0])
        return;

    uint currentID = pParams.propagateBuffer[2 + dispatchID];
    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];

    uint parentID = cBisectorData.propagationID;
    uint problematicNeighbor = cBisectorData.problematicNeighbor;

    BisectorData tBisectorData = pParams.bisectorDataBuffer[problematicNeighbor];
    uint3 tNeighbours = pParams.neighboursBuffer[problematicNeighbor];
    uint targetID = problematicNeighbor;
    uint sibling1 = tBisectorData.indices[1];

    if(tBisectorData.subdivisionPattern == NO_SPLIT)
    {
        if(tNeighbours[0] == parentID)
            pParams.neighboursBuffer[targetID][0] = currentID;
        if(tNeighbours[1] == parentID)
            pParams.neighboursBuffer[targetID][1] = currentID;
        if(tNeighbours[2] == parentID)
            pParams.neighboursBuffer[targetID][2] = currentID;
    }
    else if(tBisectorData.subdivisionPattern == CENTER_SPLIT)
    {
        if (pParams.neighboursBuffer[targetID][2] == parentID)
            pParams.neighboursBuffer[targetID][2] = currentID;
        if (pParams.neighboursBuffer[tBisectorData.propagationID][2] == parentID)
            pParams.neighboursBuffer[tBisectorData.propagationID][2] = currentID;
    }
    else if(tBisectorData.subdivisionPattern == RIGHT_DOUBLE_SPLIT)
    {
        pParams.neighboursBuffer[sibling1][2] = currentID;
    }
    else if(tBisectorData.subdivisionPattern == LEFT_DOUBLE_SPLIT)
    {
        pParams.neighboursBuffer[targetID][2] = currentID;
    }
    pParams.bisectorDataBuffer[currentID].problematicNeighbor = INVALID_POINTER;
    pParams.bisectorDataBuffer[currentID].bisectorState = UNCHANGED_ELEMENT;
}

// geometry
// classification R
// heapID R
// neighbours R
// bisectorData R
// simplify RW
[numthreads(WORKGROUP_SIZE, 1, 1)]
void prepareSimplify(uint dispatchID : SV_DispatchThreadID)
{
    if(dispatchID >= pParams.classificationBuffer[SIMPLIFY_COUNTER])
        return;

    uint currentID = pParams.classificationBuffer[CLASSIFY_COUNTER_OFFSET + pParams.geometry.totalNumElements + dispatchID];

    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];

    uint64_t cHeapID = pParams.heapIDBuffer[currentID];

    uint3 cNeighbours = pParams.neighboursBuffer[currentID];

    uint currentDepth = heapIDDepth(cHeapID);

    uint pairID = cNeighbours[0];
    uint64_t pHeapID = pParams.heapIDBuffer[pairID];
    BisectorData pBisectorData = pParams.bisectorDataBuffer[pairID];
    uint3 pNeighbours = pParams.neighboursBuffer[pairID];

    uint pairDepth = heapIDDepth(pHeapID);

    if(pairDepth != currentDepth || pBisectorData.bisectorState != SIMPLIFY_ELEMENT)
        return;

    uint twinLowID = pNeighbours[0];
    uint twinHighID = cNeighbours[1];
    if(twinLowID != INVALID_POINTER)
    {
        uint64_t twinLowHeapID = pParams.heapIDBuffer[twinLowID];
        uint64_t twinHighHeapID = pParams.heapIDBuffer[twinHighID];

        if(cHeapID > twinLowHeapID)
            return;

        uint lowFacingDepth = heapIDDepth(twinLowHeapID);
        uint highFacingDepth = heapIDDepth(twinHighHeapID);

        if(lowFacingDepth != currentDepth || highFacingDepth != currentDepth)
            return;

        BisectorData twinLowBisectorData = pParams.bisectorDataBuffer[twinLowID];
        BisectorData twinHighBisectorData = pParams.bisectorDataBuffer[twinHighID];

        if(twinLowBisectorData.bisectorState != SIMPLIFY_ELEMENT || twinHighBisectorData.bisectorState != SIMPLIFY_ELEMENT)
            return;
    }

    uint bisectorSlot = 0;
    InterlockedAdd(pParams.simplifyBuffer[0], 1, bisectorSlot);

    pParams.simplifyBuffer[1 + bisectorSlot] = currentID;
}

// simplify R
// bisectorData RW
// neighbours RW
// heapID RW
[numthreads(WORKGROUP_SIZE, 1, 1)]
void simplify(uint dispatchID : SV_DispatchThreadID)
{
    if(dispatchID >= pParams.simplifyBuffer[0])
        return;

    uint currentID = pParams.simplifyBuffer[1 + dispatchID];

    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];
    uint3 cNeighbours = pParams.neighboursBuffer[currentID];

    uint pairID = cNeighbours[0];
    BisectorData pBisectorData = pParams.bisectorDataBuffer[pairID];
    uint3 pNeighbours = pParams.neighboursBuffer[pairID];

    uint twinLowID = pNeighbours[0];
    uint twinHighID = cNeighbours[1];

    pParams.heapIDBuffer[currentID] = pParams.heapIDBuffer[currentID] / 2;
    pParams.heapIDBuffer[pairID] = 0;

    uint3 newNeighbours;
    newNeighbours[0] = cNeighbours[2];
    newNeighbours[1] = pNeighbours[2];
    newNeighbours[2] = twinLowID;
    pParams.neighboursBuffer[currentID] = newNeighbours;

    cBisectorData.propagationID = pairID;
    cBisectorData.problematicNeighbor = pNeighbours[2];
    cBisectorData.bisectorState = MERGED_ELEMENT;
    cBisectorData.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
    pParams.bisectorDataBuffer[currentID] = cBisectorData;

    if(cBisectorData.problematicNeighbor != INVALID_POINTER)
    {
        uint targetLocation = 0;
        InterlockedAdd(pParams.propagateBuffer[1], 1, targetLocation);
        pParams.propagateBuffer[2 + targetLocation] = currentID;
    }
    
    pBisectorData.bisectorState = MERGED_ELEMENT;
    pBisectorData.flags = 0;
    pParams.bisectorDataBuffer[pairID] = pBisectorData;

    set_bit_atomic_buffer(pairID, false);

    if(twinLowID != INVALID_POINTER)
    {
        pParams.heapIDBuffer[twinLowID] = pParams.heapIDBuffer[twinLowID] / 2;
        pParams.heapIDBuffer[twinHighID] = 0;

        BisectorData lowFacingBst = pParams.bisectorDataBuffer[twinLowID];
        uint3 lfNeighbours = pParams.neighboursBuffer[twinLowID];
        BisectorData highFacingBst = pParams.bisectorDataBuffer[twinHighID];
        uint3 hfNeighbours = pParams.neighboursBuffer[twinHighID];

        newNeighbours[0] = lfNeighbours[2];
        newNeighbours[1] = hfNeighbours[2];
        newNeighbours[2] = currentID;
        pParams.neighboursBuffer[twinLowID] = newNeighbours;

        lowFacingBst.propagationID = twinHighID;
        lowFacingBst.problematicNeighbor = hfNeighbours[2];
        lowFacingBst.bisectorState = MERGED_ELEMENT;
        lowFacingBst.flags = (VISIBLE_BISECTOR | MODIFIED_BISECTOR);
        pParams.bisectorDataBuffer[twinLowID] = lowFacingBst;

        if(lowFacingBst.problematicNeighbor != INVALID_POINTER)
        {
            uint targetLocation = 0;
            InterlockedAdd(pParams.propagateBuffer[1], 1, targetLocation);
            pParams.propagateBuffer[2 + targetLocation] = twinLowID;
        }

        highFacingBst.bisectorState = MERGED_ELEMENT;
        highFacingBst.flags = 0;
        pParams.bisectorDataBuffer[twinHighID] = highFacingBst;

        set_bit_atomic_buffer(twinHighID, false);
    }
}

// neighbours RW
// propagate R
// bisectorData RW
[numthreads(WORKGROUP_SIZE, 1, 1)]
void propagateSimplify(uint dispatchID : SV_DispatchThreadID)
{
    if(dispatchID >= pParams.propagateBuffer[1])
        return;

    uint currentID = pParams.propagateBuffer[2 + dispatchID];

    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];

    uint deletedPair = cBisectorData.problematicNeighbor;

    uint neighbourID = cBisectorData.problematicNeighbor;

    BisectorData nBisectorData = pParams.bisectorDataBuffer[neighbourID];
    uint3 nNeighbours = pParams.neighboursBuffer[neighbourID];

    if(nBisectorData.bisectorState != MERGED_ELEMENT)
    {
        for(uint i = 0; i < 3; ++i)
        {
            if(nNeighbours[i] == deletedPair)
                pParams.neighboursBuffer[neighbourID][i] = currentID;
        }
    }
    else if (nBisectorData.bisectorState == MERGED_ELEMENT)
    {
        if(pParams.heapIDBuffer[neighbourID] != 0)
        {
            for(uint i = 0; i < 3; ++i)
            {
                if(nNeighbours[i] == deletedPair)
                    pParams.neighboursBuffer[neighbourID][i] = currentID;
            }
        }
        else
        {
            uint neighbourPair = nNeighbours[1];
            for(uint i = 0; i < 3; ++i)
            {
                if (pParams.neighboursBuffer[neighbourPair][i] == deletedPair)
                    pParams.neighboursBuffer[neighbourPair][i] = currentID;
            }
        }
    }

    pParams.bisectorDataBuffer[currentID].problematicNeighbor = INVALID_POINTER;
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void reducePrePass(uint dispatchThreadID: SV_DispatchThreadID)
{
    reduce_prepass(dispatchThreadID);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void reduceFirstPass(uint dispatchID: SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    reduce_first_pass(dispatchID, groupIndex);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void reduceSecondPass(uint groupIndex: SV_GroupIndex)
{
    reduce_second_pass(groupIndex);
}

// geometry
// heapID R
// indirectDraw RW
// bisectorIndices RW
// bisectorData R
[numthreads(WORKGROUP_SIZE, 1, 1)]
void bisectorIndexation(uint currentID: SV_DispatchThreadID)
{
    if(currentID >= pParams.geometry.totalNumElements)
        return;

    uint64_t cHeapID = pParams.heapIDBuffer[currentID];

    if(cHeapID == 0)
        return;

    uint bisectorSlot;
    InterlockedAdd(pParams.indirectDrawBuffer[0], 3, bisectorSlot);
    pParams.bisectorIndicesBuffer[bisectorSlot / 3] = currentID;

    BisectorData cBisectorData = pParams.bisectorDataBuffer[currentID];

    if((cBisectorData.flags & VISIBLE_BISECTOR) == 0)
        return;

    InterlockedAdd(pParams.indirectDrawBuffer[4], 3, bisectorSlot);

    pParams.visibleBisectorIndices[bisectorSlot / 3] = currentID;

    if((cBisectorData.flags & MODIFIED_BISECTOR) == 0)
        return;

    InterlockedAdd(pParams.indirectDrawBuffer[8], 4, bisectorSlot);

    pParams.modifiedBisectorIndices[bisectorSlot / 4] = currentID;
}

[numthreads(1, 1, 1)]
void prepareBisectorIndirect(uint currentID: SV_DispatchThreadID)
{
    pParams.indirectDispatchBuffer[0] = (pParams.indirectDrawBuffer[0] / 3 + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    pParams.indirectDispatchBuffer[1] = 1;
    pParams.indirectDispatchBuffer[2] = 1;

    pParams.indirectDispatchBuffer[3] = (pParams.indirectDrawBuffer[0] * 4 / 3 + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    pParams.indirectDispatchBuffer[4] = 1;
    pParams.indirectDispatchBuffer[5] = 1;

    pParams.indirectDispatchBuffer[6] = (pParams.indirectDrawBuffer[8] + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    pParams.indirectDispatchBuffer[7] = 1;
    pParams.indirectDispatchBuffer[8] = 1;

    pParams.indirectDrawBuffer[9] = pParams.indirectDrawBuffer[0] / 3;
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void validate(uint currentID: SV_DispatchThreadID)
{
    if(currentID >= pParams.geometry.totalNumElements)
        return;

    uint64_t cHeapID = pParams.heapIDBuffer[currentID];

    if(cHeapID == 0)
        return;

    uint3 cNeighbours = pParams.neighboursBuffer[currentID];

    bool failed = false;
    uint targetNeighbour = INVALID_POINTER;
    uint targetIdx = INVALID_POINTER;
    for(uint i = 0; i < 3; ++i)
    {
        uint neighbourID = cNeighbours[i];
        if(neighbourID != INVALID_POINTER)
        {
            bool found = false;
            uint3 nNeighbours = pParams.neighboursBuffer[neighbourID];
            for(uint j = 0; j < 3; ++j)
            {
                if(nNeighbours[j] == currentID)
                    found = true;
            }
            if(!found)
            {
                failed = true;
                targetNeighbour = neighbourID;
                targetIdx = i;
                break;
            }
        }
    }
    if(failed)
    {
        uint preValue;
        InterlockedAdd(pParams.validationBuffer[0], 1, preValue);
    }
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void clearLeb(uint currentID: SV_DispatchThreadID)
{
    if(currentID >= pParams.geometry.totalNumElements)
        return;
    pParams.lebPositionBuffer[currentID] = float3(0, 0, 0);
}

const static uint64_t LEB_TABLE_DEPTH = 5;

groupshared float3x3 gs_MatrixCache[2ULL<<LEB_TABLE_DEPTH];

struct Triangle
{
    float3 p[3];
};

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

void leb__IdentityMatrix3x3(out float3x3 m)
{
    m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
}

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

uint64_t leb__GetBitValue(uint64_t bitField, int64_t bitID)
{
    return ((bitField >> bitID) & 1L);
}

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
        parent = mul(parent, gs_MatrixCache[index]);
        parentHeapID >>= LEB_TABLE_DEPTH;
    }
    if (parentHeapID != 0)
        parent = mul(parent, gs_MatrixCache[uint32_t(parentHeapID)]);

    // Evaluate the child
    if (depth > 0)
        child = leb__SplittingMatrix_out(parent, leb__GetBitValue(heapID, 0));
    else
        child = parent;
}

void leb_DecodeNodeAttributeArray_parent_child(uint64_t heapID, inout float3 childAttribute[3], out float3 parentAttribute[3])
{
    float3x3 child, parent;
    leb__DecodeTransformationMatrix_parent_child_Tabulated(heapID, parent, child);
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

void evaluateElementPosition(uint64_t heapID, uint32_t vertexDataOffset, uint minDepth, RWStructuredBuffer<float3> vertexBuffer, out Triangle parentTri, out Triangle childTri)
{
    // Get the depth of the element
    uint depth = heapIDDepth(heapID);

    // Compute the required shift to find the original vertices
    uint64_t subTreeDepth = depth - minDepth;

    // Compute the base heapID
    uint64_t baseHeapID = 1u << (minDepth - 1);
    uint primitiveID = uint((heapID >> subTreeDepth) - baseHeapID);

    // Grab the base positions of the element
    float3 p0 = float3(vertexBuffer[3 * primitiveID + vertexDataOffset]);
    float3 p1 = float3(vertexBuffer[3 * primitiveID + 1 + vertexDataOffset]);
    float3 p2 = float3(vertexBuffer[3 * primitiveID + 2 + vertexDataOffset]);

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

layout(push_constant)
ConstantBuffer<uint> preRender;

[numthreads(WORKGROUP_SIZE, 1, 1)]
void evaluateLeb(uint currentID : SV_DispatchThreadID, uint groupIndex: SV_GroupIndex)
{
    if(groupIndex < (2ULL << LEB_TABLE_DEPTH))
        gs_MatrixCache[groupIndex] = pParams.lebMatrixCache[groupIndex];

    GroupMemoryBarrierWithGroupSync();

    uint numBisectors;
    if(preRender == 1)
    {
        numBisectors = pParams.indirectDrawBuffer[9];
    }
    else
    {
        numBisectors = pParams.indirectDrawBuffer[8] / 4;
    }
    if(currentID >= numBisectors)
        return;

    currentID = pParams.indexedBisectorBuffer[currentID];

    uint64_t cHeapID = pParams.heapIDBuffer[currentID];
    uint depth = heapIDDepth(cHeapID);

    Triangle parentTri, childTri;
    evaluateElementPosition(cHeapID, 0, pParams.geometry.baseDepth, pParams.currentVertexBuffer, parentTri, childTri);

    pParams.lebPositionBuffer[3 * currentID + 0] = childTri.p[0];
    pParams.lebPositionBuffer[3 * currentID + 1] = childTri.p[1];
    pParams.lebPositionBuffer[3 * currentID + 2] = childTri.p[2];

    if(pParams.geometry.baseDepth < depth)
    {
        const uint parentOffset = 3 * pParams.geometry.totalNumElements;

        pParams.lebPositionBuffer[parentOffset + currentID] = cHeapID % 2 == 0 ? parentTri.p[0] : parentTri.p[2];
    }
}