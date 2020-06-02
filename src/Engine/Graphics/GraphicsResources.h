#pragma once
#include "GraphicsEnums.h"
#include "Containers/Array.h"
#include "Containers/List.h"
#include "Math/MemCRC.h"
#include "Material/MaterialInstance.h"
#include "GraphicsInitializer.h"
#include "MeshBatch.h"

#ifdef _DEBUG
#define ENABLE_VALIDATION
#endif

namespace Seele
{

namespace Gfx
{
DECLARE_REF(Graphics);

class SamplerState
{
public:
	virtual ~SamplerState()
	{
	}
};
DEFINE_REF(SamplerState);

class VertexShader
{
public:
	VertexShader() {}
	virtual ~VertexShader() {}
};
DEFINE_REF(VertexShader);
class ControlShader
{
public:
	ControlShader() {}
	virtual ~ControlShader() {}
	uint32 getNumPatches() const { return numPatchPoints; }

protected:
	uint32 numPatchPoints;
};
DEFINE_REF(ControlShader);
class EvaluationShader
{
public:
	EvaluationShader() {}
	virtual ~EvaluationShader() {}
};
DEFINE_REF(EvaluationShader);
class GeometryShader
{
public:
	GeometryShader() {}
	virtual ~GeometryShader() {}
};
DEFINE_REF(GeometryShader);
class FragmentShader
{
public:
	FragmentShader() {}
	virtual ~FragmentShader() {}
};
DEFINE_REF(FragmentShader);
class ComputeShader
{
public:
	ComputeShader() {}
	virtual ~ComputeShader() {}
};
DEFINE_REF(ComputeShader);

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

	virtual uint32 getSetIndex() const = 0;
};
DEFINE_REF(DescriptorSet);

class DescriptorLayout
{
public:
	DescriptorLayout()
		: setIndex(0)
	{
	}
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
	inline uint32 getSetIndex() const { return setIndex; }

protected:
	Array<DescriptorBinding> descriptorBindings;
	PDescriptorAllocator allocator;
	uint32 setIndex;
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
	virtual void reset() = 0;
	void addDescriptorLayout(uint32 setIndex, PDescriptorLayout layout);
	void addPushConstants(const SePushConstantRange &pushConstants);
	virtual uint32 getHash() const = 0;

protected:
	Array<PDescriptorLayout> descriptorSetLayouts;
	Array<SePushConstantRange> pushConstants;
};
DEFINE_REF(PipelineLayout);

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
			return 0x7fff;
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
	QueueOwnedResource(PGraphics graphics, QueueType startQueueType);
	virtual ~QueueOwnedResource();

	//Preliminary checks to see if the barrier should be executed at all
	void transferOwnership(QueueType newOwner);

protected:
	virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
	Gfx::QueueType currentOwner;
	PGraphics graphics;
};
DEFINE_REF(QueueOwnedResource);

class Buffer : public QueueOwnedResource
{
public:
	Buffer(PGraphics graphics, QueueType startQueueType);
	virtual ~Buffer();

protected:
	// Inherited via QueueOwnedResource
	virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
};

class UniformBuffer : public Buffer
{
public:
	UniformBuffer(PGraphics graphics, QueueType startQueueType);
	virtual ~UniformBuffer();
protected:
	// Inherited via QueueOwnedResource
	virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
};
DEFINE_REF(UniformBuffer);

class VertexBuffer : public Buffer
{
public:
	VertexBuffer(PGraphics graphics, uint32 numVertices, uint32 vertexSize, QueueType startQueueType);
	virtual ~VertexBuffer();
	inline uint32 getNumVertices()
	{
		return numVertices;
	}
	// Size of one vertex in bytes
	inline uint32 getVertexSize()
	{
		return vertexSize;
	}

protected:
	// Inherited via QueueOwnedResource
	virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
	uint32 numVertices;
	uint32 vertexSize;
};
DEFINE_REF(VertexBuffer);

class IndexBuffer : public Buffer
{
public:
	IndexBuffer(PGraphics graphics, uint32 size, Gfx::SeIndexType index, QueueType startQueueType);
	virtual ~IndexBuffer();
	inline uint32 getNumIndices() const
	{
		return numIndices;
	}
	inline Gfx::SeIndexType getIndexType() const
	{
		return indexType;
	}

protected:
	// Inherited via QueueOwnedResource
	virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
	Gfx::SeIndexType indexType;
	uint32 numIndices;
};
DEFINE_REF(IndexBuffer);

class StructuredBuffer : public Buffer
{
public:
	StructuredBuffer(PGraphics graphics, QueueType startQueueType);
	virtual ~StructuredBuffer();
protected:
	// Inherited via QueueOwnedResource
	virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
};
DEFINE_REF(StructuredBuffer);

class VertexStream
{
public:
	VertexStream() {}
	VertexStream(uint32 stride, uint8 instanced);
	~VertexStream();
	void addVertexElement(VertexElement element);
	const Array<VertexElement> getVertexDescriptions() const;
	inline uint8 isInstanced() const { return instanced; }

	uint32 stride;
	uint32 offset;
	Array<VertexElement> vertexDescription;
	uint8 instanced;
	PVertexBuffer vertexBuffer;
};
DEFINE_REF(VertexStream);
class VertexDeclaration
{
public:
	VertexDeclaration();
	~VertexDeclaration();
	uint32 addVertexStream(const VertexStream &vertexStream);
	const Array<VertexStream> &getVertexStreams() const;

private:
	Array<VertexStream> vertexStreams;
};
DEFINE_REF(VertexDeclaration);
class GraphicsPipeline
{
public:
	GraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo, PPipelineLayout layout) : createInfo(createInfo) , layout(layout) {}
	virtual ~GraphicsPipeline(){}
	const GraphicsPipelineCreateInfo& getCreateInfo() const {return createInfo;}
	PPipelineLayout getPipelineLayout() const { return layout; }
protected:
    PPipelineLayout layout;
	GraphicsPipelineCreateInfo createInfo;
};
DEFINE_REF(GraphicsPipeline);
class Texture : public QueueOwnedResource
{
public:
	Texture(PGraphics graphics, QueueType startQueueType);
	virtual ~Texture();
protected:
    // Inherited via QueueOwnedResource
	virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
};
DEFINE_REF(Texture);
class Texture2D : public Texture
{
public:
	Texture2D(PGraphics graphics, QueueType startQueueType);
	virtual ~Texture2D();
protected:
	//Inherited via QueueOwnedResource
	virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
};
DEFINE_REF(Texture2D);

class RenderCommand
{
public:
	RenderCommand();
	virtual ~RenderCommand();
	virtual void bindPipeline(Gfx::PGraphicsPipeline pipeline) = 0;
	virtual void bindDescriptor(Gfx::PDescriptorSet set) = 0;
	virtual void bindVertexBuffer(Gfx::PVertexBuffer vertexBuffer) = 0;
	virtual void bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) = 0;
	virtual void draw(const MeshBatchElement& data) = 0;
};
DEFINE_REF(RenderCommand);

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
	RenderPass(PRenderTargetLayout layout) : layout(layout) {}
	virtual ~RenderPass() {}
	inline PRenderTargetLayout getLayout() const { return layout; }

protected:
	PRenderTargetLayout layout;
};
DEFINE_REF(RenderPass);
} // namespace Gfx
} // namespace Seele