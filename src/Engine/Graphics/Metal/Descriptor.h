#pragma once
#include "Graphics/Descriptor.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
class DescriptorLayout : public Gfx::DescriptorLayout
{
public:
  DescriptorLayout(PGraphics graphics, const std::string& name);
  virtual ~DescriptorLayout();
  virtual void create();
private:
  PGraphics graphics;
};
class PipelineLayout : public Gfx::PipelineLayout
{
public:
private:

};
}
}