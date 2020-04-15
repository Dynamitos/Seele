#pragma once
#include "GraphicsEnums.h"
#include "Containers/Array.h"
#include "Math/MemCRC.h"

#ifdef _DEBUG
#define ENABLE_VALIDATION
#endif

namespace Seele
{
namespace Gfx
{
enum class QueueType
{
	GRAPHICS = 1,
	COMPUTE = 2,
	TRANSFER = 3,
	DEDICATED_TRANSFER = 4
};
} // namespace Gfx
struct GraphicsInitializer
{
	const char *windowLayoutFile;
	const char *applicationName;
	const char *engineName;
	void *windowHandle;
	/**
	 * layers defines the enabled Vulkan layers used in the instance,
	 * if ENABLE_VALIDATION is defined, standard validation is already enabled
	 * not yet implemented
	 */
	Array<const char *> layers;
	Array<const char *> instanceExtensions;
	Array<const char *> deviceExtensions;
	GraphicsInitializer()
		: applicationName("SeeleEngine"), engineName("SeeleEngine"), layers{"VK_LAYER_LUNARG_standard_validation"}, instanceExtensions{}, deviceExtensions{"VK_KHR_swapchain"}, windowHandle(nullptr)
	{
	}
	GraphicsInitializer(const GraphicsInitializer &other)
		: applicationName(other.applicationName), engineName(other.engineName), layers(other.layers), instanceExtensions(other.instanceExtensions), deviceExtensions(other.deviceExtensions)
	{
	}
};
struct WindowCreateInfo
{
	int32 width;
	int32 height;
	const char *title;
	bool bFullscreen;
	Gfx::SeFormat pixelFormat;
	void *windowHandle;
};
struct ViewportCreateInfo
{
	uint32 sizeX;
	uint32 sizeY;
	uint32 offsetX;
	uint32 offsetY;
	bool bFullscreen;
	bool bVsync;
};
struct TextureCreateInfo
{
	uint32 width;
	uint32 height;
	uint32 depth;
	bool bArray;
	uint32 arrayLayers;
	uint32 mipLevels;
	uint32 samples;
	Gfx::SeFormat format;
	Gfx::SeImageUsageFlagBits usage;
	Gfx::QueueType queueType;
	TextureCreateInfo()
		: width(1), height(1), depth(1), bArray(false), arrayLayers(1), mipLevels(1), samples(1), format(Gfx::SE_FORMAT_R32G32B32A32_SFLOAT), usage(Gfx::SE_IMAGE_USAGE_SAMPLED_BIT)
	{
	}
};
//doesnt own the data, only proxy it
struct BulkResourceData
{
	uint32 size;
	uint8 *data;
	Gfx::QueueType owner;
};
namespace Gfx
{
struct SePushConstantRange
{
	SeShaderStageFlags stageFlags;
	uint32_t offset;
	uint32_t size;
};
class RenderCommandBase
{
public:
	virtual ~RenderCommandBase()
	{
	}
};
DEFINE_REF(RenderCommandBase);
class SamplerState
{
public:
	virtual ~SamplerState()
	{
	}
};
DEFINE_REF(SamplerState);
class DescriptorBinding
{
public:
	DescriptorBinding()
		: binding(0), descriptorType(SE_DESCRIPTOR_TYPE_MAX_ENUM), descriptorCount(0x7fff), shaderStages(SE_SHADER_STAGE_ALL)
	{
	}
	DescriptorBinding(const DescriptorBinding &other)
		: binding(other.binding), descriptorType(other.descriptorType), descriptorCount(other.descriptorCount), shaderStages(other.shaderStages)
	{
	}
	void operator=(const DescriptorBinding &other)
	{
		binding = other.binding;
		descriptorType = other.descriptorType;
		descriptorCount = other.descriptorCount;
		shaderStages = other.shaderStages;
	}
	uint32_t binding;
	SeDescriptorType descriptorType;
	uint32_t descriptorCount;
	SeShaderStageFlags shaderStages;
};
DEFINE_REF(DescriptorBinding);

DECLARE_REF(DescriptorSet);
class DescriptorAllocator
{
public:
	DescriptorAllocator() {}
	virtual ~DescriptorAllocator() {}
	virtual void allocateDescriptorSet(PDescriptorSet &descriptorSet) = 0;
};
DEFINE_REF(DescriptorAllocator);
DECLARE_REF(UniformBuffer);
DECLARE_REF(StructuredBuffer);
DECLARE_REF(Texture);
class DescriptorSet
{
public:
	virtual ~DescriptorSet() {}
	virtual void writeChanges() = 0;
	virtual void updateBuffer(uint32 binding, PUniformBuffer uniformBuffer) = 0;
	virtual void updateBuffer(uint32 binding, PStructuredBuffer structuredBuffer) = 0;
	virtual void updateSampler(uint32 binding, PSamplerState samplerState) = 0;
	virtual void updateTexture(uint32 binding, PTexture texture, PSamplerState samplerState = nullptr) = 0;
	virtual bool operator<(PDescriptorSet other) = 0;
};
DEFINE_REF(DescriptorSet);

class DescriptorLayout
{
public:
	DescriptorLayout() {}
	virtual ~DescriptorLayout() {}
	void operator=(const DescriptorLayout &other)
	{
		descriptorBindings.resize(other.descriptorBindings.size());
		std::memcpy(descriptorBindings.data(), other.descriptorBindings.data(), sizeof(DescriptorLayout) * descriptorBindings.size());
	}
	virtual void create() = 0;
	virtual void addDescriptorBinding(uint32 binding, SeDescriptorType type, uint32 arrayCount = 1);
	virtual PDescriptorSet allocatedDescriptorSet();
	const Array<DescriptorBinding> &getBindings() const { return descriptorBindings; }

protected:
	Array<DescriptorBinding> descriptorBindings;
	PDescriptorAllocator allocator;
	friend class PipelineLayout;
	friend class DescriptorAllocator;
};
DEFINE_REF(DescriptorLayout);
class PipelineLayout
{
public:
	PipelineLayout() {}
	virtual ~PipelineLayout() {}
	virtual void create() = 0;
	void addDescriptorLayout(uint32 setIndex, PDescriptorLayout layout);
	void addPushConstants(const SePushConstantRange &pushConstants);
	virtual uint32 getHash() const = 0;

protected:
	Array<PDescriptorLayout> descriptorSetLayouts;
	Array<SePushConstantRange> pushConstants;
};
DEFINE_REF(PipelineLayout);
class VertexDeclaration
{
public:
	virtual ~VertexDeclaration()
	{
	}
};
DEFINE_REF(VertexDeclaration);
class GraphicsPipeline
{
public:
	virtual ~GraphicsPipeline()
	{
	}
};
DEFINE_REF(GraphicsPipeline);
class UniformBuffer
{
public:
	UniformBuffer();
	virtual ~UniformBuffer();
};
DEFINE_REF(UniformBuffer);
class VertexBuffer
{
public:
	virtual ~VertexBuffer()
	{
	}
};
DEFINE_REF(VertexBuffer);
class IndexBuffer
{
public:
	virtual ~IndexBuffer()
	{
	}
};
DEFINE_REF(IndexBuffer);
class StructuredBuffer
{
public:
	virtual ~StructuredBuffer()
	{
	}
};
DEFINE_REF(StructuredBuffer);
class Texture
{
public:
	virtual ~Texture()
	{
	}
};
DEFINE_REF(Texture);
class Texture2D : public Texture
{
public:
	virtual ~Texture2D()
	{
	}
};
DEFINE_REF(Texture2D);

class Window
{
public:
	Window(const WindowCreateInfo &createInfo);
	virtual ~Window();
	virtual void beginFrame() = 0;
	virtual void endFrame() = 0;
	virtual void onWindowCloseEvent() = 0;
	virtual PTexture2D getBackBuffer() = 0;

protected:
	uint32 sizeX;
	uint32 sizeY;
	bool bFullscreen;
	const char *title;
	SeFormat pixelFormat;
};
DEFINE_REF(Window);

class Viewport
{
public:
	Viewport(PWindow owner, const ViewportCreateInfo &createInfo);
	virtual ~Viewport();
	virtual void resize(uint32 newX, uint32 newY) = 0;
	virtual void move(uint32 newOffsetX, uint32 newOffsetY) = 0;
protected:
	uint32 sizeX;
	uint32 sizeY;
	uint32 offsetX;
	uint32 offsetY;
	PWindow owner;
};
DEFINE_REF(Viewport);

class RenderTargetAttachment
{
public:
	RenderTargetAttachment(PTexture2D texture,
						   SeAttachmentLoadOp loadOp = SE_ATTACHMENT_LOAD_OP_LOAD,
						   SeAttachmentStoreOp storeOp = SE_ATTACHMENT_STORE_OP_STORE,
						   SeAttachmentLoadOp stencilLoadOp = SE_ATTACHMENT_LOAD_OP_DONT_CARE,
						   SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE)
		: texture(texture), loadOp(loadOp), storeOp(storeOp), stencilLoadOp(stencilLoadOp), stencilStoreOp(stencilStoreOp)
	{
	}
	virtual ~RenderTargetAttachment()
	{
	}
	virtual PTexture2D getTexture()
	{
		return texture;
	}
	inline SeAttachmentLoadOp getLoadOp() const { return loadOp; }
	inline SeAttachmentStoreOp getStoreOp() const { return storeOp; }
	inline SeAttachmentLoadOp getStencilLoadOp() const { return stencilLoadOp; }
	inline SeAttachmentStoreOp getStencilStoreOp() const { return stencilStoreOp; }

protected:
	PTexture2D texture;
	SeAttachmentLoadOp loadOp;
	SeAttachmentStoreOp storeOp;
	SeAttachmentLoadOp stencilLoadOp;
	SeAttachmentStoreOp stencilStoreOp;
};
DEFINE_REF(RenderTargetAttachment);

class SwapchainAttachment : public RenderTargetAttachment
{
public:
	SwapchainAttachment(PWindow owner,
						SeAttachmentLoadOp loadOp = SE_ATTACHMENT_LOAD_OP_LOAD,
						SeAttachmentStoreOp storeOp = SE_ATTACHMENT_STORE_OP_STORE,
						SeAttachmentLoadOp stencilLoadOp = SE_ATTACHMENT_LOAD_OP_DONT_CARE,
						SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE)
		: RenderTargetAttachment(owner->getBackBuffer(), loadOp, storeOp, stencilLoadOp, stencilStoreOp), owner(owner)
	{
	}
	virtual PTexture2D getTexture() override
	{
		return owner->getBackBuffer();
	}

private:
	PWindow owner;
};
DEFINE_REF(SwapchainAttachment);

class RenderTargetLayout
{
public:
	RenderTargetLayout();
	RenderTargetLayout(PRenderTargetAttachment depthAttachment);
	RenderTargetLayout(PRenderTargetAttachment colorAttachment, PRenderTargetAttachment depthAttachment);
	RenderTargetLayout(Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachmet);
	RenderTargetLayout(Array<PRenderTargetAttachment> inputAttachments, Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachment);
	Array<PRenderTargetAttachment> inputAttachments;
	Array<PRenderTargetAttachment> colorAttachments;
	PRenderTargetAttachment depthAttachment;
};
DEFINE_REF(RenderTargetLayout);

class RenderPass
{
public:
	virtual ~RenderPass()
	{
	}
};
DEFINE_REF(RenderPass);

DEFINE_REF(RenderPass);
} // namespace Gfx
} // namespace Seele