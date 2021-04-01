#pragma once

namespace Seele
{
DECLARE_REF(MaterialAsset)
DECLARE_REF(VertexShaderInput)
DECLARE_NAME_REF(Gfx, VertexBuffer)
DECLARE_NAME_REF(Gfx, IndexBuffer)
DECLARE_NAME_REF(Gfx, UniformBuffer)
struct MeshBatchElement
{
public:
    Gfx::PUniformBuffer uniformBuffer;
    Gfx::PIndexBuffer indexBuffer;

    union
    {   
        uint32* instanceRuns;
    };
    

    uint32 firstIndex;
    uint32 numPrimitives;

    uint32 numInstances;
    uint32 baseVertexIndex;
    uint32 minVertexIndex;
    uint32 maxVertexIndex;

    uint8 isInstanced : 1;
    Gfx::PVertexBuffer indirectArgsBuffer;
    MeshBatchElement()
        : uniformBuffer(nullptr)
        , indexBuffer(nullptr)
        , firstIndex(0)
        , numPrimitives(0)
        , numInstances(1)
        , baseVertexIndex(0)
        , minVertexIndex(0)
        , maxVertexIndex(0)
        , indirectArgsBuffer(nullptr)
    {}
};
struct MeshBatch
{
    Array<MeshBatchElement> elements;

    uint8 useReverseCulling : 1;
    uint8 isBackfaceCullingDisabled : 1;
    uint8 isCastingShadow : 1;
    uint8 useWireframe : 1;

    Gfx::SePrimitiveTopology topology;

    PVertexShaderInput vertexInput;

    PMaterialAsset material;

    inline int32 getNumPrimitives() const
    {
        int32 count = 0;
        for(uint32 index = 0; index < elements.size(); ++index)
        {
            if(elements[index].isInstanced && elements[index].instanceRuns)
            {
                for(uint32 run = 0; run < elements[index].numInstances; ++run)
                {
                    count += elements[index].numPrimitives * (elements[index].instanceRuns[run * 2 + 1] - elements[index].instanceRuns[run * 2] - 1);
                }
            }
            else
            {
                count += elements[index].numPrimitives * elements[index].numInstances;
            }
        }
        return count;
    }

    MeshBatch()
        : useReverseCulling(false)
        , isBackfaceCullingDisabled(false)
        , isCastingShadow(true)
        , useWireframe(false)
        , topology(Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        , vertexInput(nullptr)
        , material(nullptr)
    {
    }
};
DECLARE_REF(PrimitiveComponent)
struct StaticMeshBatch : public MeshBatch
{
    uint32 index;
    PPrimitiveComponent primitiveComponent;
};
} // namespace Seele
