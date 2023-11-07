#pragma once
#include "Resources.h"

namespace Seele
{
namespace Gfx
{

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

DECLARE_REF(UniformBuffer)
class UniformBuffer : public Buffer
{
public:
    UniformBuffer(QueueFamilyMapping mapping, const DataSource& sourceData);
    virtual ~UniformBuffer();
    // returns true if an update was performed, false if the old contents == new contents
    virtual bool updateContents(const DataSource& sourceData);
    bool isDataEquals(PUniformBuffer other)
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

    virtual void updateRegion(DataSource update) = 0;
    virtual void download(Array<uint8>& buffer) = 0;
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
    IndexBuffer(QueueFamilyMapping mapping, uint64 size, Gfx::SeIndexType index, QueueType startQueueType);
    virtual ~IndexBuffer();
    constexpr uint64 getNumIndices() const
    {
        return numIndices;
    }
    constexpr Gfx::SeIndexType getIndexType() const
    {
        return indexType;
    }

    virtual void download(Array<uint8>& buffer) = 0;
protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, 
        SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
    Gfx::SeIndexType indexType;
    uint64 numIndices;
};
DEFINE_REF(IndexBuffer)

class ShaderBuffer : public Buffer
{
public:
    ShaderBuffer(QueueFamilyMapping mapping, uint32 stride, uint32 numElements, const DataSource& bulkResourceData);
    virtual ~ShaderBuffer();
    virtual bool updateContents(const DataSource& sourceData);
    bool isDataEquals(ShaderBuffer* other)
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
    constexpr uint32 getNumElements() const
    {
        return numElements;
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
    uint32 numElements;
    uint32 stride;
};
DEFINE_REF(ShaderBuffer)

}
}