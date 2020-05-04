#include "ForwardPlusRenderPath.h"
#include "Scene/Scene.h"
#include "Material/MaterialInstance.h"
#include "Material/Material.h"
#include "GraphicsResources.h"

using namespace Seele;

Seele::ForwardPlusRenderPath::ForwardPlusRenderPath(Gfx::PGraphics graphics, Gfx::PViewport viewport)
    : SceneRenderPath(graphics, viewport)
{
    PMaterial material = new Material("D:\\Private\\Programming\\C++\\Seele\\test.mat");
    material->compile();
}

Seele::ForwardPlusRenderPath::~ForwardPlusRenderPath()
{
}

void Seele::ForwardPlusRenderPath::beginFrame()
{
}

void Seele::ForwardPlusRenderPath::render()
{
    for (auto entry : scene->getMeshBatches())
    {
        PMaterial material = entry.key;
        Gfx::PRenderCommand renderCommand = graphics->createRenderCommand();
        renderCommand->bindPipeline(material->getPipeline());
        for (auto drawState : entry.value.instances)
        {
            renderCommand->bindVertexBuffer(drawState.vertexBuffer);
            renderCommand->bindIndexBuffer(drawState.indexBuffer);
            renderCommand->bindDescriptor(drawState.instance->getDescriptor());
            renderCommand->draw(drawState);
        }
    }
}

void Seele::ForwardPlusRenderPath::endFrame()
{
}
