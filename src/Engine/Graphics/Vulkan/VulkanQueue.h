#pragma once
#include "VulkanGraphicsResources.h"

namespace Seele
{
    namespace Vulkan
    {
        DECLARE_REF(CmdBuffer);
        DECLARE_REF(Graphics);
        class Queue
        {
        public:
            Queue(WGraphics graphics, QueueType queueType, uint32 familyIndex, uint32 queueIndex);
            virtual ~Queue();
            void submitCommandBuffer(PCmdBuffer cmdBuffer, uint32 numSignalSemaphores = 0, VkSemaphore* signalSemaphore = nullptr);
            inline void submitCommandBuffer(PCmdBuffer cmdBuffer, VkSemaphore signalSemaphore)
            {
                submitCommandBuffer(cmdBuffer, 1, &signalSemaphore);
            }
            uint32 getFamilyIndex() const
            {
                return familyIndex;
            }
            inline VkQueue getHandle() const
            {
                return queue;
            }
        private:
            WGraphics graphics;
            VkQueue queue;
            uint32 familyIndex;
            QueueType queueType;
        };
        DEFINE_REF(Queue)
    }
}