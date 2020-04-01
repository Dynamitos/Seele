#include "VulkanCommandBuffer.h"
#include "VulkanInitializer.h"
#include "VulkanGraphics.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"

using namespace Seele;
using namespace Seele::Vulkan;

CmdBufferBase::CmdBufferBase(PGraphics graphics, VkCommandPool cmdPool)
    : graphics(graphics)
    , owner(cmdPool)
{

}

CmdBufferBase::~CmdBufferBase()
{
}

CmdBuffer::CmdBuffer(PGraphics graphics, VkCommandPool cmdPool)
    : CmdBufferBase(graphics, cmdPool)
    , renderPass(nullptr)
    , framebuffer(nullptr)
    , subpassIndex(0)
{
    VkCommandBufferAllocateInfo allocInfo =
        init::CommandBufferAllocateInfo(cmdPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1);

    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle))
    state = State::ReadyBegin;
}

CmdBuffer::~CmdBuffer()
{
    vkFreeCommandBuffers(graphics->getDevice(), owner, 1, &handle);
}

void CmdBuffer::begin()
{
    VkCommandBufferBeginInfo beginInfo =
        init::CommandBufferBeginInfo();
    beginInfo.pInheritanceInfo = nullptr;
    VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
    state = State::InsideBegin;
}

void CmdBuffer::end()
{
    VK_CHECK(vkEndCommandBuffer(handle));
    state = State::Ended;
}

void CmdBuffer::beginRenderPass(PRenderPass renderPass, PFramebuffer framebuffer)
{
    VkRenderPassBeginInfo beginInfo =
        init::RenderPassBeginInfo();
    beginInfo.clearValueCount = renderPass->getClearValueCount();
    beginInfo.pClearValues = renderPass->getClearValues();
    beginInfo.renderArea = renderPass->getRenderArea();
    beginInfo.renderPass = renderPass->getHandle();
    beginInfo.framebuffer = framebuffer->getHandle();
    vkCmdBeginRenderPass(handle, &beginInfo, renderPass->getSubpassContents());
}

void CmdBuffer::endRenderPass()
{
    vkCmdEndRenderPass(handle);
}

SecondaryCmdBuffer::SecondaryCmdBuffer(PGraphics graphics, VkCommandPool cmdPool)
    : CmdBufferBase(graphics, cmdPool)
{
    VkCommandBufferAllocateInfo allocInfo =
        init::CommandBufferAllocateInfo(cmdPool,
            VK_COMMAND_BUFFER_LEVEL_SECONDARY,
            1);
    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle));
}

SecondaryCmdBuffer::~SecondaryCmdBuffer()
{
    vkFreeCommandBuffers(graphics->getDevice(), owner, 1, &handle);
}

void SecondaryCmdBuffer::begin(PCmdBuffer parent)
{
    VkCommandBufferBeginInfo beginInfo =
        init::CommandBufferBeginInfo();
    VkCommandBufferInheritanceInfo inheritanceInfo =
        init::CommandBufferInheritanceInfo();
    inheritanceInfo.framebuffer = parent->framebuffer->getHandle();
    inheritanceInfo.renderPass = parent->renderPass->getHandle();
    inheritanceInfo.subpass = parent->subpassIndex;
    beginInfo.pInheritanceInfo = &inheritanceInfo;
    VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
}

void SecondaryCmdBuffer::end()
{
    VK_CHECK(vkEndCommandBuffer(handle));
}

CommandBufferManager::CommandBufferManager(PGraphics graphics, PQueue queue)
    : graphics(graphics)
    , queue(queue)
{
    VkCommandPoolCreateInfo info =
        init::CommandPoolCreateInfo();
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = queue->getFamilyIndex();

    VK_CHECK(vkCreateCommandPool(graphics->getDevice(), &info, nullptr, &commandPool));

    activeCmdBuffer = new CmdBuffer(graphics, commandPool);
    activeCmdBuffer->begin();
}

CommandBufferManager::~CommandBufferManager()
{
    vkDestroyCommandPool(graphics->getDevice(), commandPool, nullptr);
}

PCmdBuffer CommandBufferManager::getCommands()
{
    return activeCmdBuffer;
}

PSecondaryCmdBuffer CommandBufferManager::createSecondaryCmdBuffer()
{
    return new SecondaryCmdBuffer(graphics, commandPool);
}

void CommandBufferManager::submitCommands(PSemaphore signalSemaphore)
{
    if(activeCmdBuffer->state == CmdBuffer::State::InsideBegin 
    || activeCmdBuffer->state == CmdBuffer::State::RenderPassActive)
    {
        if(!activeCmdBuffer->state == CmdBuffer::State::RenderPassActive)
        {
            std::cout << "End of renderpass forced" << std::endl;
            activeCmdBuffer->endRenderPass();
        }
        activeCmdBuffer->end();
        if(signalSemaphore != nullptr)
        {
            queue->submitCommandBuffer(activeCmdBuffer, signalSemaphore->getHandle());
        }
        else
        {
            queue->submitCommandBuffer(activeCmdBuffer);
        }
    }
    for(int32_t i = 0; i < allocatedBuffers.size(); ++i)
    {
        PCmdBuffer cmdBuffer = allocatedBuffers[i];
        cmdBuffer->refreshFences();
        if(cmdBuffer->state == CmdBuffer::State::ReadyBegin)
        {
            activeCmdBuffer = cmdBuffer;
            activeCmdBuffer->begin();
            return;
        }
        else
        {
            assert(cmdBuffer->state == CmdBuffer::State::Submitted);
        }
    }
    activeCmdBuffer = new CmdBuffer(graphics, commandPool);
    allocatedBuffers.add(activeCmdBuffer);
    activeCmdBuffer->begin();
}
