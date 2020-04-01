#pragma once
#include "VulkanGraphicsResources.h"

namespace Seele
{
    namespace Vulkan
    {
        DECLARE_REF(RenderPass);
        class Framebuffer
        {
        public:
            Framebuffer(PGraphics graphics, PRenderPass renderpass, Gfx::PRenderTargetLayout renderTargetLayout);
            virtual ~Framebuffer();
            inline VkFramebuffer getHandle() const
            {
                return handle;
            }
        private:
            PGraphics graphics;
            VkFramebuffer handle;
            Gfx::PRenderTargetLayout layout;
            PRenderPass renderPass;
        };
        DEFINE_REF(Framebuffer);
    }
}