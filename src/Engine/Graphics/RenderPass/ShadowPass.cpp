#include "ShadowPass.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/Shader.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/matrix.hpp>

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

void ShadowPass::beginFrame(const Component::Camera& camera, const Component::Transform& transform) {
    float cascadeSplits[NUM_CASCADES];

    float nearClip = camera.nearPlane;
    float farClip = camera.farPlane;
    float clipRange = farClip - nearClip;

    float minZ = nearClip;
    float maxZ = nearClip + clipRange;

    float range = maxZ - minZ;
    float ratio = maxZ / minZ;
    constexpr float cascadeSplitLambda = 0.95f;
    for (uint32 i = 0; i < NUM_CASCADES; ++i) {
        float p = (i + 1) / static_cast<float>(NUM_CASCADES);
        float log = minZ * std::pow(ratio, p);
        float uniform = minZ + range * p;
        float d = cascadeSplitLambda * (log - uniform) + uniform;
        cascadeSplits[i] = (d - nearClip) / clipRange;
        cascades[i].viewParams.clear();
    }

    // call this to update view params member, ignore descriptor set
    updateViewParameters(camera, transform);
    Array<Vector> frustumCorners = {
        Vector(-1.0f, 1.0f, 0.0f), Vector(1.0f, 1.0f, 0.0f), Vector(1.0f, -1.0f, 0.0f), Vector(-1.0f, -1.0f, 0.0f),
        Vector(-1.0f, 1.0f, 1.0f), Vector(1.0f, 1.0f, 1.0f), Vector(1.0f, -1.0f, 1.0f), Vector(-1.0f, -1.0f, 1.0f),
    };

    for (auto& c : frustumCorners) {
        Vector4 invCorner = viewParams.inverseViewProjectionMatrix * Vector4(c, 1);
        c = invCorner / invCorner.w;
    }
    for (uint32 s = 0; s < scene->getLightEnvironment()->getNumDirectionalLights(); ++s) {
        float lastSplitDist = 0.0;
        for (uint32 i = 0; i < NUM_CASCADES; ++i) {
            float splitDist = cascadeSplits[i];

            Array<Vector> cascadeCorners(8);
            for (uint32 j = 0; j < 4; j++) {
                Vector dist = frustumCorners[j + 4] - frustumCorners[j];
                cascadeCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
                cascadeCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
            }

            Vector frustumCenter = Vector(0);
            for (uint32 j = 0; j < 8; j++) {
                frustumCenter += cascadeCorners[j];
            }
            frustumCenter /= 8.0f;

            float radius = 0.0f;
            for (uint j = 0; j < 8; j++) {
                float distance = glm::length(cascadeCorners[j] - frustumCenter);
                radius = glm::max(radius, distance);
            }
            radius = std::ceil(radius * 16.0f) / 16.0f;

            Vector maxExtents = Vector(radius);
            Vector minExtents = -maxExtents;
            Vector lightDir = glm::normalize(scene->getLightEnvironment()->getDirectionalLight(s).direction);
            Vector cameraPos = frustumCenter - lightDir * -minExtents.z;
            Matrix4 viewMatrix = glm::lookAt(cameraPos, frustumCenter, Vector(0, 1, 0));
            Matrix4 projectionMatrix =
                glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);
            Matrix4 viewProjectionMatrix = projectionMatrix * viewMatrix;
            viewParams.viewMatrix = viewMatrix;
            viewParams.inverseViewMatrix = glm::inverse(viewMatrix);
            viewParams.projectionMatrix = projectionMatrix;
            viewParams.inverseProjection = glm::inverse(projectionMatrix);
            viewParams.viewProjectionMatrix = viewProjectionMatrix;
            viewParams.inverseViewProjectionMatrix = glm::inverse(viewProjectionMatrix);
            viewParams.cameraPosition_WS = Vector4(cameraPos, 1);
            viewParams.cameraForward_WS = Vector4(frustumCenter - cameraPos, 0); 
            viewParams.screenDimensions = Vector2(maxExtents.x - minExtents.x, maxExtents.y - minExtents.y);
            viewParams.invScreenDimensions = 1.0f / viewParams.screenDimensions;
            cascades[i].viewParams.add(createViewParamsSet());

            lastSplitDist = cascadeSplits[i];
        }
    }
}

void ShadowPass::render() {
    graphics->beginDebugRegion("ShadowPass");
    Gfx::ShaderPermutation permutation = graphics->getShaderCompiler()->getTemplate("ShadowPass");
    permutation.setDepthCulling(true);
    permutation.setPositionOnly(true);
    for (uint32 c = 0; c < NUM_CASCADES; ++c) {
        graphics->beginDebugRegion("Cascade");
        for (uint32 shadowIndex = 0; shadowIndex < cascades[c].shadowMaps->getNumLayers(); ++shadowIndex) {
            Array<Gfx::ORenderCommand> commands;
            renderPass = graphics->createRenderPass(
                Gfx::RenderTargetLayout{
                    .depthAttachment = Gfx::RenderTargetAttachment(cascades[c].views[shadowIndex], Gfx::SE_IMAGE_LAYOUT_UNDEFINED,
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
                    .size = {cascades[c].shadowMaps->getWidth(), cascades[c].shadowMaps->getHeight()},
                    .offset = {0, 0},
                },
                "Shadow");
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
                                .cullMode = Gfx::SE_CULL_MODE_FRONT_BIT,
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
                                .cullMode = Gfx::SE_CULL_MODE_FRONT_BIT,
                                .depthBiasEnable = true,
                                .depthBiasConstantFactor = depthBiasConstant,
                                .depthBiasSlopeFactor = depthBiasSlope,
                            },
                    };
                    Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                    command->bindPipeline(pipeline);
                }
                command->bindDescriptor(
                    {cascades[c].viewParams[shadowIndex], vertexData->getVertexDataSet(), vertexData->getInstanceDataSet()});
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
    graphics->endDebugRegion();
}

void ShadowPass::endFrame() {}

void ShadowPass::publishOutputs() {
    cascadeSplitsBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{.sourceData =
                                                                              {
                                                                                  .size = sizeof(float) * NUM_CASCADES,
                                                                                  .data = nullptr,
                                                                              },
                                                                          .name = "CascadeSplits"});
    shadowViewport = graphics->createViewport(nullptr, ViewportCreateInfo{.dimensions =
                                                                              {
                                                                                  .size = {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE},
                                                                                  .offset = {0, 0},
                                                                              },
                                                                          .fieldOfView = 0,
                                                                          .left = -100,
                                                                          .right = 100,
                                                                          .top = 100,
                                                                          .bottom = -100});
    uint32 cascadeDim = SHADOW_MAP_SIZE;
    for (uint32 s = 0; s < NUM_CASCADES; ++s) {
        cascades[s].shadowMaps = graphics->createTexture2DArray(TextureCreateInfo{
            .format = Gfx::SE_FORMAT_D32_SFLOAT,
            .width = cascadeDim,
            .height = cascadeDim,
            .elements = 1, // TODO:
            .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | Gfx::SE_IMAGE_USAGE_SAMPLED_BIT,
            .name = "ShadowMapCascade",
        });
        cascades[s].lightSpaceBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
            .sourceData = {
                .size = sizeof(Matrix4),
                .data = nullptr,
            },
            .name = "LightSpaceBuffer"
        });
        cascades[s].views.clear();
        for (uint32 j = 0; j < cascades[s].shadowMaps->getNumLayers(); ++j) {
            cascades[s].views.add(cascades[s].shadowMaps->createTextureView(0, 1, j, 1));
        }
        cascadeDim /= 2;
        resources->registerTextureOutput(fmt::format("SHADOWMAP_TEXTURE{0}", s), Gfx::PTexture2DArray(cascades[s].shadowMaps));
        resources->registerBufferOutput(fmt::format("SHADOWMAP_LIGHTSPACE{0}", s), cascades[s].lightSpaceBuffer);
    }
    cascadeSplitsBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData = {
            .size = sizeof(float) * NUM_CASCADES,
            .data = nullptr,
        },
        .name = "CASCADE_SPLITS"
    });
    resources->registerUniformOutput("SHADOWMAP_CASCADESPLITS", cascadeSplitsBuffer);
    viewport = shadowViewport;
}

void ShadowPass::createRenderPass() { cullingBuffer = resources->requestBuffer("CULLINGBUFFER"); }
