#pragma once
#include "Enums.h"
#include "Resources.h"

namespace Seele
{
namespace Gfx
{

class DescriptorBinding
{
public:
    DescriptorBinding()
        : binding(0), descriptorType(SE_DESCRIPTOR_TYPE_MAX_ENUM), descriptorCount(0x7fff), shaderStages(SE_SHADER_STAGE_ALL)
    {
    }
    DescriptorBinding(const DescriptorBinding& other)
        : binding(other.binding), descriptorType(other.descriptorType), descriptorCount(other.descriptorCount), shaderStages(other.shaderStages)
    {
    }
    uint32 binding;
    SeDescriptorType descriptorType;
    uint32 descriptorCount;
    SeDescriptorBindingFlags bindingFlags = 0;
    SeShaderStageFlags shaderStages;
};
DEFINE_REF(DescriptorBinding)

DECLARE_REF(DescriptorSet)
class DescriptorAllocator
{
public:
    DescriptorAllocator() {}
    virtual ~DescriptorAllocator() {}
    virtual void allocateDescriptorSet(PDescriptorSet& descriptorSet) = 0;
    virtual void reset() = 0;
};
DEFINE_REF(DescriptorAllocator)
DECLARE_REF(UniformBuffer)
DECLARE_REF(ShaderBuffer)
DECLARE_REF(Texture)
DECLARE_REF(SamplerState)
class DescriptorSet
{
public:
    virtual ~DescriptorSet() {}
    virtual void writeChanges() = 0;
    virtual void updateBuffer(uint32 binding, PUniformBuffer uniformBuffer) = 0;
    virtual void updateBuffer(uint32 binding, PShaderBuffer ShaderBuffer) = 0;
    virtual void updateSampler(uint32 binding, PSamplerState samplerState) = 0;
    virtual void updateTexture(uint32 binding, PTexture texture, PSamplerState samplerState = nullptr) = 0;
    virtual void updateTextureArray(uint32_t binding, Array<PTexture> texture) = 0;
    virtual bool operator<(PDescriptorSet other) = 0;

    virtual uint32 getSetIndex() const = 0;
};
DEFINE_REF(DescriptorSet)

class DescriptorLayout
{
public:
    DescriptorLayout(const std::string& name)
        : setIndex(0)
        , name(name)
    {
    }
    virtual ~DescriptorLayout() {}
    DescriptorLayout& operator=(const DescriptorLayout& other)
    {
        if (this != &other)
        {
            descriptorBindings.resize(other.descriptorBindings.size());
            for (uint32 i = 0; i < descriptorBindings.size(); ++i)
            {
                descriptorBindings[i] = other.descriptorBindings[i];
            }
        }
        return *this;
    }
    virtual void create() = 0;
    virtual void addDescriptorBinding(uint32 binding, SeDescriptorType type, uint32 arrayCount = 1, SeDescriptorBindingFlags bindingFlags = 0, SeShaderStageFlags shaderStages = SeShaderStageFlagBits::SE_SHADER_STAGE_ALL);
    virtual void reset();
    virtual PDescriptorSet allocateDescriptorSet();
    const Array<DescriptorBinding>& getBindings() const { return descriptorBindings; }
    constexpr uint32 getSetIndex() const { return setIndex; }
    constexpr void setSetIndex(uint32 _setIndex) { setIndex = _setIndex; }

protected:
    Array<DescriptorBinding> descriptorBindings;
    PDescriptorAllocator allocator;
    std::mutex allocatorLock;
    uint32 setIndex;
    std::string name;
    friend class PipelineLayout;
    friend class DescriptorAllocator;
};
DEFINE_REF(DescriptorLayout)
class PipelineLayout
{
public:
    PipelineLayout(PPipelineLayout baseLayout)
    {
        if (baseLayout != nullptr)
        {
            descriptorSetLayouts = baseLayout->descriptorSetLayouts;
            pushConstants = baseLayout->pushConstants;
        }
    }
    virtual ~PipelineLayout() {}
    virtual void create() = 0;
    virtual void reset() = 0;
    void addDescriptorLayout(uint32 setIndex, PDescriptorLayout layout);
    void addPushConstants(const SePushConstantRange& pushConstants);
    virtual uint32 getHash() const = 0;

protected:
    Array<PDescriptorLayout> descriptorSetLayouts;
    Array<SePushConstantRange> pushConstants;
};
DEFINE_REF(PipelineLayout)
}
}