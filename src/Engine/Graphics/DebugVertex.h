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
void addDebugVertex(DebugVertex vert);
void addDebugVertices(Array<DebugVertex> vert);
} // namespace Seele
