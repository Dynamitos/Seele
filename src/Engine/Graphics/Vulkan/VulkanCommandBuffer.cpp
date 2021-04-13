#include "VulkanCommandBuffer.h"
#include "VulkanInitializer.h"
#include "VulkanGraphics.h"
#include "VulkanPipeline.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorSets.h"
#include "Graphics/MeshBatch.h"

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

CmdBuffer::CmdBuffer(PGraphics graphics, VkCommandPool cmdPool, PCommandBufferManager manager)
    : CmdBufferBase(graphics, cmdPool)
    , manager(manager)
    , renderPass(nullptr)
    , framebuffer(nullptr)
    , subpassIndex(0)
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
    beginInfo.clearValueCount = (uint32)renderPass->getClearValueCount();
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

void CmdBuffer::executeCommands(Array<Gfx::PRenderCommand> commands)
{
    assert(state == State::RenderPassActive);
    Array<VkCommandBuffer> cmdBuffers(commands.size());
    for (uint32 i = 0; i < commands.size(); ++i)
    {
        auto command = commands[i].cast<SecondaryCmdBuffer>();
        // Cache array and size to save on pointer access
        DescriptorSet** boundDescriptors = command->boundDescriptors.data();
        size_t numDescriptors = command->boundDescriptors.size();
        for(size_t i = 0; i < numDescriptors; ++i)
        {
            boundDescriptors[i]->currentlyBound = true;
        }
        command->end();
        executingCommands.add(command);
        cmdBuffers[i] = command->getHandle();
    }
    vkCmdExecuteCommands(handle, (uint32)cmdBuffers.size(), cmdBuffers.data());
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
            vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
            fence->reset();
            for(auto command : executingCommands)
            {
                command->reset();
            }
            executingCommands.clear();
            state = State::ReadyBegin;
        }
    }
    else
    {
        assert(!fence->isSignaled());
    }
}

PFence CmdBuffer::getFence()
{
    return fence;
}

PCommandBufferManager CmdBuffer::getManager() 
{
    return manager;
}

SecondaryCmdBuffer::SecondaryCmdBuffer(PGraphics graphics, VkCommandPool cmdPool)
    : CmdBufferBase(graphics, cmdPool)
    , ready(true)
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
    ready = false;
    VkCommandBufferBeginInfo beginInfo =
        init::CommandBufferBeginInfo();
    VkCommandBufferInheritanceInfo inheritanceInfo =
        init::CommandBufferInheritanceInfo();
    inheritanceInfo.framebuffer = parent->framebuffer->getHandle();
    inheritanceInfo.renderPass = parent->renderPass->getHandle();
    inheritanceInfo.subpass = parent->subpassIndex;
    beginInfo.pInheritanceInfo = &inheritanceInfo;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
}

void SecondaryCmdBuffer::end()
{
    VK_CHECK(vkEndCommandBuffer(handle));
}

void SecondaryCmdBuffer::reset() 
{
    vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    size_t numBoundDescriptors = boundDescriptors.size();
    DescriptorSet** boundDescriptorSets = boundDescriptors.data();
    for(size_t i = 0; i < numBoundDescriptors; ++i)
    {
        boundDescriptorSets[i]->currentlyBound = false;
    }
    boundDescriptors.clear();
    ready = true;
}

bool SecondaryCmdBuffer::isReady() 
{
    return ready;
}

void SecondaryCmdBuffer::setViewport(Gfx::PViewport viewport) 
{
    VkViewport vp = viewport.cast<Viewport>()->getHandle();
    VkRect2D scissors = init::Rect2D(viewport->getSizeX(), viewport->getSizeY(), viewport->getOffsetX(), viewport->getOffsetY());
    vkCmdSetViewport(handle, 0, 1, &vp);
    vkCmdSetScissor(handle, 0, 1, &scissors);
}

void SecondaryCmdBuffer::bindPipeline(Gfx::PGraphicsPipeline gfxPipeline)
{
    pipeline = gfxPipeline.cast<GraphicsPipeline>();
    pipeline->bind(handle);
}
void SecondaryCmdBuffer::bindDescriptor(Gfx::PDescriptorSet descriptorSet)
{
    auto descriptor = descriptorSet.cast<DescriptorSet>();
    boundDescriptors.add(descriptor.getHandle());
    VkDescriptorSet setHandle = descriptor->getHandle();
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), descriptorSet->getSetIndex(), 1, &setHandle, 0, nullptr);
}
void SecondaryCmdBuffer::bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets)
{
    VkDescriptorSet* sets = new VkDescriptorSet[descriptorSets.size()];
    for(uint32 i = 0; i < descriptorSets.size(); ++i)
    {
        auto descriptorSet = descriptorSets[i].cast<DescriptorSet>();
        boundDescriptors.add(descriptorSet.getHandle());
        sets[descriptorSet->getSetIndex()] = descriptorSet->getHandle();
    }
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, (uint32)descriptorSets.size(), sets, 0, nullptr);
    delete[] sets;
}
void SecondaryCmdBuffer::bindVertexBuffer(const Array<VertexInputStream>& streams)
{
    Array<VkBuffer> buffers(streams.size());
    Array<VkDeviceSize> offsets(streams.size());
    for(uint32 i = 0; i < streams.size(); ++i)
    {
        PVertexBuffer buf = streams[i].vertexBuffer.cast<VertexBuffer>();
        buffers[i] = buf->getHandle();
        offsets[i] = streams[i].offset;
    };
    vkCmdBindVertexBuffers(handle, 0, (uint32)streams.size(), buffers.data(), offsets.data());
}
void SecondaryCmdBuffer::bindIndexBuffer(Gfx::PIndexBuffer indexBuffer)
{
    PIndexBuffer buf = indexBuffer.cast<IndexBuffer>();
    vkCmdBindIndexBuffer(handle, buf->getHandle(), 0, cast(buf->getIndexType()));
}
void SecondaryCmdBuffer::draw(const MeshBatchElement& data) 
{
    vkCmdDrawIndexed(handle, data.indexBuffer->getNumIndices(), data.numInstances, data.minVertexIndex, data.baseVertexIndex, 0);
}

CommandBufferManager::CommandBufferManager(PGraphics graphics, PQueue queue)
    : graphics(graphics), queue(queue), queueFamilyIndex(queue->getFamilyIndex())
{
    VkCommandPoolCreateInfo info =
        init::CommandPoolCreateInfo();
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = queue->getFamilyIndex();

    VK_CHECK(vkCreateCommandPool(graphics->getDevice(), &info, nullptr, &commandPool));

    activeCmdBuffer = new CmdBuffer(graphics, commandPool, this);
    activeCmdBuffer->begin();
    std::lock_guard lock(allocatedBufferLock);
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
    std::scoped_lock lck(allocatedSecondBufferLock);
    for (uint32 i = 0; i < allocatedSecondBuffers.size(); ++i)
    {
        PSecondaryCmdBuffer cmdBuffer = allocatedSecondBuffers[i];
        if (cmdBuffer->isReady())
        {
            cmdBuffer->begin(activeCmdBuffer);
            return cmdBuffer;
        }
    }
    PSecondaryCmdBuffer result = new SecondaryCmdBuffer(graphics, commandPool);
    allocatedSecondBuffers.add(result);
    result->begin(activeCmdBuffer);
    return result;
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
    std::scoped_lock lock(allocatedBufferLock);
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
    activeCmdBuffer = new CmdBuffer(graphics, commandPool, this);
    allocatedBuffers.add(activeCmdBuffer);
    activeCmdBuffer->begin();
}

void CommandBufferManager::waitForCommands(PCmdBuffer cmdBuffer, uint32 timeout)
{
    cmdBuffer->fence->wait(timeout);
    cmdBuffer->refreshFence();
}
