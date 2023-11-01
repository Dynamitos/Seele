#include "RenderTarget.h"

using namespace Seele;
using namespace Seele::Gfx;


RenderTargetLayout::RenderTargetLayout()
	: inputAttachments()
	, colorAttachments()
	, depthAttachment()
{
}

RenderTargetLayout::RenderTargetLayout(ORenderTargetAttachment depthAttachment)
	: inputAttachments()
	, colorAttachments()
	, depthAttachment(std::move(depthAttachment))
	, width(depthAttachment->getTexture()->getSizeX())
	, height(depthAttachment->getTexture()->getSizeY())
{
}

RenderTargetLayout::RenderTargetLayout(ORenderTargetAttachment colorAttachment, ORenderTargetAttachment depthAttachment)
	: inputAttachments()
	, depthAttachment(std::move(depthAttachment))
	, width(depthAttachment->getTexture()->getSizeX())
	, height(depthAttachment->getTexture()->getSizeY())
{
	colorAttachments.add(colorAttachment);
}
RenderTargetLayout::RenderTargetLayout(Array<ORenderTargetAttachment> colorAttachments, ORenderTargetAttachment depthAttachment)
	: inputAttachments()
	, colorAttachments(std::move(colorAttachments))
	, depthAttachment(std::move(depthAttachment))
	, width(depthAttachment->getTexture()->getSizeX())
	, height(depthAttachment->getTexture()->getSizeY())
{
}
RenderTargetLayout::RenderTargetLayout(Array<ORenderTargetAttachment> inputAttachments, Array<ORenderTargetAttachment> colorAttachments, ORenderTargetAttachment depthAttachment)
	: inputAttachments(std::move(inputAttachments))
	, colorAttachments(std::move(colorAttachments))
	, depthAttachment(std::move(depthAttachment))
	, width(depthAttachment->getTexture()->getSizeX())
	, height(depthAttachment->getTexture()->getSizeY())
{
}

Window::Window(const WindowCreateInfo& createInfo)
	: windowState(createInfo)
{
}

Window::~Window()
{
}

Viewport::Viewport(PWindow owner, const ViewportCreateInfo& viewportInfo)
	: sizeX(viewportInfo.dimensions.size.x)
	, sizeY(viewportInfo.dimensions.size.y)
	, offsetX(viewportInfo.dimensions.offset.x)
	, offsetY(viewportInfo.dimensions.offset.y)
	, fieldOfView(viewportInfo.fieldOfView)
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
