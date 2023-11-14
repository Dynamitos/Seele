#include "RenderTarget.h"

using namespace Seele;
using namespace Seele::Gfx;

Window::Window(const WindowCreateInfo& createInfo)
	: windowState(createInfo)
{
}

Window::~Window()
{
}

Viewport::Viewport(PWindow owner, const ViewportCreateInfo& viewportInfo)
	: sizeX(std::min(owner->getWidth(), viewportInfo.dimensions.size.x))
	, sizeY(std::min(owner->getHeight(), viewportInfo.dimensions.size.y))
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
RenderTargetAttachment::RenderTargetAttachment(PTexture2D texture,
	SeAttachmentLoadOp loadOp = SE_ATTACHMENT_LOAD_OP_LOAD,
	SeAttachmentStoreOp storeOp = SE_ATTACHMENT_STORE_OP_STORE,
	SeAttachmentLoadOp stencilLoadOp = SE_ATTACHMENT_LOAD_OP_DONT_CARE,
	SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE)
	: clear()
	, componentFlags(0)
	, loadOp(loadOp)
	, storeOp(storeOp)
	, stencilLoadOp(stencilLoadOp)
	, stencilStoreOp(stencilStoreOp)
	, texture(texture)
{
}

RenderTargetAttachment::~RenderTargetAttachment()
{
}

SwapchainAttachment::SwapchainAttachment(PWindow owner,
	SeAttachmentLoadOp loadOp = SE_ATTACHMENT_LOAD_OP_LOAD,
	SeAttachmentStoreOp storeOp = SE_ATTACHMENT_STORE_OP_STORE,
	SeAttachmentLoadOp stencilLoadOp = SE_ATTACHMENT_LOAD_OP_DONT_CARE,
	SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE)
	: RenderTargetAttachment(nullptr, loadOp, storeOp, stencilLoadOp, stencilStoreOp), owner(owner)
{
	clear.color.float32[0] = 0.0f;
	clear.color.float32[1] = 0.0f;
	clear.color.float32[2] = 0.0f;
	clear.color.float32[3] = 1.0f;
	componentFlags = SE_COLOR_COMPONENT_R_BIT | SE_COLOR_COMPONENT_G_BIT | SE_COLOR_COMPONENT_B_BIT | SE_COLOR_COMPONENT_A_BIT;
}

SwapchainAttachment::~SwapchainAttachment()
{
}


RenderTargetLayout::RenderTargetLayout()
	: inputAttachments()
	, colorAttachments()
	, depthAttachment()
	, width(0)
	, height(0)
{
}

RenderTargetLayout::RenderTargetLayout(PRenderTargetAttachment depthAttachment)
	: inputAttachments()
	, colorAttachments()
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getWidth())
	, height(depthAttachment->getTexture()->getHeight())
{
}

RenderTargetLayout::RenderTargetLayout(PRenderTargetAttachment colorAttachment, PRenderTargetAttachment depthAttachment)
	: inputAttachments()
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getWidth())
	, height(depthAttachment->getTexture()->getHeight())
{
	colorAttachments.add(colorAttachment);
}
RenderTargetLayout::RenderTargetLayout(Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachment)
	: inputAttachments()
	, colorAttachments(colorAttachments)
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getWidth())
	, height(depthAttachment->getTexture()->getHeight())
{
}
RenderTargetLayout::RenderTargetLayout(Array<PRenderTargetAttachment> inputAttachments, Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachment)
	: inputAttachments(inputAttachments)
	, colorAttachments(colorAttachments)
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getWidth())
	, height(depthAttachment->getTexture()->getHeight())
{
}