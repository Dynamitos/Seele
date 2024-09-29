#include "Window.h"

using namespace Seele;
using namespace Seele::Gfx;

Window::Window() {}

Window::~Window() {}

Viewport::Viewport(PWindow owner, const ViewportCreateInfo& viewportInfo)
    : sizeX(std::min(owner->getFramebufferWidth(), viewportInfo.dimensions.size.x)),
      sizeY(std::min(owner->getFramebufferHeight(), viewportInfo.dimensions.size.y)), offsetX(viewportInfo.dimensions.offset.x),
      offsetY(viewportInfo.dimensions.offset.y), fieldOfView(viewportInfo.fieldOfView), samples(viewportInfo.numSamples), owner(owner) {}

Viewport::~Viewport() {}

Matrix4 Viewport::getProjectionMatrix() const {
    if (fieldOfView > 0.0f) {
        // float h = 1.0 / tan(fieldOfView * 0.5);
        // float w = h / (sizeX / static_cast<float>(sizeY));
        // float zFar = 1000.0f;
        // float zNear = 0.1f;
        // float a = -zNear / (zFar - zNear);
        // float b = (zNear * zFar) / (zFar - zNear);
        // return Matrix4(
        //	w, 0, 0, 0,
        //	0, -h, 0, 0,
        //	0, 0, a, b,
        //	0, 0, 1, 0
        //);
        return glm::perspective(fieldOfView, sizeX / static_cast<float>(sizeY), 0.1f, 1000.0f);
    } else {
        return glm::ortho(0.0f, (float)sizeX, (float)sizeY, 0.0f);
    }
}