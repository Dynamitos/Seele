struct Test
{
    StructuredBuffer<float> vertexData;
};
ParameterBlock<Test> pTest;

struct Params
{
    float3 color;
};

struct P
{
    ParameterBlock<Params> p;
};
ParameterBlock<P> pParams;

struct Vert
{
    float4 clip : SV_Position;
    float3 color : COLOR;
};

struct Pay
{
    float3 color;
};

groupshared Pay p;

[shader("amplification")]
[numthreads(32, 1, 1)]
void taskMain(uint threadID: SV_GroupThreadID)
{
    if (threadID == 0)
    {
        p.color = pParams.p.color;
    }
    GroupMemoryBarrierWithGroupSync();
    DispatchMesh(1, 1, 1, p);
}

const static float4 verti[] = { float4(-0.5, 0.5, 1, 1), float4(0.5, -0.5, 1, 1), float4(-0.5, -0.5, 1, 1), float4(0.5, 0.5, 1, 1), };
const static uint3 indi[] = { uint3(0, 1, 2), uint3(1, 3, 0) };

[shader("mesh")]
[numthreads(32, 1, 1)]
[outputtopology("triangle")]
void meshMain(uint threadID: SV_GroupThreadID, in payload Pay p, out vertices Vert vert[4], out indices uint3 ind[2])
{
    SetMeshOutputCounts(4, 2);
    for (uint i = threadID; i < 4; i += 32)
    {
        Vert v;
        v.clip = float4(pTest.vertexData[i*4+0], pTest.vertexData[i*4+1],pTest.vertexData[i*4+2],pTest.vertexData[i*4+3]);
        v.color = p.color;
        vert[i] = v;
    }
    for (uint i = threadID; i < 2; i += 32)
    {
        ind[i] = indi[i];
    }
}

[shader("vertex")]
Vert vertexMain(uint id: SV_VertexID)
{
    Vert v;
    v.clip = verti[id];
    v.color = float3(0, 1, 0);
    return v;
}

[shader("pixel")]
float4 fragMain(Vert v): SV_Target
{
    return float4(v.color, 1);
}