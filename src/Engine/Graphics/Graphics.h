#pragma once
#include "MinimalEngine.h"
#include "GraphicsResources.h"
#include "Containers/Array.h"

namespace Seele
{
namespace Gfx
{
class Graphics
{
public:
    Graphics();
    virtual ~Graphics();
    virtual void init(GraphicsInitializer initializer) = 0;
    virtual PWindow createWindow(const WindowCreateInfo &createInfo) = 0;
    virtual PViewport createViewport(PWindow owner, const ViewportCreateInfo &createInfo) = 0;

    virtual PRenderPass createRenderPass(PRenderTargetLayout layout) = 0;
    virtual void beginRenderPass(PRenderPass renderPass) = 0;
    virtual void endRenderPass() = 0;

    virtual PTexture2D createTexture2D(const TextureCreateInfo &createInfo) = 0;
    virtual PUniformBuffer createUniformBuffer(const BulkResourceData &bulkData) = 0;
    virtual PStructuredBuffer createStructuredBuffer(const BulkResourceData &bulkData) = 0;
    virtual PVertexBuffer createVertexBuffer(const BulkResourceData &bulkData) = 0;
    virtual PIndexBuffer createIndexBuffer(const BulkResourceData &bulkData) = 0;

protected:
    friend class Window;
};
DEFINE_REF(Graphics);
} // namespace Gfx
} // namespace Seele