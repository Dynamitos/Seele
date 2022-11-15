#pragma once
#include <vulkan/vulkan.h>
#include <functional>
#include "Graphics/GraphicsResources.h"
#include "VulkanAllocator.h"

namespace Seele
{
namespace Vulkan
{

DECLARE_REF(DescriptorAllocator)
DECLARE_REF(CommandBufferManager)
DECLARE_REF(CmdBuffer)
DECLARE_REF(Graphics)
DECLARE_REF(SubAllocation)
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
DEFINE_REF(Semaphore)

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
    /*Event& operator co_await()
    {
        return signaled;
    }*/
    bool operator<(const Fence &other) const
    {
        return fence < other.fence;
    }

private:
    PGraphics graphics;
    bool signaled;
    VkFence fence;
};
DEFINE_REF(Fence)

class VertexDeclaration : public Gfx::VertexDeclaration
{
public:
    Array<Gfx::VertexElement> elementList;

    VertexDeclaration(const Array<Gfx::VertexElement>& elementList);
    virtual ~VertexDeclaration();
private:
};
DEFINE_REF(VertexDeclaration)

class ShaderBuffer
{
public:
    ShaderBuffer(PGraphics graphics, uint64 size, VkBufferUsageFlags usage, Gfx::QueueType& queueType, bool bDynamic = false);
    virtual ~ShaderBuffer();
    VkBuffer getHandle() const
    {
        return buffers[currentBuffer].buffer;
    }
    uint64 getSize() const
    {
        return size;
    }
    VkDeviceSize getOffset() const;
    void advanceBuffer()
    {
        currentBuffer = (currentBuffer + 1) % numBuffers;
    }
    virtual void *lock(bool bWriteOnly = true);
    virtual void *lockRegion(uint64 regionOffset, uint64 regionSize, bool bWriteOnly = true);
    virtual void unlock();

protected:
    struct BufferAllocation
    {
        VkBuffer buffer;
        PSubAllocation allocation;
    };
    PGraphics graphics;
    uint32 currentBuffer;
    uint64 size;
    Gfx::QueueType& owner;
    BufferAllocation buffers[Gfx::numFramesBuffered];
    uint32 numBuffers;

    void executeOwnershipBarrier(Gfx::QueueType newOwner);
    void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
        
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) = 0;

    virtual VkAccessFlags getSourceAccessMask() = 0;
    virtual VkAccessFlags getDestAccessMask() = 0;
};
DEFINE_REF(ShaderBuffer)

DECLARE_REF(StagingBuffer)
class UniformBuffer : public Gfx::UniformBuffer, public ShaderBuffer
{
public:
    UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo &resourceData);
    virtual ~UniformBuffer();
    virtual bool updateContents(const BulkResourceData &resourceData);
    
    virtual void* lock(bool bWriteOnly = true) override;
    virtual void unlock() override;
protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask();
    virtual VkAccessFlags getDestAccessMask();
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner);
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);

private:
    PStagingBuffer dedicatedStagingBuffer;
};
DEFINE_REF(UniformBuffer)

class StructuredBuffer : public Gfx::StructuredBuffer, public ShaderBuffer
{
public:
    StructuredBuffer(PGraphics graphics, const StructuredBufferCreateInfo &resourceData);
    virtual ~StructuredBuffer();
    virtual bool updateContents(const BulkResourceData &resourceData);

    virtual void* lock(bool bWriteOnly = true) override;
    virtual void unlock() override;
protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask();
    virtual VkAccessFlags getDestAccessMask();
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner);
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
private:
    PStagingBuffer dedicatedStagingBuffer;
};
DEFINE_REF(StructuredBuffer)

class VertexBuffer : public Gfx::VertexBuffer, public ShaderBuffer
{
public:
    VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo &resourceData);
    virtual ~VertexBuffer();

    virtual void updateRegion(BulkResourceData update) override;

protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask();
    virtual VkAccessFlags getDestAccessMask();
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner);
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
};
DEFINE_REF(VertexBuffer)

class IndexBuffer : public Gfx::IndexBuffer, public ShaderBuffer
{
public:
    IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo &resourceData);
    virtual ~IndexBuffer();

protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask();
    virtual VkAccessFlags getDestAccessMask();
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner);
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
};
DEFINE_REF(IndexBuffer)

class TextureHandle
{
public:
    TextureHandle(PGraphics graphics, VkImageViewType viewType, 
        const TextureCreateInfo& createInfo, Gfx::QueueType& owner, VkImage existingImage = VK_NULL_HANDLE);
    virtual ~TextureHandle();

    inline VkImage getImage() const
    {
        return image;
    }
    inline VkImageView getView() const
    {
        return defaultView;
    }
    inline Gfx::SeImageLayout getLayout() const
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
    inline Gfx::SeSampleCountFlags getNumSamples() const
    {
        return samples;
    }
    inline uint32 getMipLevels() const
    {
        return mipLevels;
    }
    inline bool isDepthStencil() const
    {
        return aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    }
    void executeOwnershipBarrier(Gfx::QueueType newOwner);
    void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
    void changeLayout(Gfx::SeImageLayout newLayout);

private:
    //Updates via reference
    Gfx::QueueType& currentOwner;
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
    Gfx::SeImageLayout layout;
    friend class TextureBase;
    friend class Texture2D;
    friend class Graphics;
};

class TextureBase
{
public:
    static TextureHandle* cast(Gfx::PTexture texture);

protected:
    TextureHandle* textureHandle;
    friend class Graphics;
};
DECLARE_REF(TextureBase)
class Texture2D : public Gfx::Texture2D, public TextureBase
{
public:
    Texture2D(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage = VK_NULL_HANDLE);
    virtual ~Texture2D();
    virtual uint32 getSizeX() const override
    {
        return textureHandle->sizeX;
    }
    virtual uint32 getSizeY() const override
    {
        return textureHandle->sizeY;
    }
    virtual uint32 getSizeZ() const override
    {
        return textureHandle->sizeZ;
    }
    virtual Gfx::SeFormat getFormat() const override
    {
        return textureHandle->format;
    }
    virtual Gfx::SeSampleCountFlags getNumSamples() const override
    {
        return textureHandle->getNumSamples();
    }
    virtual uint32 getMipLevels() const override
    {
        return textureHandle->getMipLevels();
    }
    virtual void changeLayout(Gfx::SeImageLayout newLayout) override;
    virtual void* getNativeHandle() override
    {
        return textureHandle;
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
protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);

};
DEFINE_REF(Texture2D)

class SamplerState : public Gfx::SamplerState
{
public:
    VkSampler sampler;
};
DEFINE_REF(SamplerState)

class Window : public Gfx::Window
{
public:
    Window(PGraphics graphics, const WindowCreateInfo &createInfo);
    virtual ~Window();
    virtual void beginFrame() override;
    virtual void endFrame() override;
    virtual Gfx::PTexture2D getBackBuffer() const override;
    virtual void onWindowCloseEvent() override;
    virtual void setKeyCallback(std::function<void(KeyCode, InputAction, KeyModifier)> callback) override;
    virtual void setMouseMoveCallback(std::function<void(double, double)> callback) override;
    virtual void setMouseButtonCallback(std::function<void(MouseButton, InputAction, KeyModifier)> callback) override;
    virtual void setScrollCallback(std::function<void(double, double)> callback) override;
    virtual void setFileCallback(std::function<void(int, const char**)> callback) override;
    virtual void setCloseCallback(std::function<void()> callback);

    VkFormat getPixelFormat() const
    {
        return cast(windowState.pixelFormat);
    }

    std::function<void(KeyCode, InputAction, KeyModifier)> keyCallback;
    std::function<void(double, double)> mouseMoveCallback;
    std::function<void(MouseButton, InputAction, KeyModifier)> mouseButtonCallback;
    std::function<void(double, double)> scrollCallback;
    std::function<void(int, const char**)> fileCallback;
    std::function<void()> closeCallback;
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
    VkInstance instance;
    VkSwapchainKHR swapchain;
    VkSampleCountFlags numSamples;
    VkPresentModeKHR presentMode;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    void *windowHandle;
    int32 currentImageIndex;
    int32 acquiredImageIndex;
    int32 preAcquiredImageIndex;
    int32 semaphoreIndex;
};
DEFINE_REF(Window)

class Viewport : public Gfx::Viewport
{
public:
    Viewport(PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo);
    virtual ~Viewport();
    virtual void resize(uint32 newX, uint32 newY);
    virtual void move(uint32 newOffsetX, uint32 newOffsetY);
    VkViewport getHandle() const { return handle; }
private:
    VkViewport handle;
    PGraphics graphics;
    friend class Graphics;
};
DECLARE_REF(Viewport)
} // namespace Vulkan
} // namespace Seele
