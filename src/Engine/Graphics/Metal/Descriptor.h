#pragma once
#include "Graphics/Descriptor.h"
#include "Graphics/Initializer.h"
#include "MinimalEngine.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
DECLARE_REF(DescriptorPool)
class DescriptorLayout : public Gfx::DescriptorLayout
{
public:
  DescriptorLayout(PGraphics graphics, const std::string& name);
  virtual ~DescriptorLayout();
  virtual void create() override;
private:
  PGraphics graphics;
};
class PipelineLayout : public Gfx::PipelineLayout
{
public:
  PipelineLayout(PGraphics graphics, Gfx::PPipelineLayout baseLayout)
    : Gfx::PipelineLayout(baseLayout)
    , graphics(graphics)
  {}
private:
  PGraphics graphics;
};
DEFINE_REF(PipelineLayout)

class DescriptorSet : public Gfx::DescriptorSet
{
public:
  DescriptorSet(PGraphics graphics, PDescriptorPool owner)
    : graphics(graphics)
    , owner(owner)
  {}
  virtual ~DescriptorSet();
private:
  PGraphics graphics;
  PDescriptorPool owner;
};
DEFINE_REF(DescriptorSet)

class DescriptorPool : public Gfx::DescriptorPool
{
public:
  DescriptorPool(PGraphics graphics);
private:
  PGraphics graphics;
};
}
}