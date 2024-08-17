#include "Command.h"
#include "Descriptor.h"
#include "Enums.h"
#include "Framebuffer.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "RayTracing.h"
#include "RenderPass.h"
#include "Window.h"

using namespace Seele;
using namespace Seele::Vulkan;

Command::Command(PGraphics graphics, PCommandPool pool) : graphics(graphics), pool(pool), statisticsFlags(0) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool->getHandle(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle))

    fence = new Fence(graphics);
    signalSemaphore = new Semaphore(graphics);
    state = State::Init;
    // std::cout << "Cmd " << handle << " semaphore " << signalSemaphore->getHandle() << std::endl;
}

Command::~Command() {
    vkFreeCommandBuffers(graphics->getDevice(), pool->getHandle(), 1, &handle);
    waitSemaphores.clear();
}

void Command::begin() {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };
    VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
    state = State::Begin;
}

void Command::end() {
    VK_CHECK(vkEndCommandBuffer(handle));
    state = State::End;
}

void Command::beginRenderPass(PRenderPass renderPass, PFramebuffer framebuffer) {
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

void Command::endRenderPass() {
    boundRenderPass->endRenderPass();
    boundRenderPass = nullptr;
    boundFramebuffer = nullptr;
    vkCmdEndRenderPass(handle);
    state = State::Begin;
}

void Command::executeCommands(Array<Gfx::ORenderCommand> commands) {
    if (commands.size() == 0) {
        // std::cout << "No commands provided" << std::endl;
        return;
    }
    Array<VkCommandBuffer> cmdBuffers(commands.size());
    for (uint32 i = 0; i < commands.size(); ++i) {
        auto command = Gfx::PRenderCommand(commands[i]).cast<RenderCommand>();
        command->end();
        for (auto& descriptor : command->boundResources) {
            boundResources.add(descriptor);
            // std::cout << "Cmd " << handle << " bound descriptor " << descriptor->getHandle() << std::endl;
        }
        cmdBuffers[i] = command->getHandle();
        executingRenders.add(std::move(commands[i]));
    }
    vkCmdExecuteCommands(handle, (uint32)cmdBuffers.size(), cmdBuffers.data());
}

void Command::executeCommands(Array<Gfx::OComputeCommand> commands) {
    if (commands.size() == 0) {
        return;
    }
    Array<VkCommandBuffer> cmdBuffers(commands.size());
    for (uint32 i = 0; i < commands.size(); ++i) {
        auto command = Gfx::PComputeCommand(commands[i]).cast<ComputeCommand>();
        command->end();
        for (auto& descriptor : command->boundResources) {
            boundResources.add(descriptor);
            // std::cout << "Cmd " << handle << " bound descriptor " << descriptor->getHandle() << std::endl;
        }
        cmdBuffers[i] = command->getHandle();
        executingComputes.add(std::move(commands[i]));
    }
    vkCmdExecuteCommands(handle, (uint32)cmdBuffers.size(), cmdBuffers.data());
}

void Command::waitForSemaphore(VkPipelineStageFlags flags, PSemaphore semaphore) {
    waitSemaphores.add(semaphore);
    waitFlags.add(flags);
    // std::cout << "Cmd " << handle << " wait for " << semaphore->getHandle() << std::endl;
}

void Command::checkFence() {
    assert(state == State::Submit || !fence->isSignaled());
    if (fence->isSignaled()) {
        // std::cout << "Cmd " << handle << " was signaled" << std::endl;
        vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        fence->reset();
        for (auto& command : executingComputes) {
            command->reset();
        }
        pool->cacheCommands(std::move(executingComputes));
        for (auto& command : executingRenders) {
            command->reset();
        }
        pool->cacheCommands(std::move(executingRenders));
        for (auto& descriptor : boundResources) {
            descriptor->unbind();
            // std::cout << "Cmd " << handle << " unbind " << descriptor->getHandle() << std::endl;
        }
        boundResources.clear();
        graphics->getDestructionManager()->notifyCommandComplete();
        state = State::Init;
    }
}

void Command::waitForCommand(uint32 timeout) {
    if (state == State::Begin) {
        // is already done
        return;
    }
    fence->wait(timeout);
    checkFence();
}

void Command::setPipelineStatisticsFlags(VkQueryPipelineStatisticFlags flags) { statisticsFlags = flags; }

void Command::bindResource(PCommandBoundResource resource) {
    resource->bind();
    boundResources.add(resource);
}

PFence Command::getFence() { return fence; }

PCommandPool Command::getPool() { return pool; }

RenderCommand::RenderCommand(PGraphics graphics, VkCommandPool cmdPool) : graphics(graphics), owner(cmdPool) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = owner,
        .level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = 1,
    };
    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle));
}

RenderCommand::~RenderCommand() { vkFreeCommandBuffers(graphics->getDevice(), owner, 1, &handle); }

void RenderCommand::begin(PRenderPass renderPass, PFramebuffer framebuffer, VkQueryPipelineStatisticFlags pipelineFlags) {
    threadId = std::this_thread::get_id();
    ready = false;
    VkCommandBufferInheritanceInfo inheritanceInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        .subpass = 0,
        .occlusionQueryEnable = 0,
        .queryFlags = 0,
        .pipelineStatistics = pipelineFlags,
    };
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .pInheritanceInfo = &inheritanceInfo,
    };
    if (renderPass != nullptr || framebuffer != nullptr) {
        inheritanceInfo.renderPass = renderPass->getHandle();
        inheritanceInfo.framebuffer = framebuffer->getHandle();
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
    VkDebugUtilsObjectNameInfoEXT nameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
        .objectHandle = (uint64)handle,
        .pObjectName = name.c_str(),
    };
    vkSetDebugUtilsObjectNameEXT(graphics->getDevice(), &nameInfo);
}

void RenderCommand::end() { VK_CHECK(vkEndCommandBuffer(handle)); }

void RenderCommand::reset() {
    vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    boundResources.clear();
    ready = true;
}

bool RenderCommand::isReady() { return ready; }

void RenderCommand::setViewport(Gfx::PViewport viewport) {
    assert(threadId == std::this_thread::get_id());
    VkViewport vp = viewport.cast<Viewport>()->getHandle();
    VkRect2D scissors = {
        .offset =
            {
                .x = (int32)viewport->getOffsetX(),
                .y = (int32)viewport->getOffsetY(),
            },
        .extent =
            {
                .width = viewport->getWidth(),
                .height = viewport->getHeight(),
            },
    };
    vkCmdSetViewport(handle, 0, 1, &vp);
    vkCmdSetScissor(handle, 0, 1, &scissors);
}

void RenderCommand::bindPipeline(Gfx::PGraphicsPipeline gfxPipeline) {
    assert(threadId == std::this_thread::get_id());
    pipeline = gfxPipeline.cast<GraphicsPipeline>();
    pipeline->bind(handle);
}

void RenderCommand::bindPipeline(Gfx::PRayTracingPipeline gfxPipeline) {
    assert(threadId == std::this_thread::get_id());
    rtPipeline = gfxPipeline.cast<RayTracingPipeline>();
    rtPipeline->bind(handle);
    boundResources.add(PBufferAllocation(rtPipeline->rayGen));
    boundResources.add(PBufferAllocation(rtPipeline->hit));
    boundResources.add(PBufferAllocation(rtPipeline->miss));
}

void RenderCommand::bindDescriptor(Gfx::PDescriptorSet descriptorSet, Array<uint32> dynamicOffsets) {
    assert(threadId == std::this_thread::get_id());
    auto descriptor = descriptorSet.cast<DescriptorSet>();
    assert(descriptor->writeDescriptors.size() == 0);
    descriptor->bind();
    boundResources.add(descriptor);
    for (auto binding : descriptor->boundResources) {
        for (auto res : binding) {
            res->bind();
            boundResources.add(res);
        }
    }

    VkDescriptorSet setHandle = descriptor->getHandle();
    Gfx::PPipelineLayout layout = pipeline != nullptr ? pipeline->getPipelineLayout() : rtPipeline->getPipelineLayout();
    vkCmdBindDescriptorSets(handle, pipeline != nullptr ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                            pipeline->getLayout(), layout->findParameter(descriptorSet->getName()), 1, &setHandle, dynamicOffsets.size(),
                            dynamicOffsets.data());
}

void RenderCommand::bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets, Array<uint32> dynamicOffsets) {
    assert(threadId == std::this_thread::get_id());
    VkDescriptorSet* sets = new VkDescriptorSet[descriptorSets.size()];
    std::memset(sets, 0, sizeof(VkDescriptorSet) * descriptorSets.size());
    Gfx::PPipelineLayout layout = pipeline != nullptr ? pipeline->getPipelineLayout() : rtPipeline->getPipelineLayout();
    for (uint32 i = 0; i < descriptorSets.size(); ++i) {
        auto descriptorSet = descriptorSets[i].cast<DescriptorSet>();
        assert(descriptorSet->writeDescriptors.size() == 0);
        descriptorSet->bind();
        boundResources.add(descriptorSet);

        for (auto binding : descriptorSet->boundResources) {
            for (auto res : binding) {
                // partially bound descriptors can include nulls
                if (res != nullptr) {
                    res->bind();
                    boundResources.add(res);
                }
            }
        }
        sets[layout->findParameter(descriptorSet->getName())] = descriptorSet->getHandle();
    }
    vkCmdBindDescriptorSets(handle, pipeline != nullptr ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                            pipeline != nullptr ? pipeline->getLayout() : rtPipeline->getLayout(), 0, (uint32)descriptorSets.size(), sets,
                            dynamicOffsets.size(), dynamicOffsets.data());
    delete[] sets;
}

void RenderCommand::bindVertexBuffer(const Array<Gfx::PVertexBuffer>& streams) {
    assert(threadId == std::this_thread::get_id());
    Array<VkBuffer> buffers(streams.size());
    Array<VkDeviceSize> offsets(streams.size());
    for (uint32 i = 0; i < streams.size(); ++i) {
        PVertexBuffer buf = streams[i].cast<VertexBuffer>();
        buffers[i] = buf->getHandle();
        offsets[i] = 0;
        buf->getAlloc()->bind();
        boundResources.add(buf->getAlloc());
    };
    vkCmdBindVertexBuffers(handle, 0, (uint32)streams.size(), buffers.data(), offsets.data());
}

void RenderCommand::bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) {
    assert(threadId == std::this_thread::get_id());
    PIndexBuffer buf = indexBuffer.cast<IndexBuffer>();
    buf->getAlloc()->bind();
    boundResources.add(buf->getAlloc());
    vkCmdBindIndexBuffer(handle, buf->getHandle(), 0, cast(buf->getIndexType()));
}

void RenderCommand::pushConstants(Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) {
    assert(threadId == std::this_thread::get_id());
    vkCmdPushConstants(handle, pipeline->getLayout(), stage, offset, size, data);
}

void RenderCommand::draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) {
    assert(threadId == std::this_thread::get_id());
    vkCmdDraw(handle, vertexCount, instanceCount, firstVertex, firstInstance);
}

void RenderCommand::drawIndexed(uint32 indexCount, uint32 instanceCount, int32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
    assert(threadId == std::this_thread::get_id());
    vkCmdDrawIndexed(handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
void RenderCommand::drawMesh(uint32 groupX, uint32 groupY, uint32 groupZ) {
    assert(threadId == std::this_thread::get_id());
    vkCmdDrawMeshTasksEXT(handle, groupX, groupY, groupZ);
}

void RenderCommand::drawMeshIndirect(Gfx::PShaderBuffer buffer, uint64 offset, uint32 drawCount, uint32 stride) {
    assert(threadId == std::this_thread::get_id());
    vkCmdDrawMeshTasksIndirectEXT(handle, buffer.cast<ShaderBuffer>()->getHandle(), offset, drawCount, stride);
}

void RenderCommand::traceRays(uint32 width, uint32 height, uint32 depth) {
    VkStridedDeviceAddressRegionKHR rayGenRef = rtPipeline->getRayGenRegion();
    VkStridedDeviceAddressRegionKHR hitRef = rtPipeline->getHitRegion();
    VkStridedDeviceAddressRegionKHR missRef = rtPipeline->getMissRegion();
    VkStridedDeviceAddressRegionKHR callableRef = rtPipeline->getCallableRegion();
    vkCmdTraceRaysKHR(handle, &rayGenRef, &missRef, &hitRef, &callableRef, width, height, depth);
}

ComputeCommand::ComputeCommand(PGraphics graphics, VkCommandPool cmdPool) : graphics(graphics), owner(cmdPool) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = owner,
        .level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = 1,
    };
    VK_CHECK(vkAllocateCommandBuffers(graphics->getDevice(), &allocInfo, &handle));
}

ComputeCommand::~ComputeCommand() { vkFreeCommandBuffers(graphics->getDevice(), owner, 1, &handle); }

void ComputeCommand::begin(VkQueryPipelineStatisticFlags pipelineFlags) {
    threadId = std::this_thread::get_id();
    ready = false;
    VkCommandBufferInheritanceInfo inheritanceInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
        .framebuffer = VK_NULL_HANDLE,
        .occlusionQueryEnable = 0,
        .queryFlags = 0,
        .pipelineStatistics = pipelineFlags,
    };
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = &inheritanceInfo,
    };
    VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
    VkDebugUtilsObjectNameInfoEXT nameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
        .objectHandle = (uint64)handle,
        .pObjectName = name.c_str(),
    };
    vkSetDebugUtilsObjectNameEXT(graphics->getDevice(), &nameInfo);
}

void ComputeCommand::end() {
    assert(threadId == std::this_thread::get_id());
    VK_CHECK(vkEndCommandBuffer(handle));
}

void ComputeCommand::reset() {
    assert(threadId == std::this_thread::get_id());
    vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    boundResources.clear();
    ready = true;
}
bool ComputeCommand::isReady() { return ready; }

void ComputeCommand::bindPipeline(Gfx::PComputePipeline computePipeline) {
    assert(threadId == std::this_thread::get_id());
    pipeline = computePipeline.cast<ComputePipeline>();
    pipeline->bind(handle);
}

void ComputeCommand::bindDescriptor(Gfx::PDescriptorSet descriptorSet, Array<uint32> dynamicOffsets) {
    assert(threadId == std::this_thread::get_id());
    auto descriptor = descriptorSet.cast<DescriptorSet>();
    assert(descriptor->writeDescriptors.size() == 0);
    descriptor->bind();
    boundResources.add(descriptor.getHandle());

    for (const auto& binding : descriptor->boundResources) {
        for (auto res : binding) {
            res->bind();
            boundResources.add(res);
        }
    }

    VkDescriptorSet setHandle = descriptor->getHandle();
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getLayout(),
                            pipeline->getPipelineLayout()->findParameter(descriptorSet->getName()), 1, &setHandle, dynamicOffsets.size(),
                            dynamicOffsets.data());
}

void ComputeCommand::bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets, Array<uint32> dynamicOffsets) {
    assert(threadId == std::this_thread::get_id());
    VkDescriptorSet* sets = new VkDescriptorSet[descriptorSets.size()];
    for (uint32 i = 0; i < descriptorSets.size(); ++i) {
        auto descriptorSet = descriptorSets[i].cast<DescriptorSet>();
        assert(descriptorSet->writeDescriptors.size() == 0);
        descriptorSet->bind();
        boundResources.add(descriptorSet.getHandle());

        // std::cout << "Binding descriptor " << descriptorSet->getHandle() << " to cmd " << handle << std::endl;

        for (auto binding : descriptorSet->boundResources) {
            for (auto res : binding) {
                res->bind();
                boundResources.add(res);
            }
        }
        sets[pipeline->getPipelineLayout()->findParameter(descriptorSet->getName())] = descriptorSet->getHandle();
    }
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getLayout(), 0, (uint32)descriptorSets.size(), sets,
                            dynamicOffsets.size(), dynamicOffsets.data());
    delete[] sets;
}

void ComputeCommand::pushConstants(Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) {
    assert(threadId == std::this_thread::get_id());
    vkCmdPushConstants(handle, pipeline->getLayout(), stage, offset, size, data);
}

void ComputeCommand::dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) {
    assert(threadId == std::this_thread::get_id());
    vkCmdDispatch(handle, threadX, threadY, threadZ);
}

CommandPool::CommandPool(PGraphics graphics, PQueue queue) : graphics(graphics), queue(queue), queueFamilyIndex(queue->getFamilyIndex()) {
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue->getFamilyIndex(),
    };
    VK_CHECK(vkCreateCommandPool(graphics->getDevice(), &info, nullptr, &commandPool));
    // TODO: dont reset individual commands, reset pool instead
    allocatedBuffers.add(new Command(graphics, this));

    command = allocatedBuffers.back();
    command->begin();
}

CommandPool::~CommandPool() {
    submitCommands();
    vkDeviceWaitIdle(graphics->getDevice());
    for (auto& cmd : allocatedBuffers) {
        cmd->checkFence();
    }
    allocatedRenderCommands.clear();
    allocatedComputeCommands.clear();
    allocatedBuffers.clear();
    vkDestroyCommandPool(graphics->getDevice(), commandPool, nullptr);
    graphics = nullptr;
    queue = nullptr;
}

PCommand CommandPool::getCommands() { return command; }

void CommandPool::cacheCommands(Array<ORenderCommand> commands) {
    for (auto&& cmd : commands) {
        allocatedRenderCommands.add(std::move(cmd));
    }
}

void CommandPool::cacheCommands(Array<OComputeCommand> commands) {
    for (auto&& cmd : commands) {
        allocatedComputeCommands.add(std::move(cmd));
    }
}

ORenderCommand CommandPool::createRenderCommand(const std::string& name) {
    for (uint32 i = 0; i < allocatedRenderCommands.size(); ++i) {
        if (allocatedRenderCommands[i]->isReady()) {
            ORenderCommand cmdBuffer = std::move(allocatedRenderCommands[i]);
            allocatedRenderCommands.removeAt(i, false);
            cmdBuffer->name = name;
            cmdBuffer->begin(command->boundRenderPass, command->boundFramebuffer, command->statisticsFlags);
            return cmdBuffer;
        }
    }
    ORenderCommand result = new RenderCommand(graphics, commandPool);
    result->name = name;
    result->begin(command->boundRenderPass, command->boundFramebuffer, command->statisticsFlags);
    return result;
}

OComputeCommand CommandPool::createComputeCommand(const std::string& name) {
    for (uint32 i = 0; i < allocatedComputeCommands.size(); ++i) {
        if (allocatedComputeCommands[i]->isReady()) {
            OComputeCommand cmdBuffer = std::move(allocatedComputeCommands[i]);
            allocatedComputeCommands.removeAt(i, false);
            cmdBuffer->name = name;
            cmdBuffer->begin(command->statisticsFlags);
            return cmdBuffer;
        }
    }
    OComputeCommand result = new ComputeCommand(graphics, commandPool);
    result->name = name;
    result->begin(command->statisticsFlags);
    return result;
}

void CommandPool::submitCommands(PSemaphore signalSemaphore) {
    assert(command->state == Command::State::Begin); // Not in a renderpass
    command->end();
    Array<VkSemaphore> semaphores = {command->signalSemaphore->getHandle()};
    if (signalSemaphore != nullptr) {
        semaphores.add(signalSemaphore->getHandle());
    }
    queue->submitCommandBuffer(command, semaphores);
    // std::cout << "Cmd " << command->getHandle() << " signalling " << command->signalSemaphore->getHandle() << std::endl;

    PSemaphore waitSemaphore = command->signalSemaphore;
    for (uint32 i = 0; i < allocatedBuffers.size(); ++i) {
        PCommand cmdBuffer = allocatedBuffers[i];
        cmdBuffer->checkFence();
        if (cmdBuffer->state == Command::State::Init) {
            command = cmdBuffer;
            command->begin();
            command->waitForSemaphore(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, waitSemaphore);
            return;
        } else {
            assert(cmdBuffer->state == Command::State::Submit);
        }
    }
    allocatedBuffers.add(new Command(graphics, this));
    command = allocatedBuffers.back();
    command->begin();
    command->waitForSemaphore(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, waitSemaphore);
}

void CommandPool::refreshCommands() {
    for (uint32 i = 0; i < allocatedBuffers.size(); ++i)
    {
        allocatedBuffers[i]->checkFence();
    }
}
