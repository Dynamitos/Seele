#include "Window.h"

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
    Matrix4 correctionMatrix = Matrix4(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1 / 2.f, 0, 0, 0, 1 / 2.f, 1);
    if (fieldOfView > 0.0f) {
        const float aspect = static_cast<float>(sizeX) / sizeY;
        const float e = 1.0f / std::tan(fieldOfView * 0.5f);
        return {
            {
                e / aspect,
                0.0f,
                0.0f,
                0.0f,
            },
            {
                0.0f,
                -e,
                0.0f,
                0.0f,
            },
            {
                0.0f,
                0.0f,
                0.5f * (farPlane + nearPlane) / (nearPlane - farPlane),
                -1.0f,
            },
            {
                0.0f,
                0.0f,
                (farPlane * nearPlane) / (nearPlane - farPlane),
                0.0f,
            },
        };
    } else {
        return correctionMatrix * glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, nearPlane, farPlane);
    }
}