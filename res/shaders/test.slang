
struct MeshPayload
{
    int exponent;
};

groupshared MeshPayload payload;

[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void taskMain(
	uint3 threadID: SV_GroupIndex,
	uint3 groupID: SV_GroupID
){
	DispatchMesh(1,1,1,payload);
}



const static float2 positions[3] = {
  float2(0.0, -0.5),
  float2(0.5, 0.5),
  float2(-0.5, 0.5)
};

const static float3 colors[3] = {
  float3(1.0, 1.0, 0.0),
  float3(0.0, 1.0, 1.0),
  float3(1.0, 0.0, 1.0)
};
struct Vertex
{
  float4 pos : SV_Position;
  float3 color : Color;
  int index : Index;
  int value : Value;
};

const static uint MAX_VERTS = 12;
const static uint MAX_PRIMS = 4;

[outputtopology("triangle")]
[shader("mesh")]
[numthreads(12, 1, 1)]
void meshMain(
    in uint tig : SV_GroupIndex,
    in payload MeshPayload meshPayload,
    out Vertices<Vertex, MAX_VERTS> verts,
    out Indices<uint3, MAX_PRIMS> triangles)
{
    const uint numVertices = 12;
    const uint numPrimitives = 4;
    SetMeshOutputCounts(numVertices, numPrimitives);

    if(tig < numVertices)
    {
        const int tri = tig / 3;
        verts[tig] = {float4(positions[tig % 3], 0, 1), colors[tig % 3], tri, int(pow(tri, meshPayload.exponent))};
    }

    if(tig < numPrimitives)
        triangles[tig] = tig * 3 + uint3(0,1,2);
}