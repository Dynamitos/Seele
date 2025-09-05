#include "RenderPass.h"
#include "Enums.h"
#include "Graphics/RenderTarget.h"
#include "Metal/MTLRenderPass.hpp"
#include "Texture.h"

using namespace Seele;
using namespace Seele::Metal;

RenderPass::RenderPass(PGraphics graphics, Gfx::RenderTargetLayout _layout, Array<Gfx::SubPassDependency> dependencies,
                       URect renderArea, const std::string& name)
    : Gfx::RenderPass(std::move(_layout), dependencies), graphics(graphics), renderArea(renderArea), name(name) {
    renderPass = MTL::RenderPassDescriptor::renderPassDescriptor();
    renderPass->setRenderTargetArrayLength(1);
    renderPass->setRenderTargetWidth(renderArea.size.x);
    renderPass->setRenderTargetHeight(renderArea.size.y);

    for (size_t i = 0; i < layout.colorAttachments.size(); ++i) {
        const auto& color = layout.colorAttachments[i];
        MTL::RenderPassColorAttachmentDescriptor* desc = renderPass->colorAttachments()->object(i);
        desc->setClearColor(MTL::ClearColor(color.clear.color.float32[0], color.clear.color.float32[1], color.clear.color.float32[2],
                                            color.clear.color.float32[3]));
        desc->setLoadAction(cast(color.getLoadOp()));
        desc->setStoreAction(cast(color.getStoreOp()));
        desc->setLevel(0);
        if (!layout.resolveAttachments.empty()) {
            desc->setResolveLevel(0);
            // store multisampled attachment as well
            if(color.getStoreOp() == Gfx::SE_ATTACHMENT_STORE_OP_STORE) {
                desc->setStoreAction(MTL::StoreActionStoreAndMultisampleResolve);
            } else {
                desc->setStoreAction(MTL::StoreActionMultisampleResolve);
            }
        }
    }
    if (layout.depthAttachment.getTextureView() != nullptr) {
        auto depth = renderPass->depthAttachment();
        depth->setClearDepth(layout.depthAttachment.clear.depthStencil.depth);
        depth->setLoadAction(cast(layout.depthAttachment.getLoadOp()));
        depth->setStoreAction(cast(layout.depthAttachment.getStoreOp()));

        if (layout.depthResolveAttachment.getTextureView() != nullptr) {
            // store multisampled attachment as well
            if(layout.depthAttachment.getStoreOp() == Gfx::SE_ATTACHMENT_STORE_OP_STORE) {
                depth->setStoreAction(MTL::StoreActionStoreAndMultisampleResolve);
            } else {
                depth->setStoreAction(MTL::StoreActionMultisampleResolve);
            }
        }
    }
    // TODO: stencil
}
RenderPass::~RenderPass() {}

void RenderPass::updateRenderPass() {
    for (size_t i = 0; i < layout.colorAttachments.size(); ++i) {
        const auto& color = layout.colorAttachments[i];
        auto desc = renderPass->colorAttachments()->object(i);
        desc->setTexture(color.getTextureView().cast<TextureView>()->getHandle());
        if (!layout.resolveAttachments.empty()) {
            const auto& resolve = layout.resolveAttachments[i];
            desc->setResolveTexture(resolve.getTextureView().cast<TextureView>()->getHandle());
        }
    }
    if (layout.depthAttachment.getTextureView() != nullptr) {
        auto depth = renderPass->depthAttachment();
        depth->setTexture(layout.depthAttachment.getTextureView().cast<TextureView>()->getHandle());
        if (layout.depthResolveAttachment.getTextureView() != nullptr) {
            depth->setResolveTexture(layout.depthResolveAttachment.getTextureView().cast<TextureView>()->getHandle());
        }
    }
}
