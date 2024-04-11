#pragma once
#include "Graphics/Buffer.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Resources.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
class Buffer
{
public:
  Buffer(PGraphics graphics, uint64 size, void* data, bool dynamic);
  virtual ~Buffer();
  MTL::Buffer* getHandle() const
  {
    return buffers[currentBuffer];
  }
  uint64 getSize() const
  {
    return size;
  }
  void advanceBuffer()
  {
    currentBuffer = (currentBuffer + 1) % numBuffers;
  }
  void* map(bool writeOnly = true);
  void* mapRegion(uint64 regionOffset, uint64 regionSize, bool writeOnly);
  void unmap();
private:
  PGraphics graphics;
  uint32 currentBuffer;
  uint64 size;
  MTL::Buffer* buffers[Gfx::numFramesBuffered];
  uint32 numBuffers;
};
DEFINE_REF(Buffer)
class VertexBuffer : public Gfx::VertexBuffer, public Buffer
{
public:
  VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo& createInfo);
  virtual ~VertexBuffer();
private:
};
DEFINE_REF(VertexBuffer)
class IndexBuffer : public Gfx::IndexBuffer, public Buffer
{
public:
  IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo& createInfo);
  virtual ~IndexBuffer();
private:
};
DEFINE_REF(IndexBuffer)
class UniformBuffer : public Gfx::UniformBuffer, public Buffer
{
public:
  UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo& createInfo);
  virtual ~UniformBuffer();
private:
};
DEFINE_REF(UniformBuffer)
class ShaderBuffer : public Gfx::ShaderBuffer, public Buffer
{
public:
  ShaderBuffer(PGraphics graphics, const ShaderBufferCreateInfo& createInfo);
  virtual ~ShaderBuffer();
private:
};
DEFINE_REF(ShaderBuffer)
}
}