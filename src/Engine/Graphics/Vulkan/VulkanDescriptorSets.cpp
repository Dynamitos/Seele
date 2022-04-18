#include "VulkanDescriptorSets.h"
#include "VulkanGraphicsResources.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanGraphics.h"
#include "VulkanInitializer.h"
#include "VulkanCommandBuffer.h"

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
        VkDescriptorSetLayoutBinding &binding = bindings[i];
        const Gfx::DescriptorBinding &rhiBinding = descriptorBindings[i];
        binding.binding = rhiBinding.binding;
        binding.descriptorCount = rhiBinding.descriptorCount;
        binding.descriptorType = cast(rhiBinding.descriptorType);
        binding.stageFlags = rhiBinding.shaderStages;
        binding.pImmutableSamplers = nullptr;
        bindingFlags[i] = rhiBinding.bindingFlags;
    }
    VkDescriptorSetLayoutCreateInfo createInfo =
        init::DescriptorSetLayoutCreateInfo(bindings.data(), (uint32)bindings.size());
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo = {};
    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsInfo.pNext = nullptr;
    bindingFlagsInfo.bindingCount = static_cast<uint32>(bindingFlags.size());
    bindingFlagsInfo.pBindingFlags = bindingFlags.data();
    createInfo.pNext = &bindingFlagsInfo;
    VK_CHECK(vkCreateDescriptorSetLayout(graphics->getDevice(), &createInfo, nullptr, &layoutHandle));

    allocator = new DescriptorAllocator(graphics, *this);

    boost::crc_32_type result;
    result.process_bytes(bindings.data(), sizeof(VkDescriptorSetLayoutBinding) * bindings.size());
    hash = result.checksum();
}

PipelineLayout::~PipelineLayout()
{
    if (layoutHandle != VK_NULL_HANDLE)
    {
        //vkDestroyPipelineLayout(graphics->getDevice(), layoutHandle, nullptr);
    }
}

static Map<uint32, VkPipelineLayout> layoutCache;

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

    VkPipelineLayoutCreateInfo createInfo =
        init::PipelineLayoutCreateInfo(vulkanDescriptorLayouts.data(), (uint32)vulkanDescriptorLayouts.size());
    Array<VkPushConstantRange> vkPushConstants(pushConstants.size());
    for (size_t i = 0; i < pushConstants.size(); i++)
    {
        vkPushConstants[i].offset = pushConstants[i].offset;
        vkPushConstants[i].size = pushConstants[i].size;
        vkPushConstants[i].stageFlags = (VkShaderStageFlagBits)pushConstants[i].stageFlags;
    }
    createInfo.pushConstantRangeCount = (uint32)vkPushConstants.size();
    createInfo.pPushConstantRanges = vkPushConstants.data();

    boost::crc_32_type result;
    result.process_bytes(createInfo.pPushConstantRanges, sizeof(VkPushConstantRange) * createInfo.pushConstantRangeCount);
    result.process_bytes(createInfo.pSetLayouts, sizeof(VkDescriptorSetLayout) * createInfo.setLayoutCount);
    layoutHash = result.checksum();

    if(layoutCache[layoutHash] != VK_NULL_HANDLE)
    {
        layoutHandle = layoutCache[layoutHash];
        return;
    }

    VK_CHECK(vkCreatePipelineLayout(graphics->getDevice(), &createInfo, nullptr, &layoutHandle));
    layoutCache[layoutHash] = layoutHandle;
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
    bufferInfos.add(init::DescriptorBufferInfo(vulkanBuffer->getHandle(), 0, vulkanBuffer->getSize()));

    VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, &bufferInfos.back());
    writeDescriptors.add(writeDescriptor);

    cachedData[binding] = new UniformBuffer(*vulkanBuffer.getHandle());
}

void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PStructuredBuffer structuredBuffer)
{
    PStructuredBuffer vulkanBuffer = structuredBuffer.cast<StructuredBuffer>();
    StructuredBuffer* cachedBuffer = reinterpret_cast<StructuredBuffer*>(cachedData[binding]);
    if(vulkanBuffer.getHandle() == cachedBuffer)
    {
        return;
    }
    bufferInfos.add(init::DescriptorBufferInfo(vulkanBuffer->getHandle(), 0, vulkanBuffer->getSize()));

    VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding, &bufferInfos.back());
    writeDescriptors.add(writeDescriptor);

    cachedData[binding] = vulkanBuffer.getHandle();
}

void DescriptorSet::updateSampler(uint32_t binding, Gfx::PSamplerState samplerState)
{
    PSamplerState vulkanSampler = samplerState.cast<SamplerState>();
    SamplerState* cachedSampler = reinterpret_cast<SamplerState*>(cachedData[binding]);
    if(vulkanSampler.getHandle() == cachedSampler)
    {
        return;
    }
    VkDescriptorImageInfo imageInfo =
        init::DescriptorImageInfo(
            vulkanSampler->sampler,
            VK_NULL_HANDLE,
            VK_IMAGE_LAYOUT_UNDEFINED);
    imageInfos.add(imageInfo);

    VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle, VK_DESCRIPTOR_TYPE_SAMPLER, binding, &imageInfos.back());
    writeDescriptors.add(writeDescriptor);

    cachedData[binding] = vulkanSampler.getHandle();
}

void DescriptorSet::updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSamplerState samplerState)
{
    TextureHandle* vulkanTexture = TextureBase::cast(texture);
    TextureHandle* cachedTexture = reinterpret_cast<TextureHandle*>(cachedData[binding]);
    if(vulkanTexture == cachedTexture)
    {
        return;
    }
    //It is assumed that the image is in the correct layout
    VkDescriptorImageInfo imageInfo =
        init::DescriptorImageInfo(
            VK_NULL_HANDLE,
            vulkanTexture->getView(),
            cast(vulkanTexture->getLayout()));
    if (samplerState != nullptr)
    {
        PSamplerState vulkanSampler = samplerState.cast<SamplerState>();
        imageInfo.sampler = vulkanSampler->sampler;
    }
    imageInfos.add(imageInfo);
    VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    if(samplerState != nullptr)
    {
        descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }
    else if (vulkanTexture->getUsage() & VK_IMAGE_USAGE_STORAGE_BIT)
    {
        descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    VkWriteDescriptorSet writeDescriptor = 
        init::WriteDescriptorSet(
            setHandle, 
            descriptorType, 
            binding, 
            &imageInfos.back());
    
    writeDescriptors.add(writeDescriptor);

    cachedData[binding] = vulkanTexture;
}
void DescriptorSet::updateTextureArray(uint32_t binding, Array<Gfx::PTexture> textures)
{
    // maybe make this a parameter?
    uint32 arrayElement = 0;
    for(const auto& gfxTexture : textures)
    {
        TextureHandle* vulkanTexture = TextureBase::cast(gfxTexture);
        imageInfos.add(
            init::DescriptorImageInfo(
                VK_NULL_HANDLE,
                vulkanTexture->getView(),
                cast(vulkanTexture->getLayout())));

        VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        if (vulkanTexture->getUsage() & VK_IMAGE_USAGE_STORAGE_BIT)
        {
            descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
        VkWriteDescriptorSet& write = writeDescriptors.add(
            init::WriteDescriptorSet(
                setHandle,
                descriptorType,
                binding,
                &imageInfos.back()
            ));
        write.dstArrayElement = arrayElement++;
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

DescriptorAllocator::DescriptorAllocator(PGraphics graphics, DescriptorLayout &layout)
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
    VkDescriptorPoolCreateInfo createInfo 
        = init::DescriptorPoolCreateInfo(
            (uint32)poolSizes.size(), 
            poolSizes.data(), 
            maxSets * Gfx::numFramesBuffered);
    VK_CHECK(vkCreateDescriptorPool(graphics->getDevice(), &createInfo, nullptr, &poolHandle));
}

DescriptorAllocator::~DescriptorAllocator()
{
    vkDestroyDescriptorPool(graphics->getDevice(), poolHandle, nullptr);
    graphics = nullptr;
    if(nextAlloc)
    {
        delete nextAlloc;
    }
}

void DescriptorAllocator::allocateDescriptorSet(Gfx::PDescriptorSet &descriptorSet)
{
    VkDescriptorSetLayout layoutHandle = layout.getHandle();
    VkDescriptorSetAllocateInfo allocInfo =
        init::DescriptorSetAllocateInfo(poolHandle, &layoutHandle, 1);
    VkDescriptorSetVariableDescriptorCountAllocateInfo setCounts = {};
    setCounts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    setCounts.pNext = nullptr;
    setCounts.descriptorSetCount = 1;
    uint32 counts = 0;
    for(const auto& binding : layout.bindings)
    {
        if(binding.descriptorCount > 0)
        {
            counts = binding.descriptorCount;
        }
    }
    setCounts.pDescriptorCounts = &counts;
    allocInfo.pNext = &setCounts;

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
        descriptorSet = cachedHandles[setIndex];
        
        PDescriptorSet vulkanSet = descriptorSet.cast<DescriptorSet>();
        vulkanSet->cachedData.resize(layout.bindings.size());
        // Not really pretty, but this way the set knows which ones are valid
        std::memset(vulkanSet->cachedData.data(), 0, sizeof(void*) * vulkanSet->cachedData.size());
        
        //Found set, stop searching
        return;
    }
    if(nextAlloc == nullptr)
    {
        nextAlloc = new DescriptorAllocator(graphics, layout);
    }
    nextAlloc->allocateDescriptorSet(descriptorSet);
    //throw std::logic_error("Out of descriptor sets");
}

void DescriptorAllocator::reset()
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