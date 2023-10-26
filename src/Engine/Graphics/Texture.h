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
class Texture : public QueueOwnedResource
{
public:
    Texture(QueueFamilyMapping mapping, QueueType startQueueType);
    virtual ~Texture();

    virtual SeFormat getFormat() const = 0;
    virtual uint32 getSizeX() const = 0;
    virtual uint32 getSizeY() const = 0;
    virtual uint32 getSizeZ() const = 0;
    virtual uint32 getNumFaces() const { return 1; }
    virtual SeSampleCountFlags getNumSamples() const = 0;
    virtual uint32 getMipLevels() const = 0;
    virtual void changeLayout(SeImageLayout newLayout) = 0;
    virtual class Texture2D* getTexture2D() { return nullptr; }
    virtual class Texture3D* getTexture3D() { return nullptr; }
    virtual class TextureCube* getTextureCube() { return nullptr; }
    virtual void* getNativeHandle() { return nullptr; }
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) = 0;
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

class Texture3D : public Texture
{
public:
    Texture3D(QueueFamilyMapping mapping, QueueType startQueueType);
    virtual ~Texture3D();

    virtual SeFormat getFormat() const = 0;
    virtual uint32 getSizeX() const = 0;
    virtual uint32 getSizeY() const = 0;
    virtual uint32 getSizeZ() const = 0;
    virtual SeSampleCountFlags getNumSamples() const = 0;
    virtual uint32 getMipLevels() const = 0;
    virtual void changeLayout(SeImageLayout newLayout) = 0;
    virtual class Texture3D* getTexture3D() { return this; }
protected:
    //Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, 
        SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
};
DEFINE_REF(Texture3D)

class TextureCube : public Texture
{
public:
    TextureCube(QueueFamilyMapping mapping, QueueType startQueueType);
    virtual ~TextureCube();

    virtual SeFormat getFormat() const = 0;
    virtual uint32 getSizeX() const = 0;
    virtual uint32 getSizeY() const = 0;
    virtual uint32 getSizeZ() const = 0;
    virtual uint32 getNumFaces() const { return 6; }
    virtual SeSampleCountFlags getNumSamples() const = 0;
    virtual uint32 getMipLevels() const = 0;
    virtual void changeLayout(SeImageLayout newLayout) = 0;
    virtual class TextureCube* getTextureCube() { return this; }
protected:
    //Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, 
        SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
};
DEFINE_REF(TextureCube)

}
}