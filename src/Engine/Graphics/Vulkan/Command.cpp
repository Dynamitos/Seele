#include "Command.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "Enums.h"
#include "Framebuffer.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Descriptor.h"
#include "Window.h"

using namespace Seele;
using namespace Seele::Vulkan;


Command::Command(PGraphics graphics, VkCommandPool cmdPool, PCommandPool pool)
    : graphics(graphics)
    , pool(pool)
    , owner(cmdPool)
{
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = owner,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle))

    fence = new Fence(graphics);
    signalSemaphore = new Semaphore(graphics);
    state = State::Init;
    //std::cout << "Cmd " << handle << " semaphore " << signalSemaphore->getHandle() << std::endl;
}

Command::~Command()
{
    vkFreeCommandBuffers(graphics->getDevice(), owner, 1, &handle);
    waitSemaphores.clear();
}

void Command::begin()
{
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };
    VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
    state = State::Begin;
}

void Command::end()
{
    VK_CHECK(vkEndCommandBuffer(handle));
    state = State::End;
}

void Command::beginRenderPass(PRenderPass renderPass, PFramebuffer framebuffer)
{
    assert(state == State::Begin);
    boundRenderPass = renderPass;
    boundFramebuffer = framebuffer;
    VkRenderPassBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = renderPass->getHandle(),
        .framebuffer = framebuffer->getHandle(),
        .renderArea = renderPass->getRenderArea(),
        .clearValueCount = (uint32)renderPass->getClearValueCount(),
        .pClearValues = renderPass->getClearValues(),
    };
    vkCmdBeginRenderPass(handle, &beginInfo, renderPass->getSubpassContents());
    state = State::RenderPass;
}

void Command::endRenderPass()
{
    boundRenderPass = nullptr;
    boundFramebuffer = nullptr;
    vkCmdEndRenderPass(handle);
    state = State::Begin;
}

void Command::executeCommands(const Array<Gfx::PRenderCommand>& commands)
{
    assert(state == State::RenderPass);
    if(commands.size() == 0)
    {
        //std::cout << "No commands provided" << std::endl;
        return;
    }
    Array<VkCommandBuffer> cmdBuffers(commands.size());
    for (uint32 i = 0; i < commands.size(); ++i)
    {
        auto command = commands[i].cast<RenderCommand>();
        command->end();
        executingRenders.add(command);
        for(auto& descriptor : command->boundDescriptors)
        {
            boundDescriptors.add(descriptor);
            //std::cout << "Cmd " << handle << " bound descriptor " << descriptor->getHandle() << std::endl;
        }
        cmdBuffers[i] = command->getHandle();
    }
    vkCmdExecuteCommands(handle, (uint32)cmdBuffers.size(), cmdBuffers.data());
}

void Command::executeCommands(const Array<Gfx::PComputeCommand>& commands) 
{
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
        for(auto& descriptor : command->boundDescriptors)
        {
            boundDescriptors.add(descriptor);
            //std::cout << "Cmd " << handle << " bound descriptor " << descriptor->getHandle() << std::endl;
        }
        cmdBuffers[i] = command->getHandle();
    }
    vkCmdExecuteCommands(handle, (uint32)cmdBuffers.size(), cmdBuffers.data());
}

void Command::waitForSemaphore(VkPipelineStageFlags flags, PSemaphore semaphore)
{
    waitSemaphores.add(semaphore);
    waitFlags.add(flags);
    //std::cout << "Cmd " << handle << " wait for " << semaphore->getHandle() << std::endl;
}

void Command::checkFence()
{
    assert(state == State::Submit || !fence->isSignaled());
    if (fence->isSignaled())
    {
        //std::cout << "Cmd " << handle << " was signaled" << std::endl;
        vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        fence->reset();
        for(auto& command : executingComputes)
        {
            command->reset();
        }
        executingComputes.clear();
        for(auto& command : executingRenders)
        {
            command->reset();
        }
        executingRenders.clear();
        for(auto& descriptor : boundDescriptors)
        {
            descriptor->unbind();
            //std::cout << "Cmd " << handle << " unbind " << descriptor->getHandle() << std::endl;
        }
        boundDescriptors.clear();
        graphics->getDestructionManager()->notifyCmdComplete(this);
        state = State::Init;
    }
}


void Command::waitForCommand(uint32 timeout)
{
    pool->submitCommands();
    if (state == State::Begin)
    {
        // is already done
        return;
    }
    fence->wait(timeout);
    checkFence();
}

PFence Command::getFence()
{
    return fence;
}

PCommandPool Command::getPool() 
{
    return pool;
}

RenderCommand::RenderCommand(PGraphics graphics, VkCommandPool cmdPool)
    : graphics(graphics)
    , owner(cmdPool)
{
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = owner,
        .level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = 1,
    };
    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle));
}


RenderCommand::~RenderCommand()
{
    vkFreeCommandBuffers(graphics->getDevice(), owner, 1, &handle);
}

void RenderCommand::begin(PRenderPass renderPass, PFramebuffer framebuffer)
{
    threadId = std::this_thread::get_id();
    ready = false;
    VkCommandBufferInheritanceInfo inheritanceInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        .renderPass = renderPass->getHandle(),
        .subpass = 0,
        .framebuffer = framebuffer->getHandle(),
        .occlusionQueryEnable = 0,
        .queryFlags = 0,
        .pipelineStatistics = 0,
    };
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
        .pInheritanceInfo = &inheritanceInfo,
    };
    VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
}

void RenderCommand::end() 
{
    VK_CHECK(vkEndCommandBuffer(handle));
}

void RenderCommand::reset() 
{
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
    VkRect2D scissors = {
        .offset = {
            .x = (int32)viewport->getOffsetX(),
            .y = (int32)viewport->getOffsetY(),
        },
        .extent = {
            .width = viewport->getWidth(),
            .height = viewport->getHeight(),
        },
    };
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
    assert(descriptor->writeDescriptors.size() == 0);
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
        assert(descriptorSet->writeDescriptors.size() == 0);
        descriptorSet->bind();

        boundDescriptors.add(descriptorSet.getHandle());
        sets[descriptorSet->getSetIndex()] = descriptorSet->getHandle();
    }
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, (uint32)descriptorSets.size(), sets, 0, nullptr);
    delete[] sets;
}
void RenderCommand::bindVertexBuffer(const Array<Gfx::PVertexBuffer>& streams)
{
    assert(threadId == std::this_thread::get_id());
    Array<VkBuffer> buffers(streams.size());
    Array<VkDeviceSize> offsets(streams.size());
    for(uint32 i = 0; i < streams.size(); ++i)
    {
        PVertexBuffer buf = streams[i].cast<VertexBuffer>();
        buffers[i] = buf->getHandle();
        offsets[i] = 0;
    };
    vkCmdBindVertexBuffers(handle, 0, (uint32)streams.size(), buffers.data(), offsets.data());
}
void RenderCommand::bindIndexBuffer(Gfx::PIndexBuffer indexBuffer)
{
    assert(threadId == std::this_thread::get_id());
    PIndexBuffer buf = indexBuffer.cast<IndexBuffer>();
    vkCmdBindIndexBuffer(handle, buf->getHandle(), 0, cast(buf->getIndexType()));
}

void RenderCommand::pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data)
{
    assert(threadId == std::this_thread::get_id());
    vkCmdPushConstants(handle, layout.cast<PipelineLayout>()->getHandle(), stage, offset, size, data);
}

void RenderCommand::draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) 
{
    assert(threadId == std::this_thread::get_id());
    vkCmdDraw(handle, vertexCount, instanceCount, firstVertex, firstInstance);
}

void RenderCommand::drawIndexed(uint32 indexCount, uint32 instanceCount, int32 firstIndex, uint32 vertexOffset, uint32 firstInstance) 
{
    assert(threadId == std::this_thread::get_id());
    vkCmdDrawIndexed(handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
void RenderCommand::dispatch(uint32 groupX, uint32 groupY, uint32 groupZ)
{
    assert(threadId == std::this_thread::get_id());
    graphics->vkCmdDrawMeshTasksEXT(handle, groupX, groupY, groupZ);
}

ComputeCommand::ComputeCommand(PGraphics graphics, VkCommandPool cmdPool) 
    : graphics(graphics)
    , owner(cmdPool)
{
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = owner,
        .level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = 1,
    };
    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle));
}


ComputeCommand::~ComputeCommand() 
{
    vkFreeCommandBuffers(graphics->getDevice(), owner, 1, &handle);
}

void ComputeCommand::begin() 
{
    threadId = std::this_thread::get_id();
    ready = false;
    VkCommandBufferInheritanceInfo inheritanceInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
        .framebuffer = VK_NULL_HANDLE,
        .occlusionQueryEnable = 0,
        .queryFlags = 0,
        .pipelineStatistics = 0,
    };
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = &inheritanceInfo,
    };
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
    assert(descriptor->writeDescriptors.size() == 0);
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
        assert(descriptorSet->writeDescriptors.size() == 0);
        descriptorSet->bind();

        //std::cout << "Binding descriptor " << descriptorSet->getHandle() << " to cmd " << handle << std::endl;
        boundDescriptors.add(descriptorSet.getHandle());
        sets[descriptorSet->getSetIndex()] = descriptorSet->getHandle();
    }
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getLayout(), 0, (uint32)descriptorSets.size(), sets, 0, nullptr);
    delete[] sets;
}

void ComputeCommand::pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data)
{
    assert(threadId == std::this_thread::get_id());
    vkCmdPushConstants(handle, layout.cast<PipelineLayout>()->getHandle(), stage, offset, size, data);
}

void ComputeCommand::dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) 
{
    assert(threadId == std::this_thread::get_id());
    vkCmdDispatch(handle, threadX, threadY, threadZ);
}

CommandPool::CommandPool(PGraphics graphics, PQueue queue)
    : graphics(graphics), queue(queue), queueFamilyIndex(queue->getFamilyIndex())
{
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue->getFamilyIndex(),
    };
    VK_CHECK(vkCreateCommandPool(graphics->getDevice(), &info, nullptr, &commandPool));

    allocatedBuffers.add(new Command(graphics, commandPool, this));
    
    command = allocatedBuffers.back();
    command->begin();
}

CommandPool::~CommandPool()
{
    vkDeviceWaitIdle(graphics->getDevice());
    for (auto& command : allocatedBuffers)
    {
        command->checkFence();
    }
    allocatedRenderCommands.clear();
    allocatedComputeCommands.clear();
    allocatedBuffers.clear();
    vkDestroyCommandPool(graphics->getDevice(), commandPool, nullptr);
    graphics = nullptr;
    queue = nullptr;
}

PCommand CommandPool::getCommands()
{
    return command;
}

PRenderCommand CommandPool::createRenderCommand(const std::string& name)
{
    for (uint32 i = 0; i < allocatedRenderCommands.size(); ++i)
    {
        PRenderCommand cmdBuffer = allocatedRenderCommands[i];
        if (cmdBuffer->isReady())
        {
            cmdBuffer->name = name;
            cmdBuffer->begin(command->boundRenderPass, command->boundFramebuffer);
            return cmdBuffer;
        }
    }
    allocatedRenderCommands.add(new RenderCommand(graphics, commandPool));
    PRenderCommand result = allocatedRenderCommands.back();
    result->name = name;
    result->begin(command->boundRenderPass, command->boundFramebuffer);
    return result;
}

PComputeCommand CommandPool::createComputeCommand(const std::string& name)
{
    for (uint32 i = 0; i < allocatedComputeCommands.size(); ++i)
    {
        PComputeCommand cmdBuffer = allocatedComputeCommands[i];
        if (cmdBuffer->isReady())
        {
            cmdBuffer->name = name;
            cmdBuffer->begin();
            return cmdBuffer;
        }
    }
    allocatedComputeCommands.add(new ComputeCommand(graphics, commandPool));
    PComputeCommand result = allocatedComputeCommands.back();
    result->name = name;
    result->begin();
    return result;
}

void CommandPool::submitCommands(PSemaphore signalSemaphore)
{
    assert(command->state == Command::State::Begin); // Not in a renderpass
    command->end();
    Array<VkSemaphore> semaphores = { command->signalSemaphore->getHandle() };
    if (signalSemaphore != nullptr)
    {
        semaphores.add(signalSemaphore->getHandle());
    }
    queue->submitCommandBuffer(command, semaphores);
    //std::cout << "Cmd " << command->getHandle() << " signalling " << command->signalSemaphore->getHandle() << std::endl;

    PSemaphore waitSemaphore = command->signalSemaphore;
    for (uint32 i = 0; i < allocatedBuffers.size(); ++i)
    {
        PCommand cmdBuffer = allocatedBuffers[i];
        cmdBuffer->checkFence();
        if (cmdBuffer->state == Command::State::Init)
        {
            command = cmdBuffer;
            command->begin();
            command->waitForSemaphore(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, waitSemaphore);
            return;
        }
        else
        {
            assert(cmdBuffer->state == Command::State::Submit);
        }
    }
    allocatedBuffers.add(new Command(graphics, commandPool, this));
    command = allocatedBuffers.back();
    command->begin();
    command->waitForSemaphore(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, waitSemaphore);
}
