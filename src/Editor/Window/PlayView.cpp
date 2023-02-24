#include "PlayView.h"
#include "Window/Window.h"

using namespace Seele;
using namespace Seele::Editor;

PlayView::PlayView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::string dllPath)
    : GameView(std::move(graphics), std::move(window), std::move(createInfo), dllPath)
{
}

PlayView::~PlayView()
{
}

void PlayView::beginUpdate()
{
    GameView::beginUpdate();
}

void PlayView::update()
{
    GameView::update();
}

void PlayView::commitUpdate()
{
    GameView::commitUpdate();
}

void PlayView::prepareRender()
{
    GameView::prepareRender();
}

void PlayView::render()
{
    GameView::render();
}
