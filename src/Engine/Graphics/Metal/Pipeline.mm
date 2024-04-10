#include "Pipeline.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Graphics.h"
#include "Graphics/Resources.h"
#include "Metal/MTLRenderPipeline.hpp"

using namespace Seele;
using namespace Seele::Metal;

VertexInput::VertexInput(VertexInputStateCreateInfo createInfo)
  : Gfx::VertexInput(createInfo)
{
}

VertexInput::~VertexInput()
{
}
