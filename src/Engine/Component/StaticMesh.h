#pragma once
#include "Asset/MeshAsset.h"

namespace Seele
{
namespace Component
{
struct StaticMesh
{
    StaticMesh() {}
    StaticMesh(PMeshAsset mesh) : mesh(mesh) {}
    PMeshAsset mesh;
};
} // namespace Component
} // namespace Seele
