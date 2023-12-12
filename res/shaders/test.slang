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
    };

    const static uint MAX_VERTS = 12;
    const static uint MAX_PRIMS = 4;

    [outputtopology("triangle")]
    [shader("mesh")]
    [numthreads(3, 1, 1)]
    void meshMain(
        in uint tig : SV_GroupIndex,
        out Vertices<Vertex, MAX_VERTS> verts,
        out Indices<uint3, MAX_PRIMS> triangles)
    {
        const uint numVertices = 12;
        const uint numPrimitives = 4;
        SetMeshOutputCounts(numVertices, numPrimitives);

        for(uint i = tig; i < numVertices; ++i)
        {
            const int tri = i / 3;
            verts[i] = {float4(positions[i % 3], 0, 1), colors[i % 3], tri};
        }

        for(uint i = tig; i < numPrimitives; ++i)
        {
            triangles[i] = i * 3 + uint3(0,1,2);
        }
    }