#pragma once
#include "Containers/Array.h"
#include "Math/Vector.h"


namespace Seele {
struct DebugVertex {
    Vector position;
    Vector color;
};
void addDebugVertex(DebugVertex vert);
void addDebugVertices(Array<DebugVertex> vert);
} // namespace Seele
