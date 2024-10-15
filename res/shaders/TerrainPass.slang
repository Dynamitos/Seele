import Common;
import Parameters;

// Vertex input
struct VertexInput
{
    uint instanceID : SV_InstanceID;
    uint vertexID : SV_VertexID;
};

// Vertex output
struct VertexOutput
{
    float4 positionCS : SV_POSITION;
    uint triangleID : TRIANGLE_ID;
};

[shader("vertex")]
VertexOutput vert(VertexInput input)
{
    // Initialize the output structure
    VertexOutput output;
    output = (VertexOutput)0;

    // Evaluate the properties of this triangle
    uint triangle_id = input.vertexID / 3;
    uint local_vert_id = input.vertexID % 3;

    // Operate the indirection
    output.triangleID = pParams.indexedBisectorBuffer[triangle_id];

    // Which vertex should be read?
    local_vert_id = local_vert_id == 0 ? 2 : (local_vert_id == 2 ? 0 : 1);
    float3 positionRWS = pParams.currentVertexBuffer[output.triangleID * 3 + local_vert_id].xyz;

    // Apply the view projection
    output.positionCS = mul(pViewParams.projectionMatrix, mul(pViewParams.viewMatrix, float4(positionRWS, 1.0)));
    return output;
}

// Pixel input
struct PixelInput
{
    float4 positionCS : SV_POSITION;
    uint triangleID : TRIANGLE_ID;
};

[shader("pixel")]
float4 frag(PixelInput input) : SV_Target
{
    return float4(0, 1, 0, 1);
}

[shader("compute")]
[numthreads(64, 1, 1)]
void EvaluateDeformation(uint currentID : SV_DispatchThreadID)
{
    // This thread doesn't have any work to do, we're done
    if (currentID >= pParams.indirectDrawBuffer[9] * 4)
        return;

    // Extract the bisector ID and the vertexID
    uint bisectorID = currentID / 4;
    uint localVertexID = currentID % 4;

    // Operate the indirection
    bisectorID = pParams.indexedBisectorBuffer[bisectorID];

    // Evaluate the source vertex
    currentID = localVertexID < 3 ? bisectorID * 3 + localVertexID : 3 * pParams.geometry.totalNumElements + bisectorID;

    // Grab the position that we will be displacing
    float3 positionWS = pParams.lebPositionBuffer[currentID].xyz;
    float3 positionRWS = float3(positionWS - pViewParams.cameraPosition_WS.xyz);

    pParams.currentVertexBuffer[currentID] = float4(positionRWS, 1);
}
