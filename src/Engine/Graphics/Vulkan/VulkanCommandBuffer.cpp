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


CmdBuffer::CmdBuffer(PGraphics graphics, VkCommandPool cmdPool, PCommandBufferManager manager)
    : graphics(graphics)
    , manager(manager)
    , renderPass(nullptr)
    , framebuffer(nullptr)
    , subpassIndex(0)
    , owner(cmdPool)
{
    VkCommandBufferAllocateInfo allocInfo =
        init::CommandBufferAllocateInfo(cmdPool,
                                        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                        1);
    std::unique_lock lock(handleLock);
    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle))

    fence = new Fence(graphics);
    state = State::ReadyBegin;
}

CmdBuffer::~CmdBuffer()
{
    std::unique_lock lock(handleLock);
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
    std::unique_lock lock(handleLock);
    VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
    state = State::InsideBegin;
}

void CmdBuffer::end()
{
    std::unique_lock lock(handleLock);
    VK_CHECK(vkEndCommandBuffer(handle));
    state = State::Ended;
}

void CmdBuffer::beginRenderPass(PRenderPass newRenderPass, PFramebuffer newFramebuffer)
{
    assert(state == State::InsideBegin);
    std::unique_lock lock(handleLock);
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
    std::cout << "Beginning renderPass" << std::endl;
}

void CmdBuffer::endRenderPass()
{
    std::unique_lock lock(handleLock);
    vkCmdEndRenderPass(handle);
    state = State::InsideBegin;
    std::cout << "Ending renderPass" << std::endl;
}

void CmdBuffer::executeCommands(const Array<Gfx::PRenderCommand>& commands)
{
    assert(state == State::RenderPassActive);
    if(commands.size() == 0)
    {
        return;
    }
    std::unique_lock lock(handleLock);
    Array<VkCommandBuffer> cmdBuffers(commands.size());
    for (uint32 i = 0; i < commands.size(); ++i)
    {
        auto command = commands[i].cast<RenderCommand>();
        command->end();
        executingRenders.add(command);
        for(auto descriptor : command->boundDescriptors)
        {
            descriptor->free();
            boundDescriptors.add(descriptor);
        }
        cmdBuffers[i] = command->getHandle();
    }
    vkCmdExecuteCommands(handle, (uint32)cmdBuffers.size(), cmdBuffers.data());
}

void CmdBuffer::executeCommands(const Array<Gfx::PComputeCommand>& commands) 
{
    std::unique_lock lock(handleLock);
    if(commands.size() == 0)
    {
        return;
    }
    Array<VkCommandBuffer> cmdBuffers(commands.size());
    for (uint32 i = 0; i < commands.size(); ++i)
    {
        auto command = commands[i].cast<ComputeCommand>();
        command->end();
        executingComputes.add(command);
        for(auto descriptor : command->boundDescriptors)
        {
            descriptor->free();
            boundDescriptors.add(descriptor);
        }
        cmdBuffers[i] = command->getHandle();
    }
    vkCmdExecuteCommands(handle, (uint32)cmdBuffers.size(), cmdBuffers.data());
}

void CmdBuffer::addWaitSemaphore(VkPipelineStageFlags flags, PSemaphore semaphore)
{
    std::unique_lock lock(handleLock);
    waitSemaphores.add(semaphore);
    waitFlags.add(flags);
}

void CmdBuffer::refreshFence()
{
    std::unique_lock lock(handleLock);
    if (state == State::Submitted)
    {
        if (fence->isSignaled())
        {
            vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
            fence->reset();
            for(auto command : executingComputes)
            {
                command->reset();
            }
            executingComputes.clear();
            for(auto command : executingRenders)
            {
                command->reset();
            }
            executingRenders.clear();
            for(auto descriptor : boundDescriptors)
            {
                descriptor->unbind();
            }
            boundDescriptors.clear();
            state = State::ReadyBegin;
        }
    }
    else
    {
        assert(!fence->isSignaled());
    }
}

void CmdBuffer::waitForCommand(uint32 timeout)
{
    std::unique_lock lock(handleLock);
    fence->wait(timeout);
    refreshFence();
}

Event CmdBuffer::asyncWait() const
{
    return fence->asyncWait();
}


PFence CmdBuffer::getFence()
{
    return fence;
}

PCommandBufferManager CmdBuffer::getManager() 
{
    return manager;
}

RenderCommand::RenderCommand(PGraphics graphics, VkCommandPool cmdPool)
    : graphics(graphics)
    , owner(cmdPool)
{
    VkCommandBufferAllocateInfo allocInfo =
        init::CommandBufferAllocateInfo(cmdPool,
                                        VK_COMMAND_BUFFER_LEVEL_SECONDARY,
                                        1);
    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle));
}


RenderCommand::~RenderCommand()
{
    vkFreeCommandBuffers(graphics->getDevice(), owner, 1, &handle);
}

void RenderCommand::begin(PCmdBuffer parent)
{
    threadId = std::this_thread::get_id();
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

void RenderCommand::end() 
{
    assert(threadId == std::this_thread::get_id());
    VK_CHECK(vkEndCommandBuffer(handle));
}

void RenderCommand::reset() 
{
    assert(threadId == std::this_thread::get_id());
    vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    boundDescriptors.clear();
    ready = true;
}

bool RenderCommand::isReady() 
{
    return ready;
}

void RenderCommand::setViewport(Gfx::PViewport viewport) 
{
    assert(threadId == std::this_thread::get_id());
    VkViewport vp = viewport.cast<Viewport>()->getHandle();
    VkRect2D scissors = init::Rect2D(viewport->getSizeX(), viewport->getSizeY(), viewport->getOffsetX(), viewport->getOffsetY());
    vkCmdSetViewport(handle, 0, 1, &vp);
    vkCmdSetScissor(handle, 0, 1, &scissors);
}

void RenderCommand::bindPipeline(Gfx::PGraphicsPipeline gfxPipeline)
{
    assert(threadId == std::this_thread::get_id());
    pipeline = gfxPipeline.cast<GraphicsPipeline>();
    pipeline->bind(handle);
}
void RenderCommand::bindDescriptor(Gfx::PDescriptorSet descriptorSet)
{
    assert(threadId == std::this_thread::get_id());
    auto descriptor = descriptorSet.cast<DescriptorSet>();
    boundDescriptors.add(descriptor.getHandle());
    descriptor->bind();

    VkDescriptorSet setHandle = descriptor->getHandle();
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), descriptorSet->getSetIndex(), 1, &setHandle, 0, nullptr);
}
void RenderCommand::bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets)
{
    assert(threadId == std::this_thread::get_id());
    VkDescriptorSet* sets = new VkDescriptorSet[descriptorSets.size()];
    for(uint32 i = 0; i < descriptorSets.size(); ++i)
    {
        auto descriptorSet = descriptorSets[i].cast<DescriptorSet>();
        descriptorSet->bind();

        boundDescriptors.add(descriptorSet.getHandle());
        sets[descriptorSet->getSetIndex()] = descriptorSet->getHandle();
    }
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, (uint32)descriptorSets.size(), sets, 0, nullptr);
    delete[] sets;
}
void RenderCommand::bindVertexBuffer(const Array<VertexInputStream>& streams)
{
    assert(threadId == std::this_thread::get_id());
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
void RenderCommand::bindIndexBuffer(Gfx::PIndexBuffer indexBuffer)
{
    assert(threadId == std::this_thread::get_id());
    PIndexBuffer buf = indexBuffer.cast<IndexBuffer>();
    vkCmdBindIndexBuffer(handle, buf->getHandle(), 0, cast(buf->getIndexType()));
}
void RenderCommand::draw(const MeshBatchElement& data) 
{
    assert(threadId == std::this_thread::get_id());
    vkCmdDrawIndexed(handle, data.indexBuffer->getNumIndices(), data.numInstances, data.minVertexIndex, data.baseVertexIndex, 0);
}

void RenderCommand::draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) 
{
    assert(threadId == std::this_thread::get_id());
    vkCmdDraw(handle, vertexCount, instanceCount, firstVertex, firstInstance);
}

ComputeCommand::ComputeCommand(PGraphics graphics, VkCommandPool cmdPool) 
    : graphics(graphics)
    , owner(cmdPool)
{
    VkCommandBufferAllocateInfo allocInfo =
        init::CommandBufferAllocateInfo(cmdPool,
                                        VK_COMMAND_BUFFER_LEVEL_SECONDARY,
                                        1);
    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle));
}


ComputeCommand::~ComputeCommand() 
{
    vkFreeCommandBuffers(graphics->getDevice(), owner, 1, &handle);
}

void ComputeCommand::begin(PCmdBuffer) 
{
    threadId = std::this_thread::get_id();
    ready = false;
    VkCommandBufferBeginInfo beginInfo =
        init::CommandBufferBeginInfo();
    VkCommandBufferInheritanceInfo inheritanceInfo =
        init::CommandBufferInheritanceInfo();
    inheritanceInfo.framebuffer = VK_NULL_HANDLE;
    inheritanceInfo.renderPass = VK_NULL_HANDLE;
    inheritanceInfo.subpass = 0;
    beginInfo.pInheritanceInfo = &inheritanceInfo;
    VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
}

void ComputeCommand::end() 
{
    assert(threadId == std::this_thread::get_id());
    VK_CHECK(vkEndCommandBuffer(handle));
}

void ComputeCommand::reset() 
{
    assert(threadId == std::this_thread::get_id());
    vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    boundDescriptors.clear();
    ready = true;
}
bool ComputeCommand::isReady() 
{
    return ready;
}

void ComputeCommand::bindPipeline(Gfx::PComputePipeline computePipeline) 
{
    assert(threadId == std::this_thread::get_id());
    pipeline = computePipeline.cast<ComputePipeline>();
    pipeline->bind(handle);
}

void ComputeCommand::bindDescriptor(Gfx::PDescriptorSet descriptorSet) 
{
    assert(threadId == std::this_thread::get_id());
    auto descriptor = descriptorSet.cast<DescriptorSet>();
    boundDescriptors.add(descriptor.getHandle());
    descriptor->bind();
    
    VkDescriptorSet setHandle = descriptor->getHandle();
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getLayout(), descriptorSet->getSetIndex(), 1, &setHandle, 0, nullptr);
}

void ComputeCommand::bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets) 
{
    assert(threadId == std::this_thread::get_id());
    VkDescriptorSet* sets = new VkDescriptorSet[descriptorSets.size()];
    for(uint32 i = 0; i < descriptorSets.size(); ++i)
    {
        auto descriptorSet = descriptorSets[i].cast<DescriptorSet>();
        boundDescriptors.add(descriptorSet.getHandle());
        descriptorSet->bind();
        //std::cout << "Binding descriptor " << descriptorSet->getHandle() << " to cmd " << handle << std::endl;
        sets[descriptorSet->getSetIndex()] = descriptorSet->getHandle();
    }
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getLayout(), 0, (uint32)descriptorSets.size(), sets, 0, nullptr);
    delete[] sets;
}

void ComputeCommand::dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) 
{
    assert(threadId == std::this_thread::get_id());
    vkCmdDispatch(handle, threadX, threadY, threadZ);
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
    std::unique_lock lock(allocatedBufferLock);
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

PRenderCommand CommandBufferManager::createRenderCommand(const std::string& name)
{
    std::unique_lock lck(allocatedRenderLock);
    for (uint32 i = 0; i < allocatedRenderCommands.size(); ++i)
    {
        PRenderCommand cmdBuffer = allocatedRenderCommands[i];
        if (cmdBuffer->isReady())
        {
            cmdBuffer->name = name;
            cmdBuffer->begin(activeCmdBuffer);
            return cmdBuffer;
        }
    }
    PRenderCommand result = new RenderCommand(graphics, commandPool);
    result->name = name;
    result->begin(activeCmdBuffer);
    allocatedRenderCommands.add(result);
    return result;
}

PComputeCommand CommandBufferManager::createComputeCommand(const std::string& name)
{
    std::unique_lock lck(allocatedComputeLock);
    for (uint32 i = 0; i < allocatedComputeCommands.size(); ++i)
    {
        PComputeCommand cmdBuffer = allocatedComputeCommands[i];
        if (cmdBuffer->isReady())
        {
            cmdBuffer->name = name;
            cmdBuffer->begin(activeCmdBuffer);
            return cmdBuffer;
        }
    }
    PComputeCommand result = new ComputeCommand(graphics, commandPool);
    result->name = name;
    result->begin(activeCmdBuffer);
    allocatedComputeCommands.add(result);
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
    std::unique_lock lock(allocatedBufferLock);
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
