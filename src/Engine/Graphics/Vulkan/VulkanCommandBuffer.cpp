#include "VulkanCommandBuffer.h"
#include "VulkanInitializer.h"
#include "VulkanGraphics.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"

using namespace Seele;
using namespace Seele::Vulkan;

CmdBufferBase::CmdBufferBase(PGraphics graphics, VkCommandPool cmdPool)
    : graphics(graphics), owner(cmdPool)
{
}

CmdBufferBase::~CmdBufferBase()
{
    graphics = nullptr;
}

CmdBuffer::CmdBuffer(PGraphics graphics, VkCommandPool cmdPool)
    : CmdBufferBase(graphics, cmdPool), renderPass(nullptr), framebuffer(nullptr), subpassIndex(0)
{
    VkCommandBufferAllocateInfo allocInfo =
        init::CommandBufferAllocateInfo(cmdPool,
                                        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                        1);
    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle))

    fence = new Fence(graphics);
    state = State::ReadyBegin;
}

CmdBuffer::~CmdBuffer()
{
    vkFreeCommandBuffers(graphics->getDevice(), owner, 1, &handle);
    renderPass = nullptr;
    framebuffer = nullptr;
    waitSemaphores.clear();
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

void CmdBuffer::beginRenderPass(PRenderPass newRenderPass, PFramebuffer newFramebuffer)
{
    renderPass = newRenderPass;
    framebuffer = newFramebuffer;

    VkRenderPassBeginInfo beginInfo =
        init::RenderPassBeginInfo();
    beginInfo.clearValueCount = renderPass->getClearValueCount();
    beginInfo.pClearValues = renderPass->getClearValues();
    beginInfo.renderArea = renderPass->getRenderArea();
    beginInfo.renderPass = renderPass->getHandle();
    beginInfo.framebuffer = framebuffer->getHandle();
    vkCmdBeginRenderPass(handle, &beginInfo, renderPass->getSubpassContents());
    state = State::RenderPassActive;
}

void CmdBuffer::endRenderPass()
{
    vkCmdEndRenderPass(handle);
    state = State::InsideBegin;

}

void CmdBuffer::executeCommands(Array<PSecondaryCmdBuffer> commands)
{
    assert(state == State::RenderPassActive);
    Array<VkCommandBuffer> cmdBuffers(commands.size());
    for (uint32 i = 0; i < commands.size(); ++i)
    {
        cmdBuffers[i] = commands[i]->getHandle();
    }
    vkCmdExecuteCommands(handle, cmdBuffers.size(), cmdBuffers.data());
}

void CmdBuffer::addWaitSemaphore(VkPipelineStageFlags flags, PSemaphore semaphore)
{
    waitSemaphores.add(semaphore);
    waitFlags.add(flags);
}

void CmdBuffer::refreshFence()
{
    if (state == State::Submitted)
    {
        if (fence->isSignaled())
        {
            state = State::ReadyBegin;
            vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

            fence->reset();
        }
    }
    else
    {
        assert(!fence->isSignaled());
    }
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
    : graphics(graphics), queue(queue)
{
    VkCommandPoolCreateInfo info =
        init::CommandPoolCreateInfo();
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = queue->getFamilyIndex();

    VK_CHECK(vkCreateCommandPool(graphics->getDevice(), &info, nullptr, &commandPool));

    activeCmdBuffer = new CmdBuffer(graphics, commandPool);
    activeCmdBuffer->begin();
    allocatedBuffers.add(activeCmdBuffer);
}

CommandBufferManager::~CommandBufferManager()
{
    vkDestroyCommandPool(graphics->getDevice(), commandPool, nullptr);
    graphics = nullptr;
    queue = nullptr;
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
    if (activeCmdBuffer->state == CmdBuffer::State::InsideBegin || activeCmdBuffer->state == CmdBuffer::State::RenderPassActive)
    {
        if (activeCmdBuffer->state == CmdBuffer::State::RenderPassActive)
        {
            std::cout << "End of renderpass forced" << std::endl;
            activeCmdBuffer->endRenderPass();
        }
        activeCmdBuffer->end();
        if (signalSemaphore != nullptr)
        {
            queue->submitCommandBuffer(activeCmdBuffer, signalSemaphore->getHandle());
        }
        else
        {
            queue->submitCommandBuffer(activeCmdBuffer);
        }
    }
    for (uint32 i = 0; i < allocatedBuffers.size(); ++i)
    {
        PCmdBuffer cmdBuffer = allocatedBuffers[i];
        cmdBuffer->refreshFence();
        if (cmdBuffer->state == CmdBuffer::State::ReadyBegin)
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

void CommandBufferManager::waitForCommands(PCmdBuffer cmdBuffer, uint32 timeout)
{
    cmdBuffer->fence->wait(timeout);
    cmdBuffer->refreshFence();
}