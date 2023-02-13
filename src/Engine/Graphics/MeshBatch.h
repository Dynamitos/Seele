#pragma once
#include "GraphicsEnums.h"
#include "Containers/Array.h"

namespace Seele
{
DECLARE_REF(VertexShaderInput)
DECLARE_REF(MaterialInterface)
DECLARE_NAME_REF(Gfx, VertexBuffer)
DECLARE_NAME_REF(Gfx, IndexBuffer)
DECLARE_NAME_REF(Gfx, UniformBuffer)
struct MeshBatchElement
{
public:
    Gfx::PIndexBuffer indexBuffer;
    
    uint32 sceneDataIndex;
    uint32 firstIndex;
    uint32 numPrimitives;

    uint32 numInstances;
    uint32 baseVertexIndex;
    uint32 minVertexIndex;
    uint32 maxVertexIndex;

    uint8 isInstanced : 1;
    Gfx::PVertexBuffer indirectArgsBuffer;
    MeshBatchElement();
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

    PMaterialInterface material;

    MeshBatch();
    MeshBatch(const MeshBatch& other) = default;
    MeshBatch(MeshBatch&& other) = default;
    MeshBatch& operator=(const MeshBatch& other) = default;
    MeshBatch& operator=(MeshBatch&& other) = default;
};
} // namespace Seele
