#pragma once
#include "Math/Vector.h"
#include "Containers/Array.h"

namespace Seele
{
struct DebugVertex
{
    Vector position;
    Vector color;
};
extern Array<DebugVertex> gDebugVertices;
} // namespace Seele
