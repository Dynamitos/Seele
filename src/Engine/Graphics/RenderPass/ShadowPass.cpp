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

void ShadowPass::beginFrame(const Component::Camera& cam, const Component::Transform& transform) {}

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
        graphics->beginRenderPass(renderPass);
        auto light = scene->getLightEnvironment()->getDirectionalLight(shadowIndex);
        Vector lightDirection = Vector(light.direction);
        Vector lightPosition = -light.direction * 100.0f;
        float dot = glm::dot(lightDirection, Math::Transform::FORWARD);
        Quaternion rotation = Quaternion(1, 0, 0, 0);
        // Handle the edge cases first
        if (glm::epsilonEqual(dot, 1.0f, 0.00001f)) {
            // Vectors are the same, no rotation
        } else if (glm::epsilonEqual(dot, -1.0f, 0.00001f)) {
            // Vectors are opposite need 180-degree rotation around any perpendicular axis
            rotation = glm::angleAxis(glm::pi<float>(), Vector(0, 1, 0));
        } else {
            // Normal case
            Vector axis = glm::normalize(glm::cross(lightDirection, Math::Transform::FORWARD));
            float angle = std::acos(dot); // angle between vectors
            rotation = glm::angleAxis(angle, axis);
        }
        Component::Transform lightTransform = Component::Transform{
            .transform = Math::Transform(lightPosition, rotation),
        };

        Component::Camera lightCamera = Component::Camera{
            .nearPlane = -1000.f,
            .farPlane = 1000.0f,
        };
        Gfx::PDescriptorSet viewParamsSet = createViewParamsSet(lightCamera, lightTransform);
        for (VertexData* vertexData : VertexData::getList()) {
            permutation.setVertexData(vertexData->getTypeName());
            Gfx::PermutationId id(permutation);

            Gfx::ORenderCommand command = graphics->createRenderCommand("ShadowRender");
            command->setViewport(shadowViewport);

            const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(id);
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
                        },
                    .colorBlend =
                        {
                            .attachmentCount = 1,
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
                        },
                    .colorBlend =
                        {
                            .attachmentCount = 1,
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
                                                                          .left = -10,
                                                                          .right = 10,
                                                                          .top = -10,
                                                                          .bottom = 10});
    viewport = shadowViewport;
}

void ShadowPass::createRenderPass() { cullingBuffer = resources->requestBuffer("CULLINGBUFFER"); }
