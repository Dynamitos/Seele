#include "Window.h"

using namespace Seele;
using namespace Seele::Gfx;

Window::Window() {}

Window::~Window() {}

Viewport::Viewport(PWindow owner, const ViewportCreateInfo& viewportInfo)
    : sizeX(viewportInfo.dimensions.size.x), sizeY(viewportInfo.dimensions.size.y), offsetX(viewportInfo.dimensions.offset.x),
      offsetY(viewportInfo.dimensions.offset.y), fieldOfView(viewportInfo.fieldOfView), owner(owner) {
    if (owner != nullptr) {
        sizeX = std::min(owner->getFramebufferWidth(), sizeX);
        sizeY = std::min(owner->getFramebufferHeight(), sizeY);
    }
}

Viewport::~Viewport() {}

Matrix4 Viewport::getProjectionMatrix() const {
    if (fieldOfView > 0.0f) {
        Matrix4 projectionMatrix = glm::perspective(fieldOfView, sizeX / static_cast<float>(sizeY), 0.1f, 1000.0f);
        Matrix4 correctionMatrix = Matrix4(
            1, 0, 0, 0, 
            0, -1, 0, 0, 
            0, 0, 1 / 2.f, 0, 
            0, 0, 1 / 2.f, 1);
        return correctionMatrix * projectionMatrix;
    } else {
        return glm::ortho(0.0f, (float)sizeX, (float)sizeY, 0.0f);
    }
}