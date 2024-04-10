#include "RenderPass.h"
#include "Enums.h"
#include "Graphics/RenderTarget.h"
#include "Metal/MTLRenderPass.hpp"
#include "Texture.h"

using namespace Seele;
using namespace Seele::Metal;

RenderPass::RenderPass(PGraphics graphics, Gfx::RenderTargetLayout layout,
                       Array<Gfx::SubPassDependency> dependencies,
                       Gfx::PViewport viewport)
    : Gfx::RenderPass(layout, dependencies), graphics(graphics),
      viewport(viewport) {
  renderPass = MTL::RenderPassDescriptor::alloc()->init();
  renderPass->setRenderTargetArrayLength(1);
  renderPass->setRenderTargetWidth(viewport->getWidth());
  renderPass->setRenderTargetHeight(viewport->getHeight());
  renderPass->setDefaultRasterSampleCount(viewport->getSamples());

  for (size_t i = 0; i < layout.colorAttachments.size(); ++i) {
    const auto &color = layout.colorAttachments[i];
    MTL::RenderPassColorAttachmentDescriptor *desc =
        renderPass->colorAttachments()->object(i);
    desc->setClearColor(MTL::ClearColor(
        color.clear.color.float32[0], color.clear.color.float32[1],
        color.clear.color.float32[2], color.clear.color.float32[3]));
    desc->setLoadAction(cast(color.getLoadOp()));
    desc->setStoreAction(cast(color.getStoreOp()));
    desc->setLevel(0);
    desc->setTexture(color.getTexture().cast<TextureBase>()->getTexture());
    if (!layout.resolveAttachments.empty()) {
      const auto &resolve = layout.resolveAttachments[i];
      desc->setResolveLevel(0);
      desc->setStoreAction(MTL::StoreActionStoreAndMultisampleResolve);
      desc->setResolveTexture(
          resolve.getTexture().cast<TextureBase>()->getTexture());
    }
    colorAttachments->setObject(desc, i);
  }
  if (layout.depthAttachment.getTexture() != nullptr) {
    depth = MTL::RenderPassDepthAttachmentDescriptor::alloc()->init();
    depth->setClearDepth(layout.depthAttachment.clear.depthStencil.depth);
    depth->setLoadAction(cast(layout.depthAttachment.getLoadOp()));
    depth->setStoreAction(cast(layout.depthAttachment.getStoreOp()));
    depth->setTexture(
        layout.depthAttachment.getTexture().cast<TextureBase>()->getTexture());
    if (layout.depthResolveAttachment.getTexture() != nullptr) {
      depth->setResolveTexture(layout.depthResolveAttachment.getTexture()
                                   .cast<TextureBase>()
                                   ->getTexture());
      depth->setStoreAction(MTL::StoreActionStoreAndMultisampleResolve);
    }
    renderPass->setDepthAttachment(depth);
  }
  // TODO: stencil
}
RenderPass::~RenderPass() {
  depth->release();
  stencil->release();
  renderPass->release();
}