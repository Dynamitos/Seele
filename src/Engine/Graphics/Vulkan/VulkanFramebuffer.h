#pragma once
#include "VulkanGraphicsResources.h"

namespace Seele
{
    namespace Vulkan
    {
        class Framebuffer
        {
        public:
            Framebuffer(WGraphics graphics);
            virtual ~Framebuffer();
            inline VkFramebuffer getHandle() const
            {
                return handle;
            }
        private:
            WGraphics graphics;
            VkFramebuffer handle;
        };
        DEFINE_REF(Framebuffer);
    }
}