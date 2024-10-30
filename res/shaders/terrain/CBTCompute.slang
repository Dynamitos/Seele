import update_utils;
import Parameters;
import CBT;

static const uint32_t WORKGROUP_SIZE = 64;

[numthreads(1, 1, 1)]
void Reset()
{
    ResetBuffers();
}

[numthreads(1, 1, 1)]
void Classify(uint dispatchID : SV_DispatchThreadID)
{
    // This thread doesn't have any work to do, we're done
    //if (dispatchID >= pParams.indirectDrawBuffer[9])
    //    return;
    if(dispatchID > 0)
        return;
    for(dispatchID = 0; dispatchID < pParams.indirectDrawBuffer[9]; ++dispatchID)
    {

        // Operate the indirection
        uint currentID = pParams.indexedBisectorBuffer[dispatchID];

        // Read the current geometry data
        BisectorGeometry bis;
        bis.p[0] = pParams.currentVertexBuffer[3 * currentID].xyz;
        bis.p[1] = pParams.currentVertexBuffer[3 * currentID + 1].xyz;
        bis.p[2] = pParams.currentVertexBuffer[3 * currentID + 2].xyz;
        bis.p[3] = pParams.currentVertexBuffer[3 * pParams.geometry.totalNumElements + currentID].xyz;

        // Classify the element
        ClassifyElement(currentID, bis, pParams.geometry.totalNumElements, pParams.geometry.baseDepth);
    }
}

[numthreads(1, 1, 1)]
void Split(uint dispatchID : SV_DispatchThreadID)
{
    //if (dispatchID >= pParams.classificationBuffer[SPLIT_COUNTER])
    //    return;
    if(dispatchID > 0)
        return;
    for(dispatchID = 0; dispatchID < pParams.classificationBuffer[SPLIT_COUNTER]; ++dispatchID)
    {
        // Grab the real elementID
        uint currentID = pParams.classificationBuffer[CLASSIFY_COUNTER_OFFSET + dispatchID];

        // Split the element
        SplitElement(currentID, pParams.geometry.baseDepth);
    }

}

[numthreads(1, 1, 1)]
void PrepareIndirect(uint currentID : SV_DispatchThreadID)
{
    pParams.indirectDispatchBuffer[currentID * 3 + 0] = (pParams.allocateBuffer[currentID] + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    pParams.indirectDispatchBuffer[currentID * 3 + 1] = 1;
    pParams.indirectDispatchBuffer[currentID * 3 + 2] = 1;
}

[numthreads(1, 1, 1)]
void Allocate(uint groupIndex : SV_GroupIndex, uint dispatchID : SV_DispatchThreadID)
{
    if(dispatchID > 0)
        return;

    for(uint i = 0; i < 64; ++i)
    {
        // Load the CBT to the LDS
        load_buffer_to_shared_memory(groupIndex);
    }
    
    // If this element doesn't need to be processed, we're done
    if (dispatchID >= pParams.allocateBuffer[0])
        return;
    for(dispatchID = 0; dispatchID < pParams.allocateBuffer[0]; ++dispatchID)
    {
        // Allocate the required bits
        AllocateElement(pParams.allocateBuffer[1 + dispatchID], dispatchID);
    }
}

[numthreads(1, 1, 1)]
void Bisect(uint groupIndex : SV_GroupIndex, uint dispatchID : SV_DispatchThreadID)
{
    // If this element doesn't need to be processed, we're done
    //if (dispatchID >= pParams.allocateBuffer[0])
    //    return;
    if(dispatchID > 0)
        return;
    for(dispatchID = 0; dispatchID < pParams.allocateBuffer[0]; ++dispatchID)
    {
        // Operation the bisection of this element
        BisectElement(pParams.allocateBuffer[1 + dispatchID]);
    }
}

[numthreads(1, 1, 1)]
void PropagateBisect(uint dispatchID : SV_DispatchThreadID)
{
    // If this element doesn't need to be processed, we're done
    //if (dispatchID >= pParams.propagateBuffer[0])
    //    return;
    if(dispatchID > 0)
        return;
    for(dispatchID = 0; dispatchID < pParams.propagateBuffer[0]; ++dispatchID)
    {
        PropagateBisectElement(pParams.propagateBuffer[2 + dispatchID]);
    }
}

[numthreads(1, 1, 1)]
void PrepareSimplify(uint dispatchID : SV_DispatchThreadID)
{
    // If this element doesn't need to be processed, we're done
    if (dispatchID >= pParams.classificationBuffer[SIMPLIFY_COUNTER])
        return;
    if(dispatchID > 0)
        return;
    for(dispatchID = 0; dispatchID < pParams.classificationBuffer[SIMPLIFY_COUNTER]; ++dispatchID)
    {
        // Grab the real elementID
        uint currentID = pParams.classificationBuffer[CLASSIFY_COUNTER_OFFSET + pParams.geometry.totalNumElements + dispatchID];

        // Simplify an element
        PrepareSimplifyElement(currentID);
    }
}

[numthreads(1, 1, 1)]
void Simplify(uint dispatchID : SV_DispatchThreadID)
{
    // This thread doesn't have any work to do, we're done
    if (dispatchID >= pParams.simplifyBuffer[0])
        return;
    if(dispatchID > 0)
        return;
    for(dispatchID = 0; dispatchID < pParams.simplifyBuffer[0]; ++dispatchID)
    {
        // Simplify an element
        SimplifyElement(pParams.simplifyBuffer[1 + dispatchID]);
    }
}

[numthreads(1, 1, 1)]
void PropagateSimplify(uint dispatchID : SV_DispatchThreadID)
{
    // If this element doesn't need to be processed, we're done
    if (dispatchID >= pParams.propagateBuffer[1])
        return;
    if(dispatchID > 0)
        return;
    for(dispatchID = 0; dispatchID < pParams.propagateBuffer[1]; ++dispatchID)
    {
        PropagateElementSimplify(pParams.propagateBuffer[2 + dispatchID]);
    }
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

[numthreads(1, 1, 1)]
void BisectorIndexation(uint currentID : SV_DispatchThreadID)
{
    // This thread doesn't have any work to do, we're done
    //if (currentID >= pParams.geometry.totalNumElements)
    //    return;
    if(currentID > 0)
        return;
    
    for(currentID = 0; currentID < pParams.geometry.totalNumElements; ++currentID)
    {
        // Indexate this bisector
        BisectorElementIndexation(currentID);
    }
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
