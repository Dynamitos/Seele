#pragma once
#include "Enums.h"
#include "Initializer.h"
#include "Resources.h"

namespace Seele {
namespace Gfx {
struct DescriptorBinding {
    std::string name;
    // In Metal uniforms are plain bytes, and for that we need to know the struct size
    uint32 uniformLength = 0;
    SeDescriptorType descriptorType = SE_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
    SeImageViewType textureType = SE_IMAGE_VIEW_TYPE_2D;
    uint32 descriptorCount = 1;
    SeDescriptorBindingFlags bindingFlags = 0;
    SeShaderStageFlags shaderStages = SE_SHADER_STAGE_ALL;
    SeDescriptorAccessTypeFlags access = SE_DESCRIPTOR_ACCESS_READ_BIT;
};

DECLARE_REF(DescriptorPool)
DECLARE_REF(DescriptorSet)
class DescriptorLayout {
  public:
    DescriptorLayout(const std::string& name);
    virtual ~DescriptorLayout();
    void addDescriptorBinding(DescriptorBinding binding);
    void reset();
    ODescriptorSet allocateDescriptorSet();
    virtual void create() = 0;
    constexpr uint32 getHash() const { return hash; }
    constexpr const std::string& getName() const { return name; }

  protected:
    Array<DescriptorBinding> descriptorBindings;
    ODescriptorPool pool;
    std::string name;
    uint32 hash = 0;
    friend class PipelineLayout;
    friend class DescriptorPool;
};
DEFINE_REF(DescriptorLayout)

DECLARE_REF(DescriptorSet)
class DescriptorPool {
  public:
    DescriptorPool();
    virtual ~DescriptorPool();
    virtual ODescriptorSet allocateDescriptorSet() = 0;
    virtual void reset() = 0;
};
DEFINE_REF(DescriptorPool)
DECLARE_REF(UniformBuffer)
DECLARE_REF(ShaderBuffer)
DECLARE_REF(Sampler)
DECLARE_REF(TopLevelAS)
DECLARE_REF(TextureView)
class DescriptorSet {
  public:
    DescriptorSet(PDescriptorLayout layout);
    virtual ~DescriptorSet();
    virtual void writeChanges() = 0;
    virtual void updateConstants(const std::string& name, uint32 offset, void* data) = 0;
    virtual void updateBuffer(const std::string& name, uint32 index, Gfx::PShaderBuffer shaderBuffer) = 0;
    virtual void updateBuffer(const std::string& name, uint32 index, Gfx::PVertexBuffer vertexBuffer) = 0;
    virtual void updateBuffer(const std::string& name, uint32 index, Gfx::PIndexBuffer indexBuffer) = 0;\
    virtual void updateBuffer(const std::string& name, uint32 index, Gfx::PUniformBuffer uniformBuffer) = 0;
    virtual void updateSampler(const std::string& name, uint32 index, Gfx::PSampler samplerState) = 0;
    virtual void updateTexture(const std::string& name, uint32 index, Gfx::PTextureView texture) = 0;
    virtual void updateAccelerationStructure(const std::string& name, uint32 index, Gfx::PTopLevelAS as) = 0;
    bool operator<(PDescriptorSet other);

    constexpr PDescriptorLayout getLayout() const { return layout; }
    constexpr const std::string& getName() const { return layout->getName(); }

  protected:
    PDescriptorLayout layout;
};
DEFINE_REF(DescriptorSet)

DECLARE_REF(PipelineLayout)
class PipelineLayout {
  public:
    PipelineLayout(const std::string& name);
    PipelineLayout(const std::string& name, PPipelineLayout baseLayout);
    virtual ~PipelineLayout();
    virtual void create() = 0;
    void addDescriptorLayout(PDescriptorLayout layout);
    void addPushConstants(const SePushConstantRange& pushConstants);
    constexpr uint32 getHash() const { return layoutHash; }
    constexpr const Map<std::string, PDescriptorLayout>& getLayouts() const { return descriptorSetLayouts; }
    constexpr uint32 findParameter(const std::string& param) const { return parameterMapping[param]; }
    void addMapping(std::string name, uint32 index);
    constexpr std::string getName() const { return name; };
    constexpr bool hasPushConstants() const { return !pushConstants.empty(); }
    constexpr uint64 getPushConstantsSize() const { return pushConstants[0].size; }

  protected:
    uint32 layoutHash = 0;
    Map<std::string, PDescriptorLayout> descriptorSetLayouts;
    Map<std::string, uint32> parameterMapping;
    Array<SePushConstantRange> pushConstants;
    std::string name;
};
DEFINE_REF(PipelineLayout)
} // namespace Gfx
} // namespace Seele
