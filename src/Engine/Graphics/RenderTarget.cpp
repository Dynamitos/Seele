#include "RenderTarget.h"

using namespace Seele;
using namespace Seele::Gfx;

RenderTargetAttachment::RenderTargetAttachment(PTextureView texture, SeImageLayout initialLayout, SeImageLayout finalLayout,
                                               SeAttachmentLoadOp loadOp, SeAttachmentStoreOp storeOp, SeAttachmentLoadOp stencilLoadOp,
                                               SeAttachmentStoreOp stencilStoreOp)
    : componentFlags(0), texture(texture), initialLayout(initialLayout), finalLayout(finalLayout),
      loadOp(loadOp),
      storeOp(storeOp), stencilLoadOp(stencilLoadOp), stencilStoreOp(stencilStoreOp) {}

RenderTargetAttachment::RenderTargetAttachment(PViewport viewport, SeImageLayout initialLayout, SeImageLayout finalLayout,
                                               SeAttachmentLoadOp loadOp, SeAttachmentStoreOp storeOp, SeAttachmentLoadOp stencilLoadOp,
                                               SeAttachmentStoreOp stencilStoreOp)
    : componentFlags(0), viewport(viewport), initialLayout(initialLayout), finalLayout(finalLayout), loadOp(loadOp),
      storeOp(storeOp), stencilLoadOp(stencilLoadOp), stencilStoreOp(stencilStoreOp) {}

RenderTargetAttachment::~RenderTargetAttachment() {}
