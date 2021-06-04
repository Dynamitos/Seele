#pragma once
#include "VulkanGraphicsResources.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(Graphics)
class DescriptorLayout : public Gfx::DescriptorLayout
{
public:
	DescriptorLayout(PGraphics graphics, const std::string& name);
	virtual ~DescriptorLayout();
	virtual void create();
	inline VkDescriptorSetLayout getHandle() const
	{
		return layoutHandle;
	}

private:
	uint32 hash;
	PGraphics graphics;
	Array<VkDescriptorSetLayoutBinding> bindings;
	VkDescriptorSetLayout layoutHandle;
	std::string name;
	friend class DescriptorAllocator;
};
DEFINE_REF(DescriptorLayout)
class PipelineLayout : public Gfx::PipelineLayout
{
public:
	PipelineLayout(PGraphics graphics)
		: graphics(graphics)
		, layoutHash(0)
		, layoutHandle(VK_NULL_HANDLE)
	{
	}
	virtual ~PipelineLayout();
	virtual void create();
	virtual void reset();
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
	PGraphics graphics;
	uint32 layoutHash;
	VkPipelineLayout layoutHandle;
};
DEFINE_REF(PipelineLayout)


class DescriptorSet : public Gfx::DescriptorSet
{
public:
	DescriptorSet(PGraphics graphics, PDescriptorAllocator owner)
		: setHandle(VK_NULL_HANDLE)
		, graphics(graphics)
		, owner(owner)
		, currentlyInUse(false)
		, currentlyBound(false)
	{
	}
	virtual ~DescriptorSet();
	virtual void writeChanges();
	virtual void updateBuffer(uint32_t binding, Gfx::PUniformBuffer uniformBuffer);
	virtual void updateBuffer(uint32_t binding, Gfx::PStructuredBuffer uniformBuffer);
	virtual void updateSampler(uint32_t binding, Gfx::PSamplerState samplerState);
	virtual void updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSamplerState sampler = nullptr);
	virtual bool operator<(Gfx::PDescriptorSet other);
	
	inline bool isCurrentlyBound() const
	{
		return currentlyBound;
	}
	inline bool isCurrentlyInUse() const
	{
		return currentlyInUse;
	}
	void bind()
	{
		currentlyBound = true;
	}
	void unbind()
	{
		currentlyBound = false;
	}
	void free()
	{
		currentlyInUse = false;
	}
	inline VkDescriptorSet getHandle() const
	{
		return setHandle;
	}
	virtual uint32 getSetIndex() const;

private:
	List<VkDescriptorImageInfo> imageInfos;
	List<VkDescriptorBufferInfo> bufferInfos;
	Array<VkWriteDescriptorSet> writeDescriptors;
	// contains the previously bound resources at every binding
	// since the layout is fixed, trying to bind a texture to a buffer
	// would not work anyways, so casts should be safe
	Array<void*> cachedData;
	VkDescriptorSet setHandle;
	PGraphics graphics;
	PDescriptorAllocator owner;
	//PCmdBuffer currentlyBound;
	bool currentlyBound;
	bool currentlyInUse;
	friend class DescriptorAllocator;
	friend class CmdBuffer;
	friend class SecondaryCmdBuffer;
};
DEFINE_REF(DescriptorSet)

class DescriptorAllocator : public Gfx::DescriptorAllocator
{
public:
	DescriptorAllocator(PGraphics graphics, DescriptorLayout &layout);
	virtual ~DescriptorAllocator();
	virtual void allocateDescriptorSet(Gfx::PDescriptorSet &descriptorSet);
	virtual void reset();

	inline VkDescriptorPool getHandle() const
	{
		return poolHandle;
	}
	inline DescriptorLayout& getLayout() const
	{
		return layout;
	}

private:
	PGraphics graphics;
	DescriptorLayout &layout;
	const static int maxSets = 512;
	StaticArray<PDescriptorSet, maxSets> cachedHandles;
	VkDescriptorPool poolHandle;
};
DEFINE_REF(DescriptorAllocator)
} // namespace Vulkan
} // namespace Seele