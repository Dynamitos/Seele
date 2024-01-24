#pragma once
#include "Asset/MeshAsset.h"

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
