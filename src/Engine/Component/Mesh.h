#pragma once
#include "Asset/MeshAsset.h"

namespace Seele
{
namespace Component
{
struct Mesh
{
    PMeshAsset asset;
    bool isStatic = true;
};
} // namespace Component
} // namespace Seele
