#pragma once
#include "Enums.h"
#include "Resources.h"

namespace Seele
{
namespace Gfx
{
struct DescriptorBinding
{
    uint32 binding = 0;
    SeDescriptorType descriptorType = SE_DESCRIPTOR_TYPE_MAX_ENUM;
    uint32 descriptorCount = 0x7fff;
    SeDescriptorBindingFlags bindingFlags = 0;
    SeShaderStageFlags shaderStages = SE_SHADER_STAGE_ALL;
};
DEFINE_REF(DescriptorBinding)

DECLARE_REF(DescriptorSet)
class DescriptorPool
{
public:
    DescriptorPool();
    virtual ~DescriptorPool();
    virtual PDescriptorSet allocateDescriptorSet() = 0;
    virtual void reset() = 0;
};
DEFINE_REF(DescriptorPool)
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
    ODescriptorPool pool;
    uint32 setIndex;
    std::string name;
    friend class PipelineLayout;
    friend class DescriptorPool;
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
    constexpr uint32 getHash() const { return layoutHash; }

protected:
    uint32 layoutHash;
    Array<PDescriptorLayout> descriptorSetLayouts;
    Array<SePushConstantRange> pushConstants;
};
DEFINE_REF(PipelineLayout)
}
}
