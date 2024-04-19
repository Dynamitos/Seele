#pragma once
#include "Enums.h"
#include "Resources.h"
#include <compare>

namespace Seele {
namespace Gfx {
struct DescriptorBinding {
  uint32 binding = 0;
  SeDescriptorType descriptorType = SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  SeImageViewType textureType;
  uint32 descriptorCount = 0x7fff;
  SeDescriptorBindingFlags bindingFlags = 0;
  SeShaderStageFlags shaderStages = SE_SHADER_STAGE_ALL;
  Gfx::SeDescriptorAccessTypeFlags access = SE_DESCRIPTOR_ACCESS_READ_ONLY_BIT;
};

DECLARE_REF(DescriptorPool)
DECLARE_REF(DescriptorSet)
class DescriptorLayout {
public:
  DescriptorLayout(const std::string& name);
  DescriptorLayout(const DescriptorLayout& other);
  DescriptorLayout& operator=(const DescriptorLayout& other);
  virtual ~DescriptorLayout();
  void addDescriptorBinding(DescriptorBinding binding);
  void reset();
  PDescriptorSet allocateDescriptorSet();
  virtual void create() = 0;
  constexpr const Array<DescriptorBinding>& getBindings() const { return descriptorBindings; }
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
  virtual PDescriptorSet allocateDescriptorSet() = 0;
  virtual void reset() = 0;
};
DEFINE_REF(DescriptorPool)
DECLARE_REF(UniformBuffer)
DECLARE_REF(ShaderBuffer)
DECLARE_REF(Texture)
DECLARE_REF(Sampler)
class DescriptorSet {
public:
  DescriptorSet(PDescriptorLayout layout);
  virtual ~DescriptorSet();
  virtual void writeChanges() = 0;
  virtual void updateBuffer(uint32 binding, PUniformBuffer uniformBuffer) = 0;
  virtual void updateBuffer(uint32 binding, PShaderBuffer ShaderBuffer) = 0;
  virtual void updateSampler(uint32 binding, PSampler sampler) = 0;
  virtual void updateTexture(uint32 binding, PTexture texture, PSampler samplerState = nullptr) = 0;
  virtual void updateTextureArray(uint32_t binding, Array<PTexture> texture) = 0;
  bool operator<(PDescriptorSet other);

  constexpr PDescriptorLayout getLayout() const { return layout; }

protected:
  PDescriptorLayout layout;
};
DEFINE_REF(DescriptorSet)

class PipelineLayout {
public:
  PipelineLayout();
  PipelineLayout(PPipelineLayout baseLayout);
  virtual ~PipelineLayout();
  virtual void create() = 0;
  void addDescriptorLayout(PDescriptorLayout layout);
  void addPushConstants(const SePushConstantRange& pushConstants);
  constexpr uint32 getHash() const { return layoutHash; }
  constexpr const Map<std::string, PDescriptorLayout>& getLayouts() const { return descriptorSetLayouts; }
  constexpr uint32 findParameter(const std::string& name) const { return parameterMapping[name]; }
    void addMapping(Map<std::string, uint32> mapping);
    
protected:
  uint32 layoutHash = 0;
  Map<std::string, PDescriptorLayout> descriptorSetLayouts;
    Map<std::string, uint32> parameterMapping;
  Array<SePushConstantRange> pushConstants;
};
DEFINE_REF(PipelineLayout)
} // namespace Gfx
} // namespace Seele
