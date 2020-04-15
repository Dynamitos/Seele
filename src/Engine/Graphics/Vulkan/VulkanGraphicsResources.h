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
DECLARE_REF(SubAllocation);
class Semaphore
{
public:
	Semaphore(PGraphics graphics);
	virtual ~Semaphore();
	inline VkSemaphore getHandle() const
	{
		return handle;
	}

private:
	VkSemaphore handle;
	PGraphics graphics;
};
DEFINE_REF(Semaphore);

class Fence
{
public:
	Fence(PGraphics graphics);
	~Fence();
	bool isSignaled();
	void reset();
	inline VkFence getHandle() const
	{
		return fence;
	}
	void wait(uint32 timeout);

private:
	bool signaled;
	VkFence fence;
	PGraphics graphics;
};
DEFINE_REF(Fence);

struct QueueFamilyMapping
{
	uint32 graphicsFamily;
	uint32 computeFamily;
	uint32 transferFamily;
	uint32 dedicatedTransferFamily;
	uint32 getQueueTypeFamilyIndex(Gfx::QueueType type) const
	{
		switch (type)
		{
		case Gfx::QueueType::GRAPHICS:
			return graphicsFamily;
		case Gfx::QueueType::COMPUTE:
			return computeFamily;
		case Gfx::QueueType::TRANSFER:
			return transferFamily;
		case Gfx::QueueType::DEDICATED_TRANSFER:
			return dedicatedTransferFamily;
		default:
			return VK_QUEUE_FAMILY_IGNORED;
		}
	}
	bool needsTransfer(Gfx::QueueType src, Gfx::QueueType dst) const
	{
		uint32 srcIndex = getQueueTypeFamilyIndex(src);
		uint32 dstIndex = getQueueTypeFamilyIndex(dst);
		return srcIndex != dstIndex;
	}
};
class QueueOwnedResource
{
public:
	QueueOwnedResource(PGraphics graphics, Gfx::QueueType startQueueType);
	virtual ~QueueOwnedResource();
	PCommandBufferManager getCommands();
	//Preliminary checks to see if the barrier should be executed at all
	void transferOwnership(Gfx::QueueType newOwner);

protected:
	virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) = 0;
	Gfx::QueueType currentOwner;
	PGraphics graphics;
	PCommandBufferManager cachedCmdBufferManager;
};
DEFINE_REF(QueueOwnedResource);

class Buffer : public QueueOwnedResource
{
public:
	Buffer(PGraphics graphics, uint32 size, VkBufferUsageFlags usage, Gfx::QueueType queueType);
	virtual ~Buffer();
	VkBuffer getHandle() const
	{
		return buffers[currentBuffer].buffer;
	}
	void advanceBuffer()
	{
		currentBuffer = (currentBuffer + 1) % numBuffers;
	}
	void *lock(bool bWriteOnly = true);
	void unlock();

protected:
	struct BufferAllocation
	{
		VkBuffer buffer;
		PSubAllocation allocation;
	};
	BufferAllocation buffers[Gfx::numFramesBuffered];
	uint32 numBuffers;
	uint32 currentBuffer;
	uint32 size;

	virtual VkAccessFlags getSourceAccessMask() = 0;
	virtual VkAccessFlags getDestAccessMask() = 0;
	// Inherited via QueueOwnedResource
	virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
};
DEFINE_REF(Buffer);

class UniformBuffer : public Buffer, public Gfx::UniformBuffer
{
public:
	UniformBuffer(PGraphics graphics, const BulkResourceData &resourceData);
	virtual ~UniformBuffer();

protected:
	virtual VkAccessFlags getSourceAccessMask();
	virtual VkAccessFlags getDestAccessMask();
};
DEFINE_REF(UniformBuffer);

class StructuredBuffer : public Buffer, public Gfx::StructuredBuffer
{
public:
	StructuredBuffer(PGraphics graphics, const BulkResourceData &resourceData);
	virtual ~StructuredBuffer();

protected:
	virtual VkAccessFlags getSourceAccessMask();
	virtual VkAccessFlags getDestAccessMask();
};
DEFINE_REF(StructuredBuffer);

class VertexBuffer : public Buffer, public Gfx::VertexBuffer
{
public:
	VertexBuffer(PGraphics graphics, const BulkResourceData &resourceData);
	virtual ~VertexBuffer();

protected:
	virtual VkAccessFlags getSourceAccessMask();
	virtual VkAccessFlags getDestAccessMask();
};
DEFINE_REF(VertexBuffer);

class IndexBuffer : public Buffer, public Gfx::IndexBuffer
{
public:
	IndexBuffer(PGraphics graphics, const BulkResourceData &resourceData);
	virtual ~IndexBuffer();

protected:
	virtual VkAccessFlags getSourceAccessMask();
	virtual VkAccessFlags getDestAccessMask();
};
DEFINE_REF(IndexBuffer);

class TextureHandle : public QueueOwnedResource
{
public:
	TextureHandle(PGraphics graphics, VkImageViewType viewType, uint32 sizeX, uint32 sizeY, uint32 sizeZ,
				  bool bArray, uint32 arraySize, uint32 mipLevels, Gfx::SeFormat format,
				  uint32 samples, Gfx::SeImageUsageFlags usage, Gfx::QueueType owner = Gfx::QueueType::GRAPHICS, VkImage existingImage = VK_NULL_HANDLE);
	virtual ~TextureHandle();

	inline VkImageView getView() const
	{
		return defaultView;
	}
	inline VkImageLayout getLayout() const
	{
		return layout;
	}
	inline VkImageAspectFlags getAspect() const
	{
		return aspect;
	}
	inline VkImageUsageFlags getUsage() const
	{
		return usage;
	}
	inline Gfx::SeFormat getFormat() const
	{
		return format;
	}
	inline bool isDepthStencil() const
	{
		return aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
	}
	// Inherited via QueueOwnedResource
	virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);

private:
	PGraphics graphics;
	PSubAllocation allocation;
	uint32 sizeX;
	uint32 sizeY;
	uint32 sizeZ;
	uint32 arrayCount;
	uint32 mipLevels;
	uint32 samples;
	Gfx::SeFormat format;
	Gfx::SeImageUsageFlags usage;
	VkImage image;
	VkImageView defaultView;
	VkImageAspectFlags aspect;
	VkImageLayout layout;
	friend class TextureBase;
	friend class Texture2D;
};
DEFINE_REF(TextureHandle);

DECLARE_REF(TextureBase);
class TextureBase
{
public:
	static PTextureHandle cast(Gfx::PTexture texture)
	{
		PTextureBase base = texture.cast<TextureBase>();
		return base->textureHandle;
	}
	void changeLayout(VkImageLayout newLayout);

protected:
	PTextureHandle textureHandle;
};
DEFINE_REF(TextureBase);

class Texture2D : public TextureBase, public Gfx::Texture2D
{
public:
	Texture2D(PGraphics graphics, uint32 sizeX, uint32 sizeY,
			  bool bArray, uint32 arraySize, uint32 mipLevels, Gfx::SeFormat format,
			  uint32 samples, Gfx::SeImageUsageFlags usage, Gfx::QueueType owner = Gfx::QueueType::GRAPHICS, VkImage existingImage = VK_NULL_HANDLE);
	virtual ~Texture2D();
	inline uint32 getSizeX() const
	{
		return textureHandle->sizeX;
	}
	inline uint32 getSizeY() const
	{
		return textureHandle->sizeY;
	}
	inline Gfx::SeFormat getFormat() const
	{
		return textureHandle->format;
	}
	inline VkImage getHandle() const
	{
		return textureHandle->image;
	}
	inline VkImageView getView() const
	{
		return textureHandle->defaultView;
	}
	inline bool isDepthStencil() const
	{
		return textureHandle->isDepthStencil();
	}

private:
};
DEFINE_REF(Texture2D);

class SamplerState
{
public:
	VkSampler sampler;
};
DEFINE_REF(SamplerState);

class Window : public Gfx::Window
{
public:
	Window(PGraphics graphics, const WindowCreateInfo &createInfo);
	virtual ~Window();
	virtual void beginFrame() override;
	virtual void endFrame() override;
	virtual Gfx::PTexture2D getBackBuffer() override;
	virtual void onWindowCloseEvent() override;

protected:
	void advanceBackBuffer();
	void recreateSwapchain(const WindowCreateInfo &createInfo);
	void present();
	void destroySwapchain();
	void createSwapchain();
	void chooseSurfaceFormat(const Array<VkSurfaceFormatKHR> &available, Gfx::SeFormat preferred);
	void choosePresentMode(const Array<VkPresentModeKHR> &modes);
	PTexture2D backBufferImages[Gfx::numFramesBuffered];
	PSemaphore renderFinished[Gfx::numFramesBuffered];
	PSemaphore imageAcquired[Gfx::numFramesBuffered];
	PSemaphore imageAcquiredSemaphore;

	PGraphics graphics;
	VkFormat pixelFormat;
	VkPresentModeKHR presentMode;
	VkSwapchainKHR swapchain;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;
	void *windowHandle;
	int32 currentImageIndex;
	int32 acquiredImageIndex;
	int32 preAcquiredImageIndex;
	int32 semaphoreIndex;
	VkInstance instance;
};
DEFINE_REF(Window);

class Viewport : public Gfx::Viewport
{
public:
	Viewport(PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo);
	virtual ~Viewport();
	virtual void resize(uint32 newX, uint32 newY);
	virtual void move(uint32 newOffsetX, uint32 newOffsetY);
protected:
private:
	PGraphics graphics;
	friend class Graphics;
};
DECLARE_REF(Viewport);
} // namespace Vulkan
} // namespace Seele
