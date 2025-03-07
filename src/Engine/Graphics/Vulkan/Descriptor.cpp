#include "Descriptor.h"
#include "Buffer.h"
#include "CRC.h"
#include "Command.h"
#include "Graphics.h"
#include "Graphics/Buffer.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "RayTracing.h"
#include "Texture.h"
#include "vulkan/vulkan_core.h"
#include <algorithm>
#include <ranges>

using namespace Seele;
using namespace Seele::Vulkan;

DescriptorLayout::DescriptorLayout(PGraphics graphics, const std::string& name)
    : Gfx::DescriptorLayout(name), graphics(graphics), layoutHandle(VK_NULL_HANDLE) {}

DescriptorLayout::~DescriptorLayout() {
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(pool));
    if (layoutHandle != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(graphics->getDevice(), layoutHandle, nullptr);
    }
}

void DescriptorLayout::create() {
    if (layoutHandle != VK_NULL_HANDLE) {
        return;
    }
    Array<VkDescriptorBindingFlags> bindingFlags;
    if (std::ranges::contains(descriptorBindings, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &Gfx::DescriptorBinding::descriptorType)) {
        bindings.add({
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL, // todo
            .pImmutableSamplers = nullptr,
        });
        bindingFlags.add(0);
    }
    for (const auto& gfxBinding : descriptorBindings) {
        if (gfxBinding.descriptorType == Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            mappings[gfxBinding.name] = {
                .binding = 0,
                .constantOffset = constantsSize,
                .constantSize = gfxBinding.uniformLength,
            };
            constantsSize += gfxBinding.uniformLength;
            constantsStages |= gfxBinding.shaderStages;
        } else {
            mappings[gfxBinding.name] = {
                .binding = (uint32)bindings.size(),
                .type = cast(gfxBinding.descriptorType),
            };
            bindings.add({
                .binding = (uint32)bindings.size(),
                .descriptorType = cast(gfxBinding.descriptorType),
                .descriptorCount = gfxBinding.descriptorCount,
                .stageFlags = gfxBinding.shaderStages,
                .pImmutableSamplers = nullptr,
            });
            bindingFlags.add(gfxBinding.bindingFlags);
        }
    }
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .pNext = nullptr,
        .bindingCount = static_cast<uint32>(bindingFlags.size()),
        .pBindingFlags = bindingFlags.data(),
    };
    VkDescriptorSetLayoutCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &bindingFlagsInfo,
        .flags = 0,
        .bindingCount = (uint32)bindings.size(),
        .pBindings = bindings.data(),
    };
    VK_CHECK(vkCreateDescriptorSetLayout(graphics->getDevice(), &createInfo, nullptr, &layoutHandle));

    pool = new DescriptorPool(graphics, this);

    hash = CRC::Calculate(bindings.data(), sizeof(VkDescriptorSetLayoutBinding) * bindings.size(), CRC::CRC_32());
}

DescriptorPool::DescriptorPool(PGraphics graphics, PDescriptorLayout layout)
    : CommandBoundResource(graphics, layout->getName()), graphics(graphics), layout(layout) {
    for (uint32 i = 0; i < cachedHandles.size(); ++i) {
        cachedHandles[i] = nullptr;
    }

    Map<Gfx::SeDescriptorType, uint32> perTypeSizes;
    for (uint32 i = 0; i < layout->getBindings().size(); ++i) {
        auto& binding = layout->getBindings()[i];
        perTypeSizes[binding.descriptorType] += 512;
    }
    Array<VkDescriptorPoolSize> poolSizes;
    for (const auto [type, num] : perTypeSizes) {
        VkDescriptorPoolSize size;
        size.descriptorCount = num;
        size.type = cast(type);
        poolSizes.add(size);
    }
    VkDescriptorPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = maxSets * Gfx::numFramesBuffered,
        .poolSizeCount = (uint32)poolSizes.size(),
        .pPoolSizes = poolSizes.data(),
    };
    VK_CHECK(vkCreateDescriptorPool(graphics->getDevice(), &createInfo, nullptr, &poolHandle));
}

DescriptorPool::~DescriptorPool() {
    for (size_t i = 0; i < maxSets; ++i) {
        if (cachedHandles[i] != nullptr) {
            graphics->getDestructionManager()->queueResourceForDestruction(std::move(cachedHandles[i]));
        }
    }
    vkDestroyDescriptorPool(graphics->getDevice(), poolHandle, nullptr);
    if (nextAlloc) {
        delete nextAlloc;
    }
}

Gfx::PDescriptorSet DescriptorPool::allocateDescriptorSet() {
    VkDescriptorSetLayout layoutHandle = layout->getHandle();
    VkDescriptorSetVariableDescriptorCountAllocateInfo setCounts = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorSetCount = 1,
    };
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = poolHandle,
        .descriptorSetCount = 1,
        .pSetLayouts = &layoutHandle,
    };
    uint32 counts = 0;
    for (const auto& binding : layout->bindings) {
        if (binding.descriptorCount > 0) {
            counts = binding.descriptorCount;
        }
    }
    if (layout->bindings.size() > 0) {
        setCounts.pDescriptorCounts = &counts;
        allocInfo.pNext = &setCounts;
    }
    for (uint32 setIndex = 0; setIndex < cachedHandles.size(); ++setIndex) {
        if (cachedHandles[setIndex] == nullptr) {
            cachedHandles[setIndex] = new DescriptorSet(graphics, this);
        }
        if (cachedHandles[setIndex]->isCurrentlyBound()) {
            // Currently in use, skip
            continue;
        }
        if (cachedHandles[setIndex]->getHandle() == VK_NULL_HANDLE) {
            // If it hasnt been initialized, allocate it
            VK_CHECK(vkAllocateDescriptorSets(graphics->getDevice(), &allocInfo, &cachedHandles[setIndex]->setHandle));
        }

        PDescriptorSet vulkanSet = cachedHandles[setIndex];

        // Found set, stop searching
        return vulkanSet;
    }
    if (nextAlloc == nullptr) {
        nextAlloc = new DescriptorPool(graphics, layout);
    }
    // std::cout << "Out of descriptors, forwarding" << std::endl;
    return nextAlloc->allocateDescriptorSet();
    // throw std::logic_error("Out of descriptor sets");
}

void DescriptorPool::reset() {
    for (uint32 i = 0; i < cachedHandles.size(); ++i) {
        if (cachedHandles[i] == nullptr) {
            return;
        }
    }
    if (nextAlloc != nullptr) {
        nextAlloc->reset();
    }
}

DescriptorSet::DescriptorSet(PGraphics graphics, PDescriptorPool owner)
    : Gfx::DescriptorSet(owner->getLayout()), CommandBoundResource(graphics, owner->getLayout()->getName()), setHandle(VK_NULL_HANDLE),
      graphics(graphics), owner(owner) {
    boundResources.resize(owner->getLayout()->getBindings().size());
    for (uint32 i = 0; i < boundResources.size(); ++i) {
        boundResources[i].resize(owner->getLayout()->getBindings()[i].descriptorCount);
    }
    constantData.resize(owner->getLayout()->constantsSize);
}

DescriptorSet::~DescriptorSet() {}

void DescriptorSet::updateConstants(const std::string& name, uint32 offset, void* data) {
    std::memcpy(constantData.data() + owner->getLayout()->mappings[name].constantOffset, (char*)data + offset,
                owner->getLayout()->mappings[name].constantSize);
}

void DescriptorSet::updateBuffer(const std::string& name, uint32 index, Gfx::PShaderBuffer shaderBuffer) {
    PShaderBuffer vulkanBuffer = shaderBuffer.cast<ShaderBuffer>();
    const auto& map = owner->getLayout()->mappings[name];
    uint32 binding = map.binding;
    if (boundResources[binding][index] == vulkanBuffer->getAlloc() || vulkanBuffer->getAlloc() == nullptr) {
        return;
    }
    bufferInfos.add(VkDescriptorBufferInfo{
        .buffer = vulkanBuffer->getHandle(),
        .offset = 0,
        .range = vulkanBuffer->getSize(),
    });
    writeDescriptors.add(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = setHandle,
        .dstBinding = binding,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = map.type,
        .pBufferInfo = &bufferInfos.back(),
    });

    boundResources[binding][index] = vulkanBuffer->getAlloc();
}

void DescriptorSet::updateBuffer(const std::string& name, uint32 index, Gfx::PVertexBuffer indexBuffer) {
    PVertexBuffer vulkanBuffer = indexBuffer.cast<VertexBuffer>();
    const auto& map = owner->getLayout()->mappings[name];
    uint32 binding = map.binding;
    if (boundResources[binding][index] == vulkanBuffer->getAlloc() || vulkanBuffer->getAlloc() == nullptr) {
        return;
    }

    bufferInfos.add(VkDescriptorBufferInfo{
        .buffer = vulkanBuffer->getHandle(),
        .offset = 0,
        .range = vulkanBuffer->getSize(),
    });
    writeDescriptors.add(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = setHandle,
        .dstBinding = binding,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = map.type,
        .pBufferInfo = &bufferInfos.back(),
    });

    boundResources[binding][index] = vulkanBuffer->getAlloc();
}

void DescriptorSet::updateBuffer(const std::string& name, uint32 index, Gfx::PIndexBuffer indexBuffer) {
    PIndexBuffer vulkanBuffer = indexBuffer.cast<IndexBuffer>();
    const auto& map = owner->getLayout()->mappings[name];
    uint32 binding = map.binding;
    if (boundResources[binding][index] == vulkanBuffer->getAlloc() || vulkanBuffer->getAlloc() == nullptr) {
        return;
    }

    bufferInfos.add(VkDescriptorBufferInfo{
        .buffer = vulkanBuffer->getHandle(),
        .offset = 0,
        .range = vulkanBuffer->getSize(),
    });
    writeDescriptors.add(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = setHandle,
        .dstBinding = binding,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = map.type,
        .pBufferInfo = &bufferInfos.back(),
    });

    boundResources[binding][index] = vulkanBuffer->getAlloc();
}

void DescriptorSet::updateSampler(const std::string& name, uint32 index, Gfx::PSampler samplerState) {
    PSampler vulkanSampler = samplerState.cast<Sampler>();
    const auto& map = owner->getLayout()->mappings[name];
    uint32 binding = map.binding;
    if (boundResources[binding][index] == vulkanSampler->getHandle()) {
        return;
    }

    imageInfos.add(VkDescriptorImageInfo{
        .sampler = vulkanSampler->getSampler(),
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    });

    writeDescriptors.add(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = setHandle,
        .dstBinding = binding,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = map.type,
        .pImageInfo = &imageInfos.back(),
    });

    boundResources[binding][index] = vulkanSampler->getHandle();
}

void DescriptorSet::updateTexture(const std::string& name, uint32 index, Gfx::PTexture2D texture) {
    TextureBase* vulkanTexture = texture.cast<TextureBase>().getHandle();
    const auto& map = owner->getLayout()->mappings[name];
    uint32 binding = map.binding;
    if (boundResources[binding][index] == vulkanTexture->getHandle()) {
        return;
    }

    // It is assumed that the image is in the correct layout
    imageInfos.add(VkDescriptorImageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = vulkanTexture->getView(),
        .imageLayout = cast(vulkanTexture->getLayout()),
    });
    writeDescriptors.add(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = setHandle,
        .dstBinding = binding,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = map.type,
        .pImageInfo = &imageInfos.back(),
    });

    boundResources[binding][index] = vulkanTexture->getHandle();
}

void DescriptorSet::updateTexture(const std::string& name, uint32 index, Gfx::PTexture3D texture) {
    TextureBase* vulkanTexture = texture.cast<TextureBase>().getHandle();
    const auto& map = owner->getLayout()->mappings[name];
    uint32 binding = map.binding;
    if (boundResources[binding][index] == vulkanTexture->getHandle()) {
        return;
    }

    // It is assumed that the image is in the correct layout
    imageInfos.add(VkDescriptorImageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = vulkanTexture->getView(),
        .imageLayout = cast(vulkanTexture->getLayout()),
    });
    writeDescriptors.add(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = setHandle,
        .dstBinding = binding,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = map.type,
        .pImageInfo = &imageInfos.back(),
    });

    boundResources[binding][index] = vulkanTexture->getHandle();
}

void DescriptorSet::updateTexture(const std::string& name, uint32 index, Gfx::PTextureCube texture) {
    TextureBase* vulkanTexture = texture.cast<TextureBase>().getHandle();
    const auto& map = owner->getLayout()->mappings[name];
    uint32 binding = map.binding;
    if (boundResources[binding][index] == vulkanTexture->getHandle()) {
        return;
    }

    // It is assumed that the image is in the correct layout
    imageInfos.add(VkDescriptorImageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = vulkanTexture->getView(),
        .imageLayout = cast(vulkanTexture->getLayout()),
    });
    writeDescriptors.add(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = setHandle,
        .dstBinding = binding,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = map.type,
        .pImageInfo = &imageInfos.back(),
    });

    boundResources[binding][index] = vulkanTexture->getHandle();
}

void DescriptorSet::updateAccelerationStructure(const std::string& name, uint32 index, Gfx::PTopLevelAS as) {
    auto tlas = as.cast<TopLevelAS>();
    uint32 binding = owner->getLayout()->mappings[name].binding;
    accelerationInfos.add(VkWriteDescriptorSetAccelerationStructureKHR{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
        .pNext = nullptr,
        .accelerationStructureCount = 1,
        .pAccelerationStructures = &tlas->handle,
    });
    writeDescriptors.add(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = &accelerationInfos.back(),
        .dstSet = setHandle,
        .dstBinding = binding,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
    });
}

void DescriptorSet::writeChanges() {
    if (constantData.size() > 0) {
        if (constantsBuffer != nullptr)
        {
            graphics->getDestructionManager()->queueResourceForDestruction(std::move(constantsBuffer));
        }
        constantsBuffer = new BufferAllocation(graphics, owner->getLayout()->getName(),
                                               VkBufferCreateInfo{
                                                   .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                                   .pNext = nullptr,
                                                   .size = constantData.size(),
                                                   .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                               },
                                               VmaAllocationCreateInfo{
                                                   .usage = VMA_MEMORY_USAGE_AUTO,
                                               },
                                               Gfx::QueueType::GRAPHICS);
        constantsBuffer->updateContents(0, constantData.size(), constantData.data());
        constantsBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                         Gfx::SE_ACCESS_UNIFORM_READ_BIT, Gfx::SE_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        bufferInfos.add(VkDescriptorBufferInfo{
            .buffer = constantsBuffer->buffer,
            .offset = 0,
            .range = constantsBuffer->size,
        });
        writeDescriptors.add(VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = setHandle,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &bufferInfos.back(),
        });
    }
    
    if (writeDescriptors.size() > 0) {
        if (isCurrentlyBound()) {
            std::cout << "Descriptor currently bound, allocate a new one instead" << std::endl;
            assert(!isCurrentlyBound());
        }
        vkUpdateDescriptorSets(graphics->getDevice(), (uint32)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
        writeDescriptors.clear();
        imageInfos.clear();
        bufferInfos.clear();
    }
}

PipelineLayout::PipelineLayout(PGraphics graphics, const std::string& name, Gfx::PPipelineLayout baseLayout)
    : Gfx::PipelineLayout(name, baseLayout), graphics(graphics), layoutHandle(VK_NULL_HANDLE) {}

PipelineLayout::~PipelineLayout() {}

std::mutex layoutLock;
Map<uint32, VkPipelineLayout> cachedLayouts;

void PipelineLayout::create() {
    for (auto [name, desc] : descriptorSetLayouts) {
        PDescriptorLayout layout = desc.cast<DescriptorLayout>();
        layout->create();
        uint32 parameterIndex = parameterMapping[layout->getName()];
        if (parameterIndex >= vulkanDescriptorLayouts.size()) {
            vulkanDescriptorLayouts.resize(parameterIndex + 1);
        }
        vulkanDescriptorLayouts[parameterIndex] = layout->getHandle();
    }

    Array<VkPushConstantRange> vkPushConstants(pushConstants.size());
    for (size_t i = 0; i < pushConstants.size(); i++) {
        vkPushConstants[i].offset = pushConstants[i].offset;
        vkPushConstants[i].size = pushConstants[i].size;
        vkPushConstants[i].stageFlags = pushConstants[i].stageFlags;
    }

    VkPipelineLayoutCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = (uint32)vulkanDescriptorLayouts.size(),
        .pSetLayouts = vulkanDescriptorLayouts.data(),
        .pushConstantRangeCount = (uint32)vkPushConstants.size(),
        .pPushConstantRanges = vkPushConstants.data(),
    };

    layoutHash =
        CRC::Calculate(createInfo.pPushConstantRanges, sizeof(VkPushConstantRange) * createInfo.pushConstantRangeCount, CRC::CRC_32());
    layoutHash =
        CRC::Calculate(createInfo.pSetLayouts, sizeof(VkDescriptorSetLayout) * createInfo.setLayoutCount, CRC::CRC_32(), layoutHash);

    std::unique_lock l(layoutLock);
    if (cachedLayouts.contains(layoutHash)) {
        //  std::cout << "New Layout" << std::endl;
        layoutHandle = cachedLayouts[layoutHash];
        return;
    }

    VK_CHECK(vkCreatePipelineLayout(graphics->getDevice(), &createInfo, nullptr, &layoutHandle));
    cachedLayouts[layoutHash] = layoutHandle;
}