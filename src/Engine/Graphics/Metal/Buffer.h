#pragma once
#include "Graphics/Buffer.h"
#include "Graphics/Initializer.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
class Buffer
{

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