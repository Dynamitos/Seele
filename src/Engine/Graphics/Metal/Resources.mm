#include "Resources.h"
#include "Graphics.h"

using namespace Seele;
using namespace Seele::Metal;

Fence::Fence(PGraphics graphics) : handle(graphics->getDevice()->newFence()) {}

Fence::~Fence() { handle->release(); }

Event::Event(PGraphics graphics) : handle(graphics->getDevice()->newEvent()) {}

Event::~Event() { handle->release(); }