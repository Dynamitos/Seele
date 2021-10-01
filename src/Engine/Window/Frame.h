#pragma once
#include "View.h"

namespace Seele
{
// A frame is the mutable data needed to
// process a time step for the game updates
// It contains all the game update relevant data
// and is handed over to the renderer for read only processing
// If the game loop runs faster than the renderer, the renderer
// simply discards old Frames and starts working on the more recent ones
// if the game loop runs slower than the renderer (bad), the renderer has to wait
struct Frame
{
    uint64 frameNumber;
    Array<PViewFrame> viewFrame;
};
} // namespace Seele
