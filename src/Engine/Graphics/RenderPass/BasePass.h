#pragma once
#include "MinimalEngine.h"
#include "MeshProcessor.h"

namespace Seele
{
class BasePassMeshProcessor : public MeshProcessor
{
public:
    BasePassMeshProcessor(const PScene scene, Gfx::PViewport viewport, Gfx::PGraphics graphics, uint8 translucentBasePass);
    virtual ~BasePassMeshProcessor();

    virtual void addMeshBatch(
        const MeshBatch& batch, 
//        const PPrimitiveComponent primitiveComponent,
        const Gfx::PRenderPass renderPass,
        Gfx::PPipelineLayout pipelineLayout,
        Gfx::PDescriptorLayout primitiveLayout,
        Array<Gfx::PDescriptorSet>& descriptorSets,
        int32 staticMeshId = -1) override;
    Array<Gfx::PRenderCommand> getRenderCommands();
    void clearCommands();
private:
    Array<Gfx::PRenderCommand> renderCommands;
    Array<Gfx::PDescriptorSet> cachedPrimitiveSets;
    uint32 cachedPrimitiveIndex;
    Gfx::PViewport target;
    uint8 translucentBasePass;
};
DEFINE_REF(BasePassMeshProcessor);
DECLARE_REF(CameraActor);
DECLARE_REF(CameraComponent);
class BasePass
{
public:
    BasePass(const PScene scene, Gfx::PGraphics graphics, Gfx::PViewport viewport, PCameraActor source);
    ~BasePass();
    void beginFrame();
    void render();
    void endFrame();
private:
    struct ViewParameter
    {
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
        Vector4 cameraPosition;
    } viewParams;
    struct ScreenToViewParameter
    {
        Matrix4 inverseProjectionMatrix;
        Vector2 screenDimensions;
    } screenToViewParams;

    Gfx::PRenderPass renderPass;
    Gfx::PTexture2D depthBuffer;
    const PScene scene;
    UPBasePassMeshProcessor processor;
    Gfx::PPipelineLayout basePassLayout;
    // Set 0: Light environment
    Gfx::PDescriptorLayout lightLayout;
    Gfx::PUniformBuffer lightUniform;
    // Set 1: viewParameter
    Gfx::PDescriptorLayout viewLayout;
    Gfx::PUniformBuffer viewParamBuffer;
    Gfx::PUniformBuffer screenToViewParamBuffer;
    // Set 2: materials, generated
    // Set 3: primitive scene data
    Gfx::PDescriptorLayout primitiveLayout;
    Gfx::PUniformBuffer primitiveUniformBuffer;
    Array<Gfx::PDescriptorSet> descriptorSets;
    Gfx::PGraphics graphics;
    PCameraComponent source;
    Gfx::PViewport viewport;
};
DEFINE_REF(BasePass);
} // namespace Seele
