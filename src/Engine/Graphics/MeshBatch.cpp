#include "MeshBatch.h"
#include "GraphicsResources.h"
#include "VertexShaderInput.h"
#include "Material/MaterialAsset.h"

using namespace Seele;

MeshBatchElement::MeshBatchElement()
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
MeshBatch::MeshBatch()
    : useReverseCulling(false)
    , isBackfaceCullingDisabled(false)
    , isCastingShadow(true)
    , useWireframe(false)
    , topology(Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    , vertexInput(nullptr)
    , material(nullptr)
{
}