#pragma once
#include "Asset/MeshAsset.h"
#include "Graphics/VertexData.h"
#include "Material/MaterialInstance.h"
#include "Graphics/Mesh.h"

namespace Seele
{
namespace Component
{
struct Mesh
{
    PMeshAsset asset;
    Mesh() {}
    Mesh(PMeshAsset asset) : asset(asset) {}
};
} // namespace Component
} // namespace Seele
