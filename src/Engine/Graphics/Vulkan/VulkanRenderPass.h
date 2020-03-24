#include "VulkanGraphicsResources.h"

namespace Seele
{
    namespace Vulkan
    {
        class RenderPass
        {
        public:
            RenderPass();
            virtual ~RenderPass();
            inline VkRenderPass getHandle() const
            {
                return renderPass;
            }
            inline uint32 getClearValueCount() const
            {
                return clearValues.size();
            }
            inline VkClearValue* getClearValues() const
            {
                return clearValues.data();
            }
            inline VkRect2D getRenderArea() const
            {
                return renderArea;
            }
            inline VkSubpassContents getSubpassContents() const
            {
                return subpassContents;
            }
        private:
            VkRenderPass renderPass;
            Array<VkClearValue> clearValues;
            VkRect2D renderArea;
            VkSubpassContents subpassContents;
        };
        DEFINE_REF(RenderPass);
    }
}