#include "View.h"
#include "Graphics/Graphics.h"
#include "Window.h"

using namespace Seele;

View::View(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& viewportInfo, std::string name)
    : graphics(graphics), createInfo(viewportInfo), owner(window), name(name) {
    viewport = graphics->createViewport(owner->getGfxHandle(), viewportInfo);
    owner->addView(this);
    setFocused();
}

View::~View() {}

void View::resize(URect area) {
    createInfo.dimensions = area;
    viewport = graphics->createViewport(owner->getGfxHandle(), createInfo);
    applyArea(area);
}

void View::setFocused() { owner->setFocused(this); }

const std::string& View::getName() { return name; }
