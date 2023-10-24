#pragma once
#include "Asset/MeshAsset.h"
#include "Graphics/VertexData.h"
#include "Graphics/TopologyData.h"
#include "Material/MaterialInstance.h"

namespace Seele
{
namespace Component
{
struct Mesh
{
    VertexData* vertexData;
    MeshId id;
    PMaterialInstance instance;
};
} // namespace Component
} // namespace Seele
