#pragma once
#include "MinimalEngine.h"
#include "MeshProcessor.h"

namespace Seele
{
class BasePassMeshProcessor : public MeshProcessor
{
public:
    BasePassMeshProcessor(const PScene scene, Gfx::PGraphics graphics, uint8 translucentBasePass);
    virtual ~BasePassMeshProcessor();

    virtual void addMeshBatch(
        const MeshBatch& batch, 
        const PPrimitiveComponent primitiveComponent,
        const Gfx::PRenderPass renderPass,
        int32 staticMeshId = -1) override;
    Array<Gfx::PRenderCommand> getRenderCommands();
    void clearCommands();
private:
    Array<Gfx::PRenderCommand> renderCommands;
    uint8 translucentBasePass;
};
DEFINE_REF(BasePassMeshProcessor);
class BasePass
{
public:
    BasePass(const PScene scene, Gfx::PGraphics graphics, Gfx::PViewport viewport);
    ~BasePass();
    void render();
private:
    Gfx::PRenderPass renderPass;
    Gfx::PTexture2D depthBuffer;
    const PScene scene;
    UPBasePassMeshProcessor processor;
    Gfx::PGraphics graphics;
};
DEFINE_REF(BasePass);
} // namespace Seele
