#include "RenderPass.h"
#include "Enums.h"
#include "Graphics/RenderTarget.h"
#include "Metal/MTLRenderPass.hpp"
#include "Texture.h"

using namespace Seele;
using namespace Seele::Metal;

RenderPass::RenderPass(PGraphics graphics, Gfx::RenderTargetLayout _layout, Array<Gfx::SubPassDependency> dependencies,
                       Gfx::PViewport viewport, const std::string& name)
    : Gfx::RenderPass(std::move(_layout), dependencies), graphics(graphics), viewport(viewport), name(name) {
    renderPass = MTL::RenderPassDescriptor::renderPassDescriptor();
    renderPass->setRenderTargetArrayLength(1);
    renderPass->setRenderTargetWidth(viewport->getWidth());
    renderPass->setRenderTargetHeight(viewport->getHeight());
    renderPass->setDefaultRasterSampleCount(viewport->getSamples());

    for (size_t i = 0; i < layout.colorAttachments.size(); ++i) {
        const auto& color = layout.colorAttachments[i];
        MTL::RenderPassColorAttachmentDescriptor* desc = renderPass->colorAttachments()->object(i);
        desc->setClearColor(MTL::ClearColor(color.clear.color.float32[0], color.clear.color.float32[1], color.clear.color.float32[2],
                                            color.clear.color.float32[3]));
        desc->setLoadAction(cast(color.getLoadOp()));
        desc->setStoreAction(cast(color.getStoreOp()));
        desc->setLevel(0);
        if (!layout.resolveAttachments.empty()) {
            const auto& resolve = layout.resolveAttachments[i];
            desc->setResolveLevel(0);
            // store multisampled attachment as well
            if(color.getStoreOp() == Gfx::SE_ATTACHMENT_STORE_OP_STORE) {
                desc->setStoreAction(MTL::StoreActionStoreAndMultisampleResolve);
            } else {
                desc->setStoreAction(MTL::StoreActionMultisampleResolve);
            }
            desc->setResolveTexture(resolve.getTexture().cast<TextureBase>()->getImage());
        }
    }
    if (layout.depthAttachment.getTexture() != nullptr) {
        auto depth = renderPass->depthAttachment();
        depth->setClearDepth(layout.depthAttachment.clear.depthStencil.depth);
        depth->setLoadAction(cast(layout.depthAttachment.getLoadOp()));
        depth->setStoreAction(cast(layout.depthAttachment.getStoreOp()));

        if (layout.depthResolveAttachment.getTexture() != nullptr) {
            // store multisampled attachment as well
            if(layout.depthAttachment.getStoreOp() == Gfx::SE_ATTACHMENT_STORE_OP_STORE) {
                depth->setStoreAction(MTL::StoreActionStoreAndMultisampleResolve);
            } else {
                depth->setStoreAction(MTL::StoreActionMultisampleResolve);
            }
            depth->setResolveTexture(layout.depthResolveAttachment.getTexture().cast<TextureBase>()->getImage());
        }
    }
    // TODO: stencil
}
RenderPass::~RenderPass() {}

void RenderPass::updateRenderPass() {
    for (size_t i = 0; i < layout.colorAttachments.size(); ++i) {
        const auto& color = layout.colorAttachments[i];
        auto desc = renderPass->colorAttachments()->object(i);
        desc->setTexture(color.getTexture().cast<TextureBase>()->getImage());
    }
    if (layout.depthAttachment.getTexture() != nullptr) {
        auto depth = renderPass->depthAttachment();
        depth->setTexture(layout.depthAttachment.getTexture().cast<TextureBase>()->getImage());
    }
}
