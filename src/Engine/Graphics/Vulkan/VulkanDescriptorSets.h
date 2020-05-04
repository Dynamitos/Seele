#include "VulkanGraphicsResources.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(Graphics);
class DescriptorLayout : public Gfx::DescriptorLayout
{
public:
	DescriptorLayout(PGraphics graphics)
		: graphics(graphics), layoutHandle(VK_NULL_HANDLE)
	{
	}
	virtual ~DescriptorLayout();
	virtual void create();
	inline VkDescriptorSetLayout getHandle() const
	{
		return layoutHandle;
	}

private:
	PGraphics graphics;
	Array<VkDescriptorSetLayoutBinding> bindings;
	VkDescriptorSetLayout layoutHandle;
};
DEFINE_REF(DescriptorLayout);
class PipelineLayout : public Gfx::PipelineLayout
{
public:
	PipelineLayout(PGraphics graphics)
		: graphics(graphics), layoutHash(0), layoutHandle(VK_NULL_HANDLE)
	{
	}
	virtual ~PipelineLayout();
	virtual void create();
	inline VkPipelineLayout getHandle() const
	{
		return layoutHandle;
	}
	virtual uint32 getHash() const
	{
		return layoutHash;
	}

private:
	Array<VkDescriptorSetLayout> vulkanDescriptorLayouts;
	uint32 layoutHash;
	VkPipelineLayout layoutHandle;
	PGraphics graphics;
};
DEFINE_REF(PipelineLayout);

class DescriptorAllocator : public Gfx::DescriptorAllocator
{
public:
	DescriptorAllocator(PGraphics graphics, DescriptorLayout &layout);
	virtual ~DescriptorAllocator();
	virtual void allocateDescriptorSet(Gfx::PDescriptorSet &descriptorSet);

	inline VkDescriptorPool getHandle() const
	{
		return poolHandle;
	}
	inline DescriptorLayout getLayout() const
	{
		return layout;
	}

private:
	PGraphics graphics;
	int maxSets = 512;
	VkDescriptorPool poolHandle;
	DescriptorLayout &layout;
};
DEFINE_REF(DescriptorAllocator);

class DescriptorSet : public Gfx::DescriptorSet
{
public:
	DescriptorSet(PGraphics graphics, PDescriptorAllocator owner)
		: graphics(graphics), owner(owner), setHandle(VK_NULL_HANDLE)
	{
	}
	virtual ~DescriptorSet();
	virtual void updateBuffer(uint32_t binding, Gfx::PUniformBuffer uniformBuffer);
	virtual void updateBuffer(uint32_t binding, Gfx::PStructuredBuffer uniformBuffer);
	virtual void updateSampler(uint32_t binding, Gfx::PSamplerState samplerState);
	virtual void updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSamplerState sampler = nullptr);
	virtual bool operator<(Gfx::PDescriptorSet other);
	inline VkDescriptorSet getHandle() const
	{
		return setHandle;
	}
	virtual uint32 getSetIndex() const
	{
		return owner->getLayout().getSetIndex();
	}

private:
	virtual void writeChanges();
	Array<VkDescriptorImageInfo> imageInfos;
	Array<VkDescriptorBufferInfo> bufferInfos;
	Array<VkWriteDescriptorSet> writeDescriptors;
	VkDescriptorSet setHandle;
	PDescriptorAllocator owner;
	PGraphics graphics;
	friend class DescriptorAllocator;
};
DEFINE_REF(DescriptorSet);

} // namespace Vulkan
} // namespace Seele