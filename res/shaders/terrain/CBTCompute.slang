import update_utils;
import Parameters;
import CBT;

static const uint32_t WORKGROUP_SIZE = 64;

[numthreads(1, 1, 1)]
void Reset()
{
    ResetBuffers();
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void Classify(uint currentID : SV_DispatchThreadID)
{
    // This thread doesn't have any work to do, we're done
    if (currentID >= pParams.indirectDrawBuffer[9])
        return;

    // Operate the indirection
    currentID = pParams.indexedBisectorBuffer[currentID];

    // Read the current geometry data
    BisectorGeometry bis;
    bis.p[0] = pParams.currentVertexBuffer[3 * currentID].xyz;
    bis.p[1] = pParams.currentVertexBuffer[3 * currentID + 1].xyz;
    bis.p[2] = pParams.currentVertexBuffer[3 * currentID + 2].xyz;
    bis.p[3] = pParams.currentVertexBuffer[3 * pParams.geometry.totalNumElements + currentID].xyz;

    // Classify the element
    ClassifyElement(currentID, bis, pParams.geometry.totalNumElements, pParams.geometry.baseDepth);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void Split(uint dispatchID : SV_DispatchThreadID)
{
    if (dispatchID >= pParams.classificationBuffer[SPLIT_COUNTER])
        return;

    // Grab the real elementID
    uint currentID = pParams.classificationBuffer[CLASSIFY_COUNTER_OFFSET + dispatchID];

    // Split the element
    SplitElement(currentID, pParams.geometry.baseDepth, dispatchID);
}

[numthreads(1, 1, 1)]
void PrepareIndirect(uint currentID : SV_DispatchThreadID)
{
    pParams.indirectDispatchBuffer[currentID * 3 + 0] = (pParams.allocateBuffer[currentID] + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    pParams.indirectDispatchBuffer[currentID * 3 + 1] = 1;
    pParams.indirectDispatchBuffer[currentID * 3 + 2] = 1;
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void Allocate(uint groupIndex : SV_GroupIndex, uint dispatchID : SV_DispatchThreadID)
{
    // Load the CBT to the LDS
    load_buffer_to_shared_memory(groupIndex);

    // If this element doesn't need to be processed, we're done
    if (dispatchID >= pParams.allocateBuffer[0])
        return;

    // Allocate the required bits
    AllocateElement(pParams.allocateBuffer[1 + dispatchID]);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void Bisect(uint groupIndex : SV_GroupIndex, uint dispatchID : SV_DispatchThreadID)
{
    // If this element doesn't need to be processed, we're done
    if (dispatchID >= pParams.allocateBuffer[0])
        return;

    // Operation the bisection of this element
    BisectElement(pParams.allocateBuffer[1 + dispatchID]);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void PropagateBisect(uint dispatchID : SV_DispatchThreadID)
{
    // If this element doesn't need to be processed, we're done
    if (dispatchID >= pParams.propagateBuffer[0])
        return;

    PropagateBisectElement(pParams.propagateBuffer[2 + dispatchID]);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void PrepareSimplify(uint dispatchID : SV_DispatchThreadID)
{
    // If this element doesn't need to be processed, we're done
    if (dispatchID >= pParams.classificationBuffer[SIMPLIFY_COUNTER])
        return;

    // Grab the real elementID
    uint currentID = pParams.classificationBuffer[CLASSIFY_COUNTER_OFFSET + pParams.geometry.totalNumElements + dispatchID];

    // Simplify an element
    PrepareSimplifyElement(currentID);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void Simplify(uint currentID : SV_DispatchThreadID)
{
    // This thread doesn't have any work to do, we're done
    if (currentID >= pParams.simplifyBuffer[0])
        return;

    // Simplify an element
    SimplifyElement(pParams.simplifyBuffer[1 + currentID]);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void PropagateSimplify(uint dispatchID : SV_DispatchThreadID)
{
    // If this element doesn't need to be processed, we're done
    if (dispatchID >= pParams.propagateBuffer[1])
        return;
    
    PropagateElementSimplify(pParams.propagateBuffer[2 + dispatchID]);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void ReducePrePass(uint dispatchThreadID : SV_DispatchThreadID)
{
    reduce_prepass(dispatchThreadID);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void ReduceFirstPass(uint dispatchThreadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    // Reduce Mid Pass
    reduce_first_pass(dispatchThreadID, groupIndex);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void ReduceSecondPass(uint groupIndex : SV_GroupIndex)
{
    // Find the ith bit set to one
    reduce_second_pass(groupIndex);
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void BisectorIndexation(uint currentID : SV_DispatchThreadID)
{
    // This thread doesn't have any work to do, we're done
    if (currentID >= pParams.geometry.totalNumElements)
        return;

    // Indexate this bisector
    BisectorElementIndexation(currentID);
}

[numthreads(1, 1, 1)]
void PrepareBisectorIndirect(uint currentID : SV_DispatchThreadID)
{
    // Indirect dispatch for each bisector
    pParams.indirectDispatchBuffer[0] = (pParams.indirectDrawBuffer[0] / 3 + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    pParams.indirectDispatchBuffer[1] = 1;
    pParams.indirectDispatchBuffer[2] = 1;
    
    // Indirect dispatch for each vertex to process (3 per bisector + 1 for the parent)
    pParams.indirectDispatchBuffer[3] = (pParams.indirectDrawBuffer[0] * 4 / 3 + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    pParams.indirectDispatchBuffer[4] = 1;
    pParams.indirectDispatchBuffer[5] = 1;

    // Indirect dispatch for each modified vertex to process
    pParams.indirectDispatchBuffer[6] = (pParams.indirectDrawBuffer[8] + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    pParams.indirectDispatchBuffer[7] = 1;
    pParams.indirectDispatchBuffer[8] = 1;

    // Explicit counter of the number of bisectors
    pParams.indirectDrawBuffer[9] = pParams.indirectDrawBuffer[0] / 3;
}

[numthreads(WORKGROUP_SIZE, 1, 1)]
void Validate(uint currentID : SV_DispatchThreadID)
{
    // This thread doesn't have any work to do, we're done
    if (currentID >= pParams.geometry.totalNumElements)
        return;

    ValidateBisector(currentID);
}
