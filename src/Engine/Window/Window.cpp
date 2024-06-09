#include "Window.h"
#include "WindowManager.h"
#include <functional>

using namespace Seele;

Window::Window(PWindowManager owner, Gfx::OWindow handle) : owner(owner), gfxHandle(std::move(handle)) {
    gfxHandle->setResizeCallback([this](uint32 w, uint32 h) { onResize(w, h); });
}

Window::~Window() {}

void Window::addView(PView view) { views.add(view); }

void Window::pollInputs() { gfxHandle->pollInput(); }

void Window::render() {
    gfxHandle->beginFrame();
    for (auto& view : views) {
        view->beginUpdate();
        view->update();
        view->commitUpdate();
        view->prepareRender();
        view->render();
    }
    gfxHandle->endFrame();
}

Gfx::PWindow Window::getGfxHandle() { return gfxHandle; }

void Window::setFocused(PView view) {
    auto keyFunction = std::bind(&View::keyCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto mouseMoveFunction = std::bind(&View::mouseMoveCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2);
    auto mouseButtonFunction =
        std::bind(&View::mouseButtonCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto scrollFunction = std::bind(&View::scrollCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2);
    auto fileFunction = std::bind(&View::fileCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2);
    gfxHandle->setKeyCallback(keyFunction);
    gfxHandle->setMouseMoveCallback(mouseMoveFunction);
    gfxHandle->setMouseButtonCallback(mouseButtonFunction);
    gfxHandle->setScrollCallback(scrollFunction);
    gfxHandle->setFileCallback(fileFunction);
    gfxHandle->setCloseCallback([this]() { owner->notifyWindowClosed(this); });
}

void Window::onResize(uint32 width, uint32 height) {
    for (auto& view : views) {
        // TODO: some sort of layouting algorithm should do this
        view->resize(URect{
            .size = UVector2(width, height),
            .offset = UVector2(0, 0),
        });
    }
}
