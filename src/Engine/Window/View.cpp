#pragma once
#include "View.h"
#include "Window.h"

Seele::View::View(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &viewportInfo)
	: graphics(graphics), owner(window)
{
	viewport = graphics->createViewport(owner->getGfxHandle(), viewportInfo);
}

Seele::View::~View()
{
}

void Seele::View::beginFrame()
{
	renderer->beginFrame();
}

void Seele::View::render()
{
	renderer->render();
}

void Seele::View::endFrame()
{
	renderer->endFrame();
}

void Seele::View::applyArea(URect area)
{
	renderer->applyArea(area);
}

void View::setFocused() 
{
	owner->setFocused(this);	
}
