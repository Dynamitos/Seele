#include "Window.h"
#include "Math/Matrix.h"

using namespace Seele;
using namespace Seele::Gfx;

Window::Window() {}

Window::~Window() {}

Viewport::Viewport(PWindow owner, const ViewportCreateInfo& viewportInfo)
    : sizeX(viewportInfo.dimensions.size.x), sizeY(viewportInfo.dimensions.size.y), offsetX(viewportInfo.dimensions.offset.x),
      offsetY(viewportInfo.dimensions.offset.y), fieldOfView(viewportInfo.fieldOfView), orthoLeft(viewportInfo.left),
      orthoRight(viewportInfo.right), orthoTop(viewportInfo.top), orthoBottom(viewportInfo.bottom), owner(owner) {
    if (owner != nullptr) {
        sizeX = std::min(owner->getFramebufferWidth(), sizeX);
        sizeY = std::min(owner->getFramebufferHeight(), sizeY);
    }
}

Viewport::~Viewport() {}

Matrix4 Viewport::getProjectionMatrix(float nearPlane, float farPlane) const {
    if (fieldOfView > 0.0f) {
        return perspectiveProjection(fieldOfView, static_cast<float>(sizeX) / sizeY, nearPlane, farPlane);
    } else {
        return orthographicProjection(orthoLeft, orthoRight, orthoBottom, orthoTop, nearPlane, farPlane);
    }
}