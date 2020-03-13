#pragma once
#include "Graphics/GraphicsResources.h"
#include <vulkan/vulkan.h>

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
		virtual ~VulkanPipelineLayout();
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
		PVulkanGraphics graphics;
		friend class VulkanPipelineStateCacheManager;
	};
	DEFINE_REF(VulkanPipelineLayout);

	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		VulkanDescriptorSet(PVulkanGraphics graphics, PVulkanDescriptorAllocator owner) : graphics(graphics), owner(owner) {}
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
		PVulkanGraphics graphics;
		friend class VulkanDescriptorAllocator;
	};
	DEFINE_REF(VulkanDescriptorSet);

	class VulkanDescriptorAllocator : public DescriptorAllocator
	{
	public:
		VulkanDescriptorAllocator(PVulkanGraphics graphics, VulkanDescriptorLayout& layout);
		~VulkanDescriptorAllocator();
		virtual void allocateDescriptorSet(PDescriptorSet& descriptorSet);

		inline VkDescriptorPool getHandle()
		{
			return poolHandle;
		}
	private:
		PVulkanGraphics graphics;
		int maxSets = 512;
		VkDescriptorPool poolHandle;
		VulkanDescriptorLayout& layout;
	};
	DEFINE_REF(VulkanDescriptorAllocator);

	class VulkanUniformBuffer : public UniformBuffer
	{

	};
	DEFINE_REF(VulkanUniformBuffer);

	class VulkanStructuredBuffer : public StructuredBuffer
	{

	};
	DEFINE_REF(VulkanStructuredBuffer);

	class VulkanTextureBase
	{
	private:
		uint32 sizeX;
		uint32 sizeY;
		uint32 sizeZ;
		uint32 arrayCount;
		uint32 layerCount;
	};
	DEFINE_REF(VulkanTextureBase);

	class VulkanTexture2D : public Texture2D
	{
	public:
	private:
		PVulkanTextureBase textureHandle;
	};
	DEFINE_REF(VulkanTexture2D);

	class VulkanSamplerState
	{
	public:
		VkSampler sampler;
	};
	DEFINE_REF(VulkanSamplerState);
}