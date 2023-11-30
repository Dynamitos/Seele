#include "Descriptor.h"
#include "Graphics.h"
#include "Texture.h"
#include "Buffer.h"

using namespace Seele;
using namespace Seele::Vulkan;

DescriptorLayout::DescriptorLayout(PGraphics graphics, const std::string& name)
    : Gfx::DescriptorLayout(name)
    , graphics(graphics)
    , layoutHandle(VK_NULL_HANDLE)
{
}
DescriptorLayout::~DescriptorLayout()
{
    if (layoutHandle != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(graphics->getDevice(), layoutHandle, nullptr);
    }
}

void DescriptorLayout::create()
{
    if (layoutHandle != VK_NULL_HANDLE)
    {
        return;
    }
    bindings.resize(descriptorBindings.size());
    Array<VkDescriptorBindingFlags> bindingFlags(descriptorBindings.size());
    for (size_t i = 0; i < descriptorBindings.size(); ++i)
    {
        const Gfx::DescriptorBinding &gfxBinding = descriptorBindings[i];
        bindings[i] = {
            .binding = gfxBinding.binding,
            .descriptorType = cast(gfxBinding.descriptorType),
            .descriptorCount = gfxBinding.descriptorCount,
            .stageFlags = gfxBinding.shaderStages,
            .pImmutableSamplers = nullptr,
        };
        bindingFlags[i] = gfxBinding.bindingFlags;
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

    pool = new DescriptorPool(graphics, *this);

    hash = CRC::Calculate(bindings.data(), sizeof(VkDescriptorSetLayoutBinding) * bindings.size(), CRC::CRC_32());
}

PipelineLayout::~PipelineLayout()
{
}

Map<uint32, VkPipelineLayout> cachedLayouts;

void PipelineLayout::create()
{
    vulkanDescriptorLayouts.resize(descriptorSetLayouts.size());
    for (size_t i = 0; i < descriptorSetLayouts.size(); ++i)
    {
        // There could be unused descriptor set indices
        if(descriptorSetLayouts[i] == nullptr)
        {
            vulkanDescriptorLayouts[i] = VK_NULL_HANDLE;
            continue;
        }
        PDescriptorLayout layout = descriptorSetLayouts[i].cast<DescriptorLayout>();
        layout->create();
        vulkanDescriptorLayouts[i] = layout->getHandle();
    }

    Array<VkPushConstantRange> vkPushConstants(pushConstants.size());
    for (size_t i = 0; i < pushConstants.size(); i++)
    {
        vkPushConstants[i].offset = pushConstants[i].offset;
        vkPushConstants[i].size = pushConstants[i].size;
        vkPushConstants[i].stageFlags = (VkShaderStageFlagBits)pushConstants[i].stageFlags;
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

    layoutHash = CRC::Calculate(createInfo.pPushConstantRanges, sizeof(VkPushConstantRange) * createInfo.pushConstantRangeCount, CRC::CRC_32());
    layoutHash = CRC::Calculate(createInfo.pSetLayouts, sizeof(VkDescriptorSetLayout) * createInfo.setLayoutCount, CRC::CRC_32(), layoutHash);

    if (cachedLayouts.contains(layoutHash))
    {
        layoutHandle = cachedLayouts[layoutHash];
        return;
    }

    VK_CHECK(vkCreatePipelineLayout(graphics->getDevice(), &createInfo, nullptr, &layoutHandle));
    cachedLayouts[layoutHash] = layoutHandle;
}

void PipelineLayout::reset()
{
    vkDestroyPipelineLayout(graphics->getDevice(), layoutHandle, nullptr);
    descriptorSetLayouts.clear();
    pushConstants.clear();
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PUniformBuffer uniformBuffer)
{
    PUniformBuffer vulkanBuffer = uniformBuffer.cast<UniformBuffer>();
    UniformBuffer* cachedBuffer = reinterpret_cast<UniformBuffer*>(cachedData[binding]);
    if(vulkanBuffer->isDataEquals(cachedBuffer))
    {
        //std::cout << "uniform data equal, skip" << std::endl; 
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
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bufferInfos.back(),
    });

    cachedData[binding] = vulkanBuffer.getHandle();
}

void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PShaderBuffer shaderBuffer)
{
    PShaderBuffer vulkanBuffer = shaderBuffer.cast<ShaderBuffer>();
    ShaderBuffer* cachedBuffer = reinterpret_cast<ShaderBuffer*>(cachedData[binding]);
    if(vulkanBuffer.getHandle() == cachedBuffer)
    {
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
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &bufferInfos.back(),
    });

    cachedData[binding] = vulkanBuffer.getHandle();
}

void DescriptorSet::updateSampler(uint32_t binding, Gfx::PSampler samplerState)
{
    PSampler vulkanSampler = samplerState.cast<Sampler>();
    Sampler* cachedSampler = reinterpret_cast<Sampler*>(cachedData[binding]);
    if(vulkanSampler.getHandle() == cachedSampler)
    {
        return;
    }
    imageInfos.add(VkDescriptorImageInfo{
        .sampler = vulkanSampler->sampler,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    });

    writeDescriptors.add(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = setHandle,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &imageInfos.back(),
    });

    cachedData[binding] = vulkanSampler.getHandle();
}

void DescriptorSet::updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSampler samplerState)
{
    TextureBase* vulkanTexture = texture.cast<TextureBase>().getHandle();
    TextureBase* cachedTexture = reinterpret_cast<TextureBase*>(cachedData[binding]);
    if(vulkanTexture == cachedTexture)
    {
        return;
    }
    //It is assumed that the image is in the correct layout
    imageInfos.add(VkDescriptorImageInfo{
        .sampler = samplerState != nullptr ? samplerState.cast<Sampler>()->sampler : VK_NULL_HANDLE,
        .imageView = vulkanTexture->getView(),
        .imageLayout = cast(vulkanTexture->getLayout()),
    });
    VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    if(samplerState != nullptr)
    {
        descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }
    else if (vulkanTexture->getUsage() & VK_IMAGE_USAGE_STORAGE_BIT)
    {
        descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    writeDescriptors.add(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = setHandle,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = descriptorType,
        .pImageInfo = &imageInfos.back(),
        });

    cachedData[binding] = vulkanTexture;
}
void DescriptorSet::updateTextureArray(uint32_t binding, Array<Gfx::PTexture> textures)
{
    // maybe make this a parameter?
    uint32 arrayElement = 0;
    for(auto& gfxTexture : textures)
    {
        TextureBase* vulkanTexture = gfxTexture.cast<TextureBase>().getHandle();
        imageInfos.add(VkDescriptorImageInfo{
            .sampler = VK_NULL_HANDLE,
            .imageView = vulkanTexture->getView(),
            .imageLayout = cast(vulkanTexture->getLayout()),
        });

        VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        if (vulkanTexture->getUsage() & VK_IMAGE_USAGE_STORAGE_BIT)
        {
            descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
        writeDescriptors.add(VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = setHandle,
            .dstBinding = binding,
            .dstArrayElement = arrayElement++,
            .descriptorCount = 1,
            .descriptorType = descriptorType,
            .pImageInfo = &imageInfos.back(),
        });
    }
}

bool DescriptorSet::operator<(Gfx::PDescriptorSet other)
{
    PDescriptorSet otherSet = other.cast<DescriptorSet>();
    return this < otherSet.getHandle();
}

uint32 DescriptorSet::getSetIndex() const
{
    return owner->getLayout().getSetIndex();
}

void DescriptorSet::writeChanges()
{
    if (writeDescriptors.size() > 0)
    {
        if(isCurrentlyBound())
        {
            std::cout << "Descriptor currently bound, allocate a new one instead" << std::endl;
            assert(!isCurrentlyBound());
        }
        vkUpdateDescriptorSets(graphics->getDevice(), (uint32)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
        writeDescriptors.clear();
        imageInfos.clear();
        bufferInfos.clear();
    }
}

DescriptorPool::DescriptorPool(PGraphics graphics, DescriptorLayout &layout)
    : graphics(graphics)
    , layout(layout)
{
    for(uint32 i = 0; i < cachedHandles.size(); ++i)
    {
        cachedHandles[i] = nullptr;
    }

    uint32 perTypeSizes[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT]; // TODO: FIX ENUM
    std::memset(perTypeSizes, 0, sizeof(perTypeSizes));
    for (uint32 i = 0; i < layout.getBindings().size(); ++i)
    {
        auto &binding = layout.getBindings()[i];
        int typeIndex = binding.descriptorType;
        perTypeSizes[typeIndex] += 256;
    }
    Array<VkDescriptorPoolSize> poolSizes;
    for (uint32 i = 0; i < VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; ++i)
    {
        if (perTypeSizes[i] > 0)
        {
            VkDescriptorPoolSize size;
            size.descriptorCount = perTypeSizes[i];
            size.type = (VkDescriptorType)i;
            poolSizes.add(size);
        }
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

DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(graphics->getDevice(), poolHandle, nullptr);
    graphics = nullptr;
    if(nextAlloc)
    {
        delete nextAlloc;
    }
}

Gfx::PDescriptorSet DescriptorPool::allocateDescriptorSet()
{
    VkDescriptorSetLayout layoutHandle = layout.getHandle();
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
    for(const auto& binding : layout.bindings)
    {
        if(binding.descriptorCount > 0)
        {
            counts = binding.descriptorCount;
        }
    }
    if (layout.bindings.size() > 0)
    {
        setCounts.pDescriptorCounts = &counts;
        allocInfo.pNext = &setCounts;
    }
    for(uint32 setIndex = 0; setIndex < cachedHandles.size(); ++setIndex)
    {
        if(cachedHandles[setIndex] == nullptr)
        {
            cachedHandles[setIndex] = new DescriptorSet(graphics, this);
        }
        if(cachedHandles[setIndex]->isCurrentlyBound() || cachedHandles[setIndex]->isCurrentlyInUse())
        {
            // Currently in use, skip
            continue;
        }
        if(cachedHandles[setIndex]->getHandle() == VK_NULL_HANDLE)
        {
            //If it hasnt been initialized, allocate it
            VK_CHECK(vkAllocateDescriptorSets(graphics->getDevice(), &allocInfo, &cachedHandles[setIndex]->setHandle));
        }
        cachedHandles[setIndex]->allocate();
        
        PDescriptorSet vulkanSet = cachedHandles[setIndex];
        vulkanSet->cachedData.resize(layout.bindings.size());
        // Not really pretty, but this way the set knows which ones are valid
        std::memset(vulkanSet->cachedData.data(), 0, sizeof(void*) * vulkanSet->cachedData.size());
        
        //Found set, stop searching
        return vulkanSet;
    }
    if(nextAlloc == nullptr)
    {
        nextAlloc = new DescriptorPool(graphics, layout);
    }
    std::cout << "Out of descriptors, forwarding" << std::endl;
    return nextAlloc->allocateDescriptorSet();
    //throw std::logic_error("Out of descriptor sets");
}

void DescriptorPool::reset()
{
    for(uint32 i = 0; i < cachedHandles.size(); ++i)
    {
        if(cachedHandles[i] == nullptr)
        {
            return;
        }
        cachedHandles[i]->free();
    }
    if(nextAlloc != nullptr)
    {
        nextAlloc->reset();
    }
}