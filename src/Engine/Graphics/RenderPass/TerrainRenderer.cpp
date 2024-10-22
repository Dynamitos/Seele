#include "TerrainRenderer.h"
#include "Asset/AssetRegistry.h"
#include "Component/TerrainTile.h"
#include "Graphics/Graphics.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Shader.h"

using namespace Seele;

TerrainRenderer::TerrainRenderer(Gfx::PGraphics graphics, PScene scene, Gfx::PDescriptorLayout viewParamsLayout,
                                 Gfx::PDescriptorSet viewParamsSet)
    : graphics(graphics), scene(scene) {
    Gfx::OPipelineLayout test = graphics->createPipelineLayout();
    graphics->beginShaderCompilation(ShaderCompilationInfo{
        .modules = {"CompileTest"},
        .entryPoints = {{"GetHeap", "CompileTest"}},
        .rootSignature = test,
    });
    graphics->createComputeShader({0});
    meshUpdater.init(graphics, viewParamsLayout);
    lebCache.init(graphics, 5);
    CBT<18> cbt;
    CPUMesh cpuMesh = generateCPUMesh(cbt.numElements());
    plainMesh.gpuCBT.lastLevelSize = cbt.lastLevelSize();
    plainMesh.gpuCBT.bufferCount = 2;
    for (uint32 i = 0; i < 2; ++i) {
        uint32 bufferSize = cbt.bufferSize(i);
        uint32 elementSize = cbt.elementSize(i);
        plainMesh.gpuCBT.bufferArray[i] = graphics->createShaderBuffer(ShaderBufferCreateInfo{
            .sourceData =
                {
                    .size = bufferSize,
                    .data = (uint8*)cbt.rawBuffer(i),
                },
            .name = fmt::format("GPUCBT{0}", i).c_str(),
        });
        plainMesh.gpuCBT.bufferArray[i]->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                                         Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                                         Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    plainMesh.totalNumElements = cpuMesh.totalNumElements;
    plainMesh.numBaseVertices = (uint32)cpuMesh.basePoints.size();
    plainMesh.baseDepth = cpuMesh.minimalDepth;

    plainMesh.heapIDBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint64) * cpuMesh.totalNumElements,
                .data = (uint8*)cpuMesh.heapIDArray.data(),
            },
        .name = "HeapID",
    });
    plainMesh.heapIDBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                            Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                            Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    plainMesh.currentNeighborsBufferIdx = 0;
    plainMesh.neighborsBuffers[0] = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(UVector4) * cpuMesh.totalNumElements,
                .data = (uint8*)cpuMesh.neighborsArray.data(),
            },
        .name = "Neighbours0",
    });
    plainMesh.neighborsBuffers[0]->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                                   Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                                   Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    plainMesh.neighborsBuffers[1] = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(UVector4) * cpuMesh.totalNumElements,
            },
        .name = "Neighbours1",
    });
    plainMesh.neighborsBuffers[1]->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                                   Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                                   Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    plainMesh.updateBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(BisectorData) * cpuMesh.totalNumElements,
            },
        .name = "UpdateBuffer",
    });
    plainMesh.classificationBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * (2 + cpuMesh.totalNumElements * 2),
            },
        .name = "Classification",
    });
    plainMesh.simplificationBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * (1 + cpuMesh.totalNumElements),
            },
        .name = "Simplification",
    });
    plainMesh.allocateBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * (1 + cpuMesh.totalNumElements),
            },
        .name = "Allocation",
    });
    plainMesh.propagateBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * (2 + cpuMesh.totalNumElements),
            },
        .name = "Propagate",
    });

    plainMesh.indirectDrawBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * (4 * 2 + 2),
            },
        .usage = Gfx::SE_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        .name = "IndirectDraw",
    });
    plainMesh.indirectDispatchBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * 3 * 3,
            },
        .usage = Gfx::SE_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        .name = "IndirectDispatch",
    });
    plainMesh.indexedBisectorBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * cpuMesh.totalNumElements,
            },
        .name = "IndexedBisector",
    });
    plainMesh.visibleIndexedBisectorBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * cpuMesh.totalNumElements,
            },
        .name = "VisibleIndexedBisector",
    });
    plainMesh.modifiedIndexedBisectorBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * cpuMesh.totalNumElements,
            },
        .name = "ModifiedIndexedBisector",
    });

    plainMesh.lebVertexBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(Vector4) * plainMesh.totalNumElements * 4,
            },
        .name = "LebVertexBuffer",
    });
    plainMesh.lebVertexBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                               Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    plainMesh.currentVertexBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(Vector4) * plainMesh.totalNumElements * 4,
            },
        .name = "CurrentVertexBuffer",
    });
    plainMesh.currentDisplacementBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(Vector4) * plainMesh.totalNumElements * 3,
            },
        .name = "CurrentDisplacementBuffer",
    });
    uint32 numBaseVertex = (uint32)cpuMesh.basePoints.size();
    baseMesh.numVertices = numBaseVertex;
    baseMesh.numElements = numBaseVertex / 3;
    uint32 baseVertexBufferSize = sizeof(Vector4) * numBaseVertex;
    baseMesh.vertexBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = baseVertexBufferSize,
                .data = (uint8*)cpuMesh.basePoints.data(),
            },
        .name = "VertexBuffer",
    });
    baseMesh.vertexBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                           Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                           Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    uint32 indexBufferSize = cpuMesh.totalNumElements * sizeof(UVector);
    Array<uint32> indices;
    for (uint32 i = 0; i < cpuMesh.totalNumElements * 3; ++i)
        indices.add(i);
    baseMesh.indexBuffer = graphics->createIndexBuffer(IndexBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * indices.size(),
                .data = (uint8*)indices.data(),
            },
        .indexType = Gfx::SE_INDEX_TYPE_UINT32,
        .name = "IndexBuffer",
    });
    baseMesh.indexBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                          Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                          Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    GeometryCB geometryCB = {
        .totalNumElements = plainMesh.totalNumElements,
        .baseDepth = plainMesh.baseDepth,
        .totalNumVertices = plainMesh.totalNumElements * 3,
    };
    geometryBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(GeometryCB),
                .data = (uint8*)&geometryCB,
            },
        .name = "GeometryCB",
    });
    geometryBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_UNIFORM_READ_BIT,
                                    Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    UpdateCB updateCB = {
        .triangleSize = 60.0f,
        .maxSubdivisionDepth = 63,
        .fov = 1.22173f,
        .farPlaneDistance = 1000.0f,
    };
    updateBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(UpdateCB),
                .data = (uint8*)&updateCB,
            },
        .name = "UpdateCB",
    });
    updateBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_UNIFORM_READ_BIT,
                                  Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    graphics->beginShaderCompilation(ShaderCompilationInfo{
        .modules = {"TerrainPass"},
        .entryPoints =
            {
                {"vert", "TerrainPass"},
                {"frag", "TerrainPass"},
                {"EvaluateDeformation", "TerrainPass"},
            },
        .rootSignature = meshUpdater.pipelineLayout,
    });
    vert = graphics->createVertexShader({0});
    frag = graphics->createFragmentShader({1});
    deformCS = graphics->createComputeShader({2});
    deform = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = deformCS,
        .pipelineLayout = meshUpdater.pipelineLayout,
    });
    meshUpdater.resetBuffers(plainMesh);
    meshUpdater.prepareIndirection(plainMesh, geometryBuffer);
    meshUpdater.evaluateLeb(baseMesh, plainMesh, viewParamsSet, geometryBuffer, updateBuffer, lebCache.getLebMatrixBuffer(), true, true);
    applyDeformation(viewParamsSet);
}

TerrainRenderer::~TerrainRenderer() {}

static bool first = true;

void TerrainRenderer::beginFrame(Gfx::PDescriptorSet viewParamsSet, const Component::Camera& cam) {
    meshUpdater.update(plainMesh, viewParamsSet, geometryBuffer, updateBuffer);
    meshUpdater.evaluateLeb(baseMesh, plainMesh, viewParamsSet, geometryBuffer, updateBuffer, lebCache.getLebMatrixBuffer(), false, false);
    applyDeformation(viewParamsSet);
}

Gfx::ORenderCommand TerrainRenderer::render(Gfx::PDescriptorSet viewParamsSet) {
    Gfx::PDescriptorSet set = meshUpdater.layout->allocateDescriptorSet();
    set->updateBuffer(CURRENT_VERTEX_BUFFER, 0, plainMesh.currentVertexBuffer);
    set->updateBuffer(INDEXED_BISECTOR_BUFFER, 0, plainMesh.indexedBisectorBuffer);
    set->writeChanges();
    Gfx::ORenderCommand command = graphics->createRenderCommand("TerrainRender");
    command->setViewport(viewport);
    command->bindPipeline(pipeline);
    command->bindDescriptor({viewParamsSet, set});
    command->drawIndirect(plainMesh.indirectDrawBuffer, 0, 1, 0);
    return command;
}

void TerrainRenderer::setViewport(Gfx::PViewport _viewport, Gfx::PRenderPass renderPass) {
    viewport = _viewport;
    inp = graphics->createVertexInput({});
    Gfx::LegacyPipelineCreateInfo pipelineInfo = {
        .vertexInput = inp,
        .vertexShader = vert,
        .fragmentShader = frag,
        .renderPass = renderPass,
        .pipelineLayout = meshUpdater.pipelineLayout,
        .multisampleState =
            {
                .samples = viewport->getSamples(),
            },
        .rasterizationState =
            {
                .polygonMode = Gfx::SE_POLYGON_MODE_LINE,
                .cullMode = Gfx::SE_CULL_MODE_BACK_BIT,
            },
        .colorBlend =
            {
                .attachmentCount = 1,
                .blendAttachments =
                    {
                        Gfx::ColorBlendState::BlendAttachment{
                            .blendEnable = false,
                        },
                    },
            },

    };
    pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
}

void TerrainRenderer::applyDeformation(Gfx::PDescriptorSet viewParamsSet) {
    graphics->beginDebugRegion("ApplyDeformation");
    Gfx::PDescriptorSet set = meshUpdater.layout->allocateDescriptorSet();
    set->updateBuffer(GEOMETRY_CB, 0, geometryBuffer);
    set->updateBuffer(INDIRECT_DRAW_BUFFER, 0, plainMesh.indirectDrawBuffer);
    set->updateBuffer(INDEXED_BISECTOR_BUFFER, 0, plainMesh.indexedBisectorBuffer);
    set->updateBuffer(LEB_POSITION_BUFFER, 0, plainMesh.lebVertexBuffer);
    set->updateBuffer(CURRENT_VERTEX_BUFFER, 0, plainMesh.currentVertexBuffer);
    set->writeChanges();
    Gfx::OComputeCommand command = graphics->createComputeCommand("Deform");
    command->bindPipeline(deform);
    command->bindDescriptor({viewParamsSet, set});
    command->dispatchIndirect(plainMesh.indirectDispatchBuffer, 3 * sizeof(uint32));
    graphics->executeCommands(std::move(command));
    plainMesh.currentVertexBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                   Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    graphics->endDebugRegion();
}
