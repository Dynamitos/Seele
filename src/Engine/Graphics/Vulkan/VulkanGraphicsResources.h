#pragma once
#include "Graphics/GraphicsResources.h"

namespace Seele
{
	DECLARE_REF(VulkanDescriptorAllocator);
	DECLARE_REF(VulkanGraphics);
	class VulkanDescriptorLayout : public DescriptorLayout
	{
	public:
		VulkanDescriptorLayout(PVulkanGraphics graphics)
			: graphics(graphics)
		{}
		virtual ~VulkanDescriptorLayout();
		virtual void create();
		inline VkDescriptorSetLayout getHandle() const
		{
			return layoutHandle;
		}
	private:
		PVulkanGraphics graphics;
		Array<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout layoutHandle;
		friend class VulkanPipelineStateCacheManager;
	};
	DEFINE_REF(VulkanDescriptorLayout);
	class VulkanPipelineLayout : public PipelineLayout
	{
	public:
		VulkanPipelineLayout(PVulkanGraphics graphics)
			: graphics(graphics)
		{}
		virtual ~VulkanPipelineLayout()
		{
			if (layoutHandle != VK_NULL_HANDLE)
			{
				vkDestroyPipelineLayout(graphics->getDevice(), layoutHandle, nullptr);
			}
		}
		virtual void create();
		inline VkPipelineLayout getHandle() const
		{
			return layoutHandle;
		}
		virtual uint32 getHash() const;
	private:
		Array<VkDescriptorSetLayout> vulkanDescriptorLayouts;
		uint32 layoutHash;
		VkPipelineLayout layoutHandle;
		PVulkanGraphics graphics;
		friend class VulkanPipelineStateCacheManager;
	};
	DEFINE_REF(VulkanPipelineLayout);

	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		VulkanDescriptorSet(PVulkanGraphics device, PVulkanDescriptorAllocator owner) : device(device), owner(owner) {}
		virtual ~VulkanDescriptorSet();
		virtual void updateBuffer(uint32_t binding, PUniformBuffer uniformBuffer);
		virtual void updateBuffer(uint32_t binding, PStructuredBuffer uniformBuffer);
		virtual void updateSampler(uint32_t binding, PSamplerState samplerState);
		virtual void updateTexture(uint32_t binding, PTexture texture, PSamplerState sampler = nullptr);
		virtual bool operator<(PDescriptorSet other);
		inline VkDescriptorSet getHandle() const
		{
			return setHandle;
		}
	private:
		virtual void writeChanges();
		Array<VkDescriptorImageInfo> imageInfos;
		Array<VkDescriptorBufferInfo> bufferInfos;
		Array<VkWriteDescriptorSet> writeDescriptors;
		VkDescriptorSet setHandle;
		PVulkanDescriptorAllocator owner;
		PVulkanGraphics device;
		friend class VulkanDescriptorAllocator;
	};
	DEFINE_REF(VulkanDescriptorSet);

	class VulkanDescriptorAllocator : public DescriptorAllocator
	{
	public:
		VulkanDescriptorAllocator(PVulkanGraphics device, VulkanDescriptorLayout& layout);
		~VulkanDescriptorAllocator();
		virtual void allocateDescriptorSet(PDescriptorSet& descriptorSet);

		inline VkDescriptorPool getHandle()
		{
			return poolHandle;
		}
	private:
		PVulkanGraphics device;
		int maxSets = 512;
		VkDescriptorPool poolHandle;
		VulkanDescriptorLayout& layout;
	};
	DEFINE_REF(VulkanDescriptorAllocator);
}