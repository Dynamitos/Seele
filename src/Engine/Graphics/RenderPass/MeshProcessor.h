#pragma once
#include "MinimalEngine.h"
#include "Scene/Scene.h"
#include "Graphics/GraphicsResources.h"
#include "Graphics/MeshBatch.h"

namespace Seele
{
class MeshProcessor
{
public:
    MeshProcessor(const PScene scene, Gfx::PGraphics graphics);
    virtual ~MeshProcessor();
protected:
    PScene scene;
    Gfx::PGraphics graphics;
    virtual void addMeshBatch(
        const MeshBatch& meshBatch, 
//        const PPrimitiveComponent primitiveComponent,
        const Gfx::PRenderPass renderPass,
        Gfx::PPipelineLayout pipelineLayout,
        Gfx::PDescriptorLayout primitiveLayout,
        Array<Gfx::PDescriptorSet>& descriptorSets,
        int32 staticMeshId = -1) = 0;
    void buildMeshDrawCommand(
        const MeshBatch& meshBatch, 
//        const PPrimitiveComponent primitiveComponent,
        const Gfx::PRenderPass renderPass,
        Gfx::PPipelineLayout pipelineLayout,
        Gfx::PRenderCommand drawCommand,
        const Array<Gfx::PDescriptorSet>& descriptors, 
        Gfx::PVertexShader vertexShader,
        Gfx::PControlShader controlShader,
        Gfx::PEvaluationShader evaluationShader,
        Gfx::PGeometryShader geometryShader,
        Gfx::PFragmentShader fragmentShader,
        bool positionOnly);
};
} // namespace Seele