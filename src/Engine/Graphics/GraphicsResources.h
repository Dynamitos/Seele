#pragma once
#include "GraphicsEnums.h"
#include "Containers/Array.h"
#include "Containers/List.h"
#include "GraphicsInitializer.h"
#include <boost/crc.hpp>
#include <functional>


#ifndef ENABLE_VALIDATION
#define ENABLE_VALIDATION 0
#endif

namespace Seele
{
struct VertexInputStream;
struct VertexStreamComponent;
class VertexInputType;
struct MeshBatchElement;
DECLARE_REF(MaterialAsset)
namespace Gfx
{
DECLARE_REF(Graphics)

class SamplerState
{
public:
    virtual ~SamplerState()
    {
    }
};
DEFINE_REF(SamplerState)

class Shader
{};
DEFINE_REF(Shader)

class VertexShader
{
public:
    VertexShader() {}
    virtual ~VertexShader() {}
};
DEFINE_REF(VertexShader)
class ControlShader
{
public:
    ControlShader() {}
    virtual ~ControlShader() {}
    uint32 getNumPatches() const { return numPatchPoints; }

protected:
    uint32 numPatchPoints;
};
DEFINE_REF(ControlShader)
class EvaluationShader
{
public:
    EvaluationShader() {}
    virtual ~EvaluationShader() {}
};
DEFINE_REF(EvaluationShader)
class GeometryShader
{
public:
    GeometryShader() {}
    virtual ~GeometryShader() {}
};
DEFINE_REF(GeometryShader)
class FragmentShader
{
public:
    FragmentShader() {}
    virtual ~FragmentShader() {}
};
DEFINE_REF(FragmentShader)

class ComputeShader
{
public:
    ComputeShader() {}
    virtual ~ComputeShader() {}
};
DEFINE_REF(ComputeShader)

//Uniquely identifies a permutation of shaders
//using the type parameters used to generate it
struct ShaderPermutation
{
    RenderPassType passType;
    char vertexInputName[15];
    char materialName[16];
    //TODO: lightmapping etc
};
//Hashed ShaderPermutation for fast lookup
struct PermutationId
{
    uint32 hash;
    PermutationId()
    {}
    PermutationId(ShaderPermutation permutation)
    {
        boost::crc_32_type result;
        result.process_bytes(&permutation, sizeof(ShaderPermutation));
        hash = result.checksum();
    }
    friend inline bool operator==(const PermutationId& lhs, const PermutationId& rhs)
    {
        return lhs.hash == rhs.hash;
    }
    friend inline bool operator!=(const PermutationId& lhs, const PermutationId& rhs)
    {
        return lhs.hash != rhs.hash;
    }
    friend inline bool operator<(const PermutationId& lhs, const PermutationId& rhs)
    {
        return lhs.hash < rhs.hash;
    }
    friend inline bool operator>(const PermutationId& lhs, const PermutationId& rhs)
    {
        return lhs.hash > rhs.hash;
    }
};
struct ShaderCollection
{
    PermutationId id;
    //PVertexDeclaration vertexDeclaration;
    PVertexShader vertexShader;
    PControlShader controlShader;
    PEvaluationShader evalutionShader;
    PGeometryShader geometryShader;
    PFragmentShader fragmentShader;
};
class ShaderMap
{
public:
    ShaderMap();
    ~ShaderMap();
    const ShaderCollection* findShaders(PermutationId&& id) const;
    ShaderCollection& createShaders(
        PGraphics graphics, 
        RenderPassType passName, 
        PMaterialAsset material, 
        VertexInputType* vertexInput,
        bool bPositionOnly);
private:
    std::mutex shadersLock;
    Array<ShaderCollection> shaders;
};
DEFINE_REF(ShaderMap)

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
    uint32_t binding;
    SeDescriptorType descriptorType;
    uint32_t descriptorCount;
    SeDescriptorBindingFlags bindingFlags = 0;
    SeShaderStageFlags shaderStages;
};
DEFINE_REF(DescriptorBinding)

DECLARE_REF(DescriptorSet)
class DescriptorAllocator
{
public:
    DescriptorAllocator() {}
    virtual ~DescriptorAllocator() {}
    virtual void allocateDescriptorSet(PDescriptorSet &descriptorSet) = 0;
    virtual void reset() = 0;
};
DEFINE_REF(DescriptorAllocator)
DECLARE_REF(UniformBuffer)
DECLARE_REF(StructuredBuffer)
DECLARE_REF(Texture)
class DescriptorSet
{
public:
    virtual ~DescriptorSet() {}
    virtual void writeChanges() = 0;
    virtual void updateBuffer(uint32 binding, PUniformBuffer uniformBuffer) = 0;
    virtual void updateBuffer(uint32 binding, PStructuredBuffer structuredBuffer) = 0;
    virtual void updateSampler(uint32 binding, PSamplerState samplerState) = 0;
    virtual void updateTexture(uint32 binding, PTexture texture, PSamplerState samplerState = nullptr) = 0;
    virtual void updateTextureArray(uint32_t binding, Array<PTexture> texture) = 0;
    virtual bool operator<(PDescriptorSet other) = 0;

    virtual uint32 getSetIndex() const = 0;
};
DEFINE_REF(DescriptorSet)

class DescriptorLayout
{
public:
    DescriptorLayout(const std::string& name)
        : setIndex(0)
        , name(name)
    {
    }
    virtual ~DescriptorLayout() {}
    DescriptorLayout& operator=(const DescriptorLayout &other)
    {
        if(this != &other)
        {
            descriptorBindings.resize(other.descriptorBindings.size());
            for(uint32 i = 0; i < descriptorBindings.size(); ++i)
            {
                descriptorBindings[i] = other.descriptorBindings[i];
            }
        }
        return *this;
    }
    virtual void create() = 0;
    virtual void addDescriptorBinding(uint32 binding, SeDescriptorType type, uint32 arrayCount = 1, SeDescriptorBindingFlags bindingFlags = 0);
    virtual void reset();
    virtual PDescriptorSet allocateDescriptorSet();
    const Array<DescriptorBinding> &getBindings() const { return descriptorBindings; }
    inline uint32 getSetIndex() const { return setIndex; }

protected:
    Array<DescriptorBinding> descriptorBindings;
    PDescriptorAllocator allocator;
    std::mutex allocatorLock;
    uint32 setIndex;
    std::string name;
    friend class PipelineLayout;
    friend class DescriptorAllocator;
};
DEFINE_REF(DescriptorLayout)
class PipelineLayout
{
public:
    PipelineLayout(PPipelineLayout baseLayout)
    {
        if(baseLayout != nullptr)
        {
            descriptorSetLayouts = baseLayout->descriptorSetLayouts;
            pushConstants = baseLayout->pushConstants;
        }
    }
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
DEFINE_REF(PipelineLayout)

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
    QueueOwnedResource(QueueFamilyMapping mapping, QueueType startQueueType);
    virtual ~QueueOwnedResource();

    //Preliminary checks to see if the barrier should be executed at all
    void transferOwnership(QueueType newOwner);
    void pipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess, SePipelineStageFlags dstStage);

protected:
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, 
        SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
    Gfx::QueueType currentOwner;
    QueueFamilyMapping mapping;
};
DEFINE_REF(QueueOwnedResource)

// IMPORTANT!! 
// WHEN DERIVING FROM ANY Gfx:: BASE CLASSES WITH MULTIPLE INHERITANCE
// ALWAYS PUT THE Gfx:: BASE CLASS FIRST
// This is because the refcounting object is unique per allocation, so
// the base address of both the Gfx:: and the implementation class
// need to match for it to work
class Buffer : public QueueOwnedResource
{
public:
    Buffer(QueueFamilyMapping mapping, QueueType startQueueType);
    virtual ~Buffer();

protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
};

class UniformBuffer : public Buffer
{
public:
    UniformBuffer(QueueFamilyMapping mapping, const BulkResourceData& resourceData);
    virtual ~UniformBuffer();
    // returns true if an update was performed, false if the old contents == new contents
    virtual bool updateContents(const BulkResourceData& resourceData);
    bool isDataEquals(UniformBuffer* other)
    {
        if(other == nullptr)
        {
            return false;
        }
        if(contents.size() != other->contents.size())
        {
            return false;
        }
        if(std::memcmp(contents.data(), other->contents.data(), contents.size()) != 0)
        {
            return false;
        }
        return true;
    }
protected:
    Array<uint8> contents;
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, 
        SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
};
DEFINE_REF(UniformBuffer)

class VertexBuffer : public Buffer
{
public:
    VertexBuffer(QueueFamilyMapping mapping, uint32 numVertices, uint32 vertexSize, QueueType startQueueType);
    virtual ~VertexBuffer();
    constexpr uint32 getNumVertices() const
    {
        return numVertices;
    }
    // Size of one vertex in bytes
    constexpr uint32 getVertexSize() const
    {
        return vertexSize;
    }

protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, 
        SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
    uint32 numVertices;
    uint32 vertexSize;
};
DEFINE_REF(VertexBuffer)

class IndexBuffer : public Buffer
{
public:
    IndexBuffer(QueueFamilyMapping mapping, uint32 size, Gfx::SeIndexType index, QueueType startQueueType);
    virtual ~IndexBuffer();
    constexpr uint32 getNumIndices() const
    {
        return numIndices;
    }
    constexpr Gfx::SeIndexType getIndexType() const
    {
        return indexType;
    }

protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, 
        SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
    Gfx::SeIndexType indexType;
    uint32 numIndices;
};
DEFINE_REF(IndexBuffer)

class StructuredBuffer : public Buffer
{
public:
    StructuredBuffer(QueueFamilyMapping mapping, uint32 stride, const BulkResourceData& bulkResourceData);
    virtual ~StructuredBuffer();
    virtual bool updateContents(const BulkResourceData& resourceData);
    bool isDataEquals(StructuredBuffer* other)
    {
        if(other == nullptr)
        {
            return false;
        }
        if(contents.size() != other->contents.size())
        {
            return false;
        }
        if(std::memcmp(contents.data(), other->contents.data(), contents.size()) != 0)
        {
            return false;
        }
        return true;
    }
    constexpr uint32 getStride() const
    {
        return stride;
    }
protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, 
        SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
    
    Array<uint8> contents;
    uint32 stride;
};
DEFINE_REF(StructuredBuffer)

class VertexStream
{
public:
    VertexStream();
    VertexStream(uint32 stride, uint32 offset, uint8 instanced);
    ~VertexStream();
    void addVertexElement(VertexElement element);
    const Array<VertexElement> getVertexDescriptions() const;
    inline uint8 isInstanced() const { return instanced; }

    uint32 stride;
    uint32 offset;
    Array<VertexElement> vertexDescription;
    uint8 instanced;
};
DEFINE_REF(VertexStream)
class VertexDeclaration
{
public:
    VertexDeclaration();
    ~VertexDeclaration();

    static PVertexDeclaration createDeclaration(PGraphics graphics, const Array<VertexElement>& elementList);
private:
};
DEFINE_REF(VertexDeclaration)
class GraphicsPipeline
{
public:
    GraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo, PPipelineLayout layout) : createInfo(createInfo) , layout(layout) {}
    virtual ~GraphicsPipeline(){}
    const GraphicsPipelineCreateInfo& getCreateInfo() const {return createInfo;}
    PPipelineLayout getPipelineLayout() const { return layout; }
protected:
    GraphicsPipelineCreateInfo createInfo;
    PPipelineLayout layout;
};
DEFINE_REF(GraphicsPipeline)

class ComputePipeline
{
public:
    ComputePipeline(const ComputePipelineCreateInfo& createInfo, PPipelineLayout layout) : createInfo(createInfo), layout(layout) {}
    virtual ~ComputePipeline(){}
    const ComputePipelineCreateInfo& getCreateInfo() const { return createInfo; }
    PPipelineLayout getPipelineLayout() const { return layout; }
protected:
    ComputePipelineCreateInfo createInfo;
    PPipelineLayout layout;
};
DEFINE_REF(ComputePipeline)

// IMPORTANT!! 
// WHEN DERIVING FROM ANY Gfx:: BASE CLASSES WITH MULTIPLE INHERITANCE
// ALWAYS PUT THE Gfx:: BASE CLASS FIRST
// This is because the refcounting object is unique per allocation, so
// the base address of both the Gfx:: and the implementation class
// need to match for it to work
class Texture : public QueueOwnedResource
{
public:
    Texture(QueueFamilyMapping mapping, QueueType startQueueType);
    virtual ~Texture();

    virtual SeFormat getFormat() const = 0;
    virtual uint32 getSizeX() const = 0;
    virtual uint32 getSizeY() const = 0;
    virtual uint32 getSizeZ() const = 0;
    virtual SeSampleCountFlags getNumSamples() const = 0;
    virtual uint32 getMipLevels() const = 0;
    virtual void changeLayout(SeImageLayout newLayout) = 0;
    virtual class Texture2D* getTexture2D() { return nullptr; }
    virtual void* getNativeHandle() { return nullptr; }
protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, 
        SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
};
DEFINE_REF(Texture)
class Texture2D : public Texture
{
public:
    Texture2D(QueueFamilyMapping mapping, QueueType startQueueType);
    virtual ~Texture2D();

    virtual SeFormat getFormat() const = 0;
    virtual uint32 getSizeX() const = 0;
    virtual uint32 getSizeY() const = 0;
    virtual uint32 getSizeZ() const = 0;
    virtual SeSampleCountFlags getNumSamples() const = 0;
    virtual uint32 getMipLevels() const = 0;
    virtual void changeLayout(SeImageLayout newLayout) = 0;
    virtual class Texture2D* getTexture2D() { return this; }
protected:
    //Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, 
        SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
};
DEFINE_REF(Texture2D)

DECLARE_REF(Viewport)
class RenderCommand
{
public:
    RenderCommand();
    virtual ~RenderCommand();
    virtual bool isReady() = 0;
    virtual void setViewport(Gfx::PViewport viewport) = 0;
    virtual void bindPipeline(Gfx::PGraphicsPipeline pipeline) = 0;
    virtual void bindDescriptor(Gfx::PDescriptorSet set) = 0;
    virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& sets) = 0;
    virtual void bindVertexBuffer(const Array<VertexInputStream>& streams) = 0;
    virtual void bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) = 0;
    virtual void pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) = 0;
    virtual void draw(const MeshBatchElement& data) = 0;
    virtual void draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) = 0;
    std::string name;
};
DEFINE_REF(RenderCommand)
class ComputeCommand
{
public:
    ComputeCommand();
    virtual ~ComputeCommand();
    virtual bool isReady() = 0;
    virtual void bindPipeline(Gfx::PComputePipeline pipeline) = 0;
    virtual void bindDescriptor(Gfx::PDescriptorSet set) = 0;
    virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& sets) = 0;
    virtual void pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) = 0;
    virtual void dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) = 0;
    std::string name;
};
DEFINE_REF(ComputeCommand)

class Window
{
public:
    Window(const WindowCreateInfo &createInfo);
    virtual ~Window();
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void onWindowCloseEvent() = 0;
    virtual PTexture2D getBackBuffer() const = 0;
    virtual void setKeyCallback(std::function<void(KeyCode, InputAction, KeyModifier)> callback) = 0;
    virtual void setMouseMoveCallback(std::function<void(double, double)> callback) = 0;
    virtual void setMouseButtonCallback(std::function<void(MouseButton, InputAction, KeyModifier)> callback) = 0;
    virtual void setScrollCallback(std::function<void(double, double)> callback) = 0;
    virtual void setFileCallback(std::function<void(int, const char**)> callback) = 0;
    virtual void setCloseCallback(std::function<void()> callback) = 0;
    SeFormat getSwapchainFormat() const
    {
        return windowState.pixelFormat;
    }
    SeSampleCountFlags getNumSamples() const
    {
        return windowState.numSamples;
    }
    uint32 getSizeX() const
    {
        return windowState.width;
    }
    uint32 getSizeY() const
    {
        return windowState.height;
    }

protected:
    WindowCreateInfo windowState;
};
DEFINE_REF(Window)

class Viewport
{
public:
    Viewport(PWindow owner, const ViewportCreateInfo &createInfo);
    virtual ~Viewport();
    virtual void resize(uint32 newX, uint32 newY) = 0;
    virtual void move(uint32 newOffsetX, uint32 newOffsetY) = 0;
    inline PWindow getOwner() const {return owner;}
    inline uint32 getSizeX() const {return sizeX;}
    inline uint32 getSizeY() const {return sizeY;}
    inline uint32 getOffsetX() const {return offsetX;}
    inline uint32 getOffsetY() const {return offsetY;}
protected:
    uint32 sizeX;
    uint32 sizeY;
    uint32 offsetX;
    uint32 offsetY;
    PWindow owner;
};
DEFINE_REF(Viewport)

class RenderTargetAttachment
{
public:
    RenderTargetAttachment(PTexture2D texture,
                           SeAttachmentLoadOp loadOp = SE_ATTACHMENT_LOAD_OP_LOAD,
                           SeAttachmentStoreOp storeOp = SE_ATTACHMENT_STORE_OP_STORE,
                           SeAttachmentLoadOp stencilLoadOp = SE_ATTACHMENT_LOAD_OP_DONT_CARE,
                           SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE)
        : loadOp(loadOp)
        , storeOp(storeOp)
        , stencilLoadOp(stencilLoadOp)
        , stencilStoreOp(stencilStoreOp)
        , texture(texture)
    {
    }
    virtual ~RenderTargetAttachment()
    {
    }
    virtual PTexture2D getTexture()
    {
        return texture;
    }
    virtual SeFormat getFormat() const
    {
        return texture->getFormat();
    }
    virtual SeSampleCountFlags getNumSamples() const
    {
        return texture->getNumSamples();
    }
    virtual uint32 getSizeX() const 
    { 
        return texture->getSizeX(); 
    }
    virtual uint32 getSizeY() const 
    { 
        return texture->getSizeY(); 
    }
    inline SeAttachmentLoadOp getLoadOp() const { return loadOp; }
    inline SeAttachmentStoreOp getStoreOp() const { return storeOp; }
    inline SeAttachmentLoadOp getStencilLoadOp() const { return stencilLoadOp; }
    inline SeAttachmentStoreOp getStencilStoreOp() const { return stencilStoreOp; }
    SeClearValue clear;
    SeColorComponentFlags componentFlags;
    SeAttachmentLoadOp loadOp;
    SeAttachmentStoreOp storeOp;
    SeAttachmentLoadOp stencilLoadOp;
    SeAttachmentStoreOp stencilStoreOp;
protected:
    PTexture2D texture;
};
DEFINE_REF(RenderTargetAttachment)

class SwapchainAttachment : public RenderTargetAttachment
{
public:
    SwapchainAttachment(PWindow owner,
                        SeAttachmentLoadOp loadOp = SE_ATTACHMENT_LOAD_OP_LOAD,
                        SeAttachmentStoreOp storeOp = SE_ATTACHMENT_STORE_OP_STORE,
                        SeAttachmentLoadOp stencilLoadOp = SE_ATTACHMENT_LOAD_OP_DONT_CARE,
                        SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE)
        : RenderTargetAttachment(nullptr, loadOp, storeOp, stencilLoadOp, stencilStoreOp), owner(owner)
    {
        clear.color.float32[0] = 0.0f;
        clear.color.float32[1] = 0.0f;
        clear.color.float32[2] = 0.0f;
        clear.color.float32[3] = 1.0f;
        componentFlags = SE_COLOR_COMPONENT_R_BIT | SE_COLOR_COMPONENT_G_BIT | SE_COLOR_COMPONENT_B_BIT | SE_COLOR_COMPONENT_A_BIT;
    }
    virtual PTexture2D getTexture() override
    {
        return owner->getBackBuffer();
    }
    virtual SeFormat getFormat() const override
    {
        return owner->getSwapchainFormat();
    }
    virtual SeSampleCountFlags getNumSamples() const override
    {
        return owner->getNumSamples();
    }
    virtual uint32 getSizeX() const 
    { 
        return owner->getSizeX(); 
    }
    virtual uint32 getSizeY() const 
    { 
        return owner->getSizeY(); 
    }
private:
    PWindow owner;
};
DEFINE_REF(SwapchainAttachment)

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
    uint32 width;
    uint32 height;
};
DEFINE_REF(RenderTargetLayout)

class RenderPass
{
public:
    RenderPass(PRenderTargetLayout layout) : layout(layout) {}
    virtual ~RenderPass() {}
    inline PRenderTargetLayout getLayout() const { return layout; }

protected:
    PRenderTargetLayout layout;
};
DEFINE_REF(RenderPass)
} // namespace Gfx
} // namespace Seele