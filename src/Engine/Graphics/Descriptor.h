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
    DescriptorBinding();
    DescriptorBinding(const DescriptorBinding& other);
    DescriptorBinding& operator=(const DescriptorBinding& other);
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
    DescriptorAllocator();
    virtual ~DescriptorAllocator();
    virtual PDescriptorSet allocateDescriptorSet() = 0;
    virtual void reset() = 0;
};
DEFINE_REF(DescriptorAllocator)
DECLARE_REF(UniformBuffer)
DECLARE_REF(ShaderBuffer)
DECLARE_REF(Texture)
DECLARE_REF(Sampler)
class DescriptorSet
{
public:
    DescriptorSet() {}
    virtual ~DescriptorSet() {}
    virtual void writeChanges() = 0;
    virtual void updateBuffer(uint32 binding, PUniformBuffer uniformBuffer) = 0;
    virtual void updateBuffer(uint32 binding, PShaderBuffer ShaderBuffer) = 0;
    virtual void updateSampler(uint32 binding, PSampler sampler) = 0;
    virtual void updateTexture(uint32 binding, PTexture texture, PSampler samplerState = nullptr) = 0;
    virtual void updateTextureArray(uint32_t binding, Array<PTexture> texture) = 0;
    virtual bool operator<(PDescriptorSet other) = 0;

    virtual uint32 getSetIndex() const = 0;
};
DEFINE_REF(DescriptorSet)

class DescriptorLayout
{
public:
    DescriptorLayout(const std::string& name);
    DescriptorLayout(const DescriptorLayout& other);
    DescriptorLayout& operator=(const DescriptorLayout& other);
    virtual ~DescriptorLayout();
    virtual void create() = 0;
    virtual void addDescriptorBinding(uint32 binding, SeDescriptorType type, uint32 arrayCount = 1, SeDescriptorBindingFlags bindingFlags = 0, SeShaderStageFlags shaderStages = SeShaderStageFlagBits::SE_SHADER_STAGE_ALL);
    virtual void reset();
    virtual PDescriptorSet allocateDescriptorSet();
    constexpr const Array<DescriptorBinding>& getBindings() const { return descriptorBindings; }
    constexpr uint32 getSetIndex() const { return setIndex; }
    constexpr void setSetIndex(uint32 _setIndex) { setIndex = _setIndex; }

protected:
    Array<DescriptorBinding> descriptorBindings;
    ODescriptorAllocator allocator;
    uint32 setIndex;
    std::string name;
    friend class PipelineLayout;
    friend class DescriptorAllocator;
};
DEFINE_REF(DescriptorLayout)
class PipelineLayout
{
public:
    PipelineLayout();
    PipelineLayout(PPipelineLayout baseLayout);
    virtual ~PipelineLayout();
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
