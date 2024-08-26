#pragma once
#include "Graphics/Buffer.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Metal/MTLResource.hpp"
#include "Metal/MTLTypes.hpp"
#include "MinimalEngine.h"
#include "Resources.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
class BufferAllocation : public CommandBoundResource {
  public:
    BufferAllocation(PGraphics graphics, const std::string& name, uint64 size, MTL::ResourceOptions options = MTL::ResourceOptionCPUCacheModeDefault);
    virtual ~BufferAllocation();
    void pipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage);
    void transferOwnership(Gfx::QueueType newOwner);
    void updateContents(uint64 regionOffset, uint64 regionSize, void* ptr);
    void readContents(uint64 regionOffset, uint64 regionSize, void* ptr);
    void* map();
    void unmap();
    MTL::Buffer* buffer = nullptr;
    uint64 size = 0;
};
DECLARE_REF(BufferAllocation)
class Buffer {
  public:
    Buffer(PGraphics graphics, uint64 size, Gfx::SeBufferUsageFlags usage, Gfx::QueueType queueType, bool dynamic, std::string name,
               bool createCleared = false, uint32 clearValue = 0);
    virtual ~Buffer();
    MTL::Buffer* getHandle() const { return buffers[currentBuffer]->buffer; }
    PBufferAllocation getAlloc() const { return buffers[currentBuffer]; }
    uint64 getSize() const { return buffers[currentBuffer]->size; }
    void updateContents(uint64 regionOffset, uint64 regionSize, void* ptr);
    void readContents(uint64 regionOffset, uint64 regionSize, void* ptr);


  protected:
    PGraphics graphics;
    uint32 currentBuffer;
    Gfx::QueueType initialOwner;
    Array<OBufferAllocation> buffers;
    Gfx::SeBufferUsageFlags usage;
    bool dynamic;
    bool createCleared;
    std::string name;
    uint32 clearValue;
    void rotateBuffer(uint64 size, bool preserveContents = false);
    void createBuffer(uint64 size, uint32 destIndex);
    void copyBuffer(uint64 src, uint64 dest);

    void transferOwnership(Gfx::QueueType newOwner);
    void pipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage);

};
DEFINE_REF(Buffer)

class VertexBuffer : public Gfx::VertexBuffer, public Buffer {
  public:
    VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo& createInfo);
    virtual ~VertexBuffer();
    virtual void updateRegion(DataSource update) override;
    virtual void download(Array<uint8>& buffer) override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(VertexBuffer)
class IndexBuffer : public Gfx::IndexBuffer, public Buffer {
  public:
    IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo& createInfo);
    virtual ~IndexBuffer();

    virtual void download(Array<uint8>& buffer) override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(IndexBuffer)
DEFINE_REF(IndexBuffer)
class UniformBuffer : public Gfx::UniformBuffer, public Buffer {
  public:
    UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo& createInfo);
    virtual ~UniformBuffer();

    virtual void rotateBuffer(uint64 size) override;
    virtual void updateContents(const DataSource& sourceData) override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(UniformBuffer)
class ShaderBuffer : public Gfx::ShaderBuffer, public Buffer {
  public:
    ShaderBuffer(PGraphics graphics, const ShaderBufferCreateInfo& createInfo);
    virtual ~ShaderBuffer();
    virtual void readContents(Array<uint8>& data) override;
    virtual void rotateBuffer(uint64 size, bool preserveContents = false) override;
    virtual void updateContents(const ShaderBufferCreateInfo& sourceData) override;
    virtual void clear() override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(ShaderBuffer)
} // namespace Metal
} // namespace Seele
