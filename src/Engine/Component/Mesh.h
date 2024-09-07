#pragma once
#include "Asset/MeshAsset.h"

namespace Seele {
namespace Component {
struct Mesh {
    PMeshAsset asset;
    Array<uint32> meshletOffsets;
    bool isStatic = true;
};
} // namespace Component
} // namespace Seele
