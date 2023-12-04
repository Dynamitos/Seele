#include "Window.h"

using namespace Seele;
using namespace Seele::Gfx;


Window::Window()
{
}

Window::~Window()
{
}

Viewport::Viewport(PWindow owner, const ViewportCreateInfo& viewportInfo)
	: sizeX(std::min(owner->getFramebufferWidth(), viewportInfo.dimensions.size.x))
	, sizeY(std::min(owner->getFramebufferHeight(), viewportInfo.dimensions.size.y))
	, offsetX(viewportInfo.dimensions.offset.x)
	, offsetY(viewportInfo.dimensions.offset.y)
	, fieldOfView(viewportInfo.fieldOfView)
	, samples(viewportInfo.numSamples)
	, owner(owner)
{
}

Viewport::~Viewport()
{
}

Matrix4 Viewport::getProjectionMatrix() const
{
	if (fieldOfView > 0.0f)
	{
		return glm::perspective(fieldOfView, sizeX / static_cast<float>(sizeY), 0.1f, 1000.0f);
	}
	else
	{
		return glm::ortho(0.0f, (float)sizeX, (float)sizeY, 0.0f);
	}
}