#include "ShadowPass.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"

using namespace Seele;

ShadowPass::ShadowPass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics), scene(scene) {
    shadowLayout = graphics->createPipelineLayout("ShadowLayout");
    shadowLayout->addDescriptorLayout(viewParamsLayout);
    shadowLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(VertexData::DrawCallOffsets),
    });
    if (graphics->supportMeshShading()) {
        graphics->getShaderCompiler()->registerRenderPass("ShadowPass", Gfx::PassConfig{
                                                                            .baseLayout = shadowLayout,
                                                                            .taskFile = "DrawListTask",
                                                                            .mainFile = "DrawListMesh",
                                                                            .hasFragmentShader = false,
                                                                            .useMeshShading = true,
                                                                            .hasTaskShader = true,
                                                                            .useMaterial = false,
                                                                            .useVisibility = false,
                                                                        });
    } else {
        graphics->getShaderCompiler()->registerRenderPass("ShadowPass", Gfx::PassConfig{
                                                                            .baseLayout = shadowLayout,
                                                                            .taskFile = "",
                                                                            .mainFile = "LegacyPass",
                                                                            .hasFragmentShader = false,
                                                                            .useMeshShading = false,
                                                                            .hasTaskShader = false,
                                                                            .useMaterial = false,
                                                                            .useVisibility = false,
                                                                        });
    }
}

ShadowPass::~ShadowPass() {}

void ShadowPass::beginFrame(const Component::Camera&, const Component::Transform&) {}

void ShadowPass::render() {
    graphics->beginDebugRegion("ShadowPass");
    Gfx::ShaderPermutation permutation = graphics->getShaderCompiler()->getTemplate("ShadowPass");
    permutation.setDepthCulling(true);
    permutation.setPositionOnly(true);
    const auto& shadowMaps = scene->getLightEnvironment()->getShadowMaps();
    for (uint32 shadowIndex = 0; shadowIndex < scene->getLightEnvironment()->getNumDirectionalLights(); ++shadowIndex) {
        Array<Gfx::ORenderCommand> commands;
        renderPass = graphics->createRenderPass(
            Gfx::RenderTargetLayout{
                .depthAttachment = Gfx::RenderTargetAttachment(shadowMaps[shadowIndex], Gfx::SE_IMAGE_LAYOUT_UNDEFINED,
                                                               Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                               Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE),

            },
            {
                Gfx::SubPassDependency{
                    .srcSubpass = ~0U,
                    .dstSubpass = 0,
                    .srcStage = Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    .dstStage = Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    .srcAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    .dstAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                },
                Gfx::SubPassDependency{
                    .srcSubpass = 0,
                    .dstSubpass = ~0U,
                    .srcStage = Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    .dstStage = Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    .srcAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    .dstAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                },
            },
            {
                .size = {shadowMaps[shadowIndex]->getWidth(), shadowMaps[shadowIndex]->getHeight()},
                .offset = {0, 0},
            },
            "Shadow");
        auto light = scene->getLightEnvironment()->getDirectionalLight(shadowIndex);
        Component::Camera lightCamera = Component::Camera{
            .nearPlane = -100.f,
            .farPlane = 100.0f,
        };
        Gfx::PDescriptorSet viewParamsSet = createViewParamsSet(lightCamera, scene->getLightEnvironment()->getDirectionalTransform(shadowIndex));
        graphics->beginRenderPass(renderPass);
        for (VertexData* vertexData : VertexData::getList()) {
            permutation.setVertexData(vertexData->getTypeName());
            Gfx::PermutationId id(permutation);

            Gfx::ORenderCommand command = graphics->createRenderCommand("ShadowRender");
            command->setViewport(shadowViewport);

            const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(id);
            constexpr float depthBiasConstant = -1.25f;
            constexpr float depthBiasSlope = -1.75f;
            if (graphics->supportMeshShading()) {
                Gfx::MeshPipelineCreateInfo pipelineInfo = {
                    .taskShader = collection->taskShader,
                    .meshShader = collection->meshShader,
                    .fragmentShader = collection->fragmentShader,
                    .renderPass = renderPass,
                    .pipelineLayout = collection->pipelineLayout,
                    .rasterizationState =
                        {
                            .cullMode = Gfx::SE_CULL_MODE_NONE,
                            .depthBiasEnable = true,
                            .depthBiasConstantFactor = depthBiasConstant,
                            .depthBiasSlopeFactor = depthBiasSlope,
                        },
                };
                Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                command->bindPipeline(pipeline);
            } else {
                Gfx::LegacyPipelineCreateInfo pipelineInfo = {
                    .vertexShader = collection->vertexShader,
                    .fragmentShader = collection->fragmentShader,
                    .renderPass = renderPass,
                    .pipelineLayout = collection->pipelineLayout,
                    .rasterizationState =
                        {
                            .cullMode = Gfx::SE_CULL_MODE_NONE,
                            .depthBiasEnable = true,
                            .depthBiasConstantFactor = depthBiasConstant,
                            .depthBiasSlopeFactor = depthBiasSlope,
                        },
                };
                Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                command->bindPipeline(pipeline);
            }
            command->bindDescriptor({viewParamsSet, vertexData->getVertexDataSet(), vertexData->getInstanceDataSet()});
            VertexData::DrawCallOffsets offsets = {
                .instanceOffset = 0,
            };
            command->pushConstants(Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT, 0,
                                   sizeof(VertexData::DrawCallOffsets), &offsets);
            if (graphics->supportMeshShading()) {
                command->drawMesh((uint32)vertexData->getNumInstances(), 1, 1);
            } else {
                const auto& materials = vertexData->getMaterialData();
                for (const auto& materialData : materials) {
                    // material not used for any active meshes, skip
                    if (materialData.instances.size() == 0)
                        continue;
                    for (const auto& drawCall : materialData.instances) {
                        command->bindIndexBuffer(vertexData->getIndexBuffer());
                        uint32 inst = drawCall.offsets.instanceOffset;
                        for (const auto& meshData : drawCall.instanceMeshData) {
                            // all meshlets of a mesh share the same indices offset
                            command->drawIndexed(meshData.indicesRange.size, 1, meshData.indicesRange.offset,
                                                 vertexData->getIndicesOffset(meshData.meshletRange.offset), inst++);
                        }
                    }
                }
            }
            commands.add(std::move(command));
        }
        graphics->executeCommands(std::move(commands));
        graphics->endRenderPass();
        graphics->waitDeviceIdle();
    }
    graphics->endDebugRegion();
}

void ShadowPass::endFrame() {}

void ShadowPass::publishOutputs() {
    shadowViewport = graphics->createViewport(nullptr, ViewportCreateInfo{.dimensions =
                                                                              {
                                                                                  .size = {2048, 2048},
                                                                                  .offset = {0, 0},
                                                                              },
                                                                          .fieldOfView = 0,
                                                                          .left = -100,
                                                                          .right = 100,
                                                                          .top = 100,
                                                                          .bottom = -100});
    viewport = shadowViewport;
}

void ShadowPass::createRenderPass() { cullingBuffer = resources->requestBuffer("CULLINGBUFFER"); }
