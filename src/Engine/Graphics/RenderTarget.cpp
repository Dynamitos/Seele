#include "RenderTarget.h"

using namespace Seele;
using namespace Seele::Gfx;


RenderTargetLayout::RenderTargetLayout()
	: inputAttachments()
	, colorAttachments()
	, depthAttachment()
{
}

RenderTargetLayout::RenderTargetLayout(PRenderTargetAttachment depthAttachment)
	: inputAttachments()
	, colorAttachments()
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getSizeX())
	, height(depthAttachment->getTexture()->getSizeY())
{
}

RenderTargetLayout::RenderTargetLayout(PRenderTargetAttachment colorAttachment, PRenderTargetAttachment depthAttachment)
	: inputAttachments()
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getSizeX())
	, height(depthAttachment->getTexture()->getSizeY())
{
	colorAttachments.add(colorAttachment);
}
RenderTargetLayout::RenderTargetLayout(Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachment)
	: inputAttachments()
	, colorAttachments(colorAttachments)
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getSizeX())
	, height(depthAttachment->getTexture()->getSizeY())
{
}
RenderTargetLayout::RenderTargetLayout(Array<PRenderTargetAttachment> inputAttachments, Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachment)
	: inputAttachments(inputAttachments)
	, colorAttachments(colorAttachments)
	, depthAttachment(depthAttachment)
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
