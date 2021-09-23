#include "RenderPass.h"

using namespace Seele;

RenderPass::RenderPass(PRenderGraph renderGraph, Gfx::PGraphics graphics, Gfx::PViewport viewport) 
    : renderGraph(renderGraph), graphics(graphics), viewport(viewport)
{
}

RenderPass::~RenderPass() 
{
}

