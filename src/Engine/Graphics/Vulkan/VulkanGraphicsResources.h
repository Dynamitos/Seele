#pragma once
#include "Graphics/GraphicsResources.h"
#include <vulkan/vulkan.h>

namespace Seele
{
namespace Vulkan
{

DECLARE_REF(DescriptorAllocator);
DECLARE_REF(CommandBufferManager);
DECLARE_REF(Graphics);
class Semaphore
{
public:
	Semaphore(WGraphics graphics);
	virtual ~Semaphore();

private:
	VkSemaphore handle;
	WGraphics graphics;
};
DEFINE_REF(Semaphore);

class DescriptorLayout : public Gfx::DescriptorLayout
{
public:
	DescriptorLayout(WGraphics graphics)
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
	WGraphics graphics;
	Array<VkDescriptorSetLayoutBinding> bindings;
	VkDescriptorSetLayout layoutHandle;
	friend class PipelineStateCacheManager;
};
DEFINE_REF(DescriptorLayout);
class PipelineLayout : public Gfx::PipelineLayout
{
public:
	PipelineLayout(WGraphics graphics)
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
	WGraphics graphics;
	friend class PipelineStateCacheManager;
};
DEFINE_REF(PipelineLayout);

class DescriptorSet : public Gfx::DescriptorSet
{
public:
	DescriptorSet(WGraphics graphics, PDescriptorAllocator owner)
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

private:
	virtual void writeChanges();
	Array<VkDescriptorImageInfo> imageInfos;
	Array<VkDescriptorBufferInfo> bufferInfos;
	Array<VkWriteDescriptorSet> writeDescriptors;
	VkDescriptorSet setHandle;
	PDescriptorAllocator owner;
	WGraphics graphics;
	friend class DescriptorAllocator;
};
DEFINE_REF(DescriptorSet);

class DescriptorAllocator : public Gfx::DescriptorAllocator
{
public:
	DescriptorAllocator(WGraphics graphics, DescriptorLayout &layout);
	~DescriptorAllocator();
	virtual void allocateDescriptorSet(Gfx::PDescriptorSet &descriptorSet);

	inline VkDescriptorPool getHandle()
	{
		return poolHandle;
	}

private:
	WGraphics graphics;
	int maxSets = 512;
	VkDescriptorPool poolHandle;
	DescriptorLayout &layout;
};
DEFINE_REF(DescriptorAllocator);
enum class QueueType
{
	GRAPHICS = 1,
	COMPUTE = 2,
	TRANSFER = 3,
	DEDICATED_TRANSFER = 4
};

struct QueueFamilyMapping
{
	uint32 graphicsFamily;
	uint32 computeFamily;
	uint32 transferFamily;
	uint32 dedicatedTransferFamily;
	uint32 getQueueTypeFamilyIndex(QueueType type)
	{
		switch (type)
		{
		case QueueType::GRAPHICS:
			return graphicsFamily;
		case QueueType::COMPUTE:
			return computeFamily;
		case QueueType::TRANSFER:
			return transferFamily;
		case QueueType::DEDICATED_TRANSFER:
			return dedicatedTransferFamily;
		default:
			return VK_QUEUE_FAMILY_IGNORED;
		}
	}
	bool needsTransfer(QueueType src, QueueType dst)
	{
		uint32 srcIndex = getQueueTypeFamilyIndex(src);
		uint32 dstIndex = getQueueTypeFamilyIndex(dst);
		return srcIndex != dstIndex;
	}
};
class QueueOwnedResource
{
public:
	QueueOwnedResource(WGraphics graphics, QueueType startQueueType);
	~QueueOwnedResource();
	PCommandBufferManager getCommands();
	//Preliminary checks to see if the barrier should be executed at all
	void transferOwnership(QueueType newOwner);

protected:
	virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
	QueueType currentOwner;
	WGraphics graphics;
	CommandBufferManager *cachedCmdBufferManager;
};
DEFINE_REF(QueueOwnedResource);

class Buffer : public QueueOwnedResource
{
public:
	Buffer(WGraphics graphics, uint32 size, VkBufferUsageFlags usage, QueueType queueType = QueueType::GRAPHICS);
	virtual ~Buffer();

private:
	virtual VkAccessFlags getSourceAccessMask() = 0;
	virtual VkAccessFlags getDestAccessMask() = 0;
	// Inherited via QueueOwnedResource
	virtual void executeOwnershipBarrier(QueueType newOwner);
};
DEFINE_REF(Buffer);

class UniformBuffer : public Buffer, public Gfx::UniformBuffer
{
};
DEFINE_REF(UniformBuffer);

class StructuredBuffer : public Buffer, public Gfx::StructuredBuffer
{
};
DEFINE_REF(StructuredBuffer);

class TextureBase
{
public:
	TextureBase();
	virtual ~TextureBase();
private:
	uint32 sizeX;
	uint32 sizeY;
	uint32 sizeZ;
	uint32 arrayCount;
	uint32 layerCount;
	VkImage image;
};
DEFINE_REF(TextureBase);

class Texture2D : public Gfx::Texture2D
{
public:
private:
	PTextureBase textureHandle;
};
DEFINE_REF(Texture2D);

class SamplerState
{
public:
	VkSampler sampler;
};
DEFINE_REF(SamplerState);

class Viewport : public Gfx::Viewport
{
public:
private:
	uint32 sizeX;
	uint32 sizeY;
	uint32 offsetX;
	uint32 offsetY;
	VkSwapchainKHR swapchain;
	void *windowHandle;
};
DECLARE_REF(Viewport);
} // namespace Vulkan
} // namespace Seele
