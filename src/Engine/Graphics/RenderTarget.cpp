#include "RenderTarget.h"

using namespace Seele;
using namespace Seele::Gfx;

RenderTargetAttachment::RenderTargetAttachment(PTexture2D texture,
	SeAttachmentLoadOp loadOp,
	SeAttachmentStoreOp storeOp,
	SeAttachmentLoadOp stencilLoadOp,
	SeAttachmentStoreOp stencilStoreOp)
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
	SeAttachmentLoadOp loadOp,
	SeAttachmentStoreOp storeOp,
	SeAttachmentLoadOp stencilLoadOp,
	SeAttachmentStoreOp stencilStoreOp)
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
	: colorAttachments({colorAttachment})
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getWidth())
	, height(depthAttachment->getTexture()->getHeight())
{
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

RenderTargetLayout::RenderTargetLayout(PRenderTargetAttachment colorAttachment, PRenderTargetAttachment depthAttachment, PRenderTargetAttachment resolveAttachment)
	: colorAttachments({ colorAttachment })
	, depthAttachment(depthAttachment)
	, resolveAttachments({ resolveAttachment })
	, width(depthAttachment->getTexture()->getWidth())
	, height(depthAttachment->getTexture()->getHeight())
{
}
