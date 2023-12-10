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
