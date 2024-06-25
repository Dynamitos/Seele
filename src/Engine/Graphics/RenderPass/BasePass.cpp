#include "BasePass.h"
#include "Actor/CameraActor.h"
#include "Component/Camera.h"
#include "Component/Mesh.h"
#include "Graphics/Command.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/Shader.h"
#include "Graphics/StaticMeshVertexData.h"
#include "Material/MaterialInstance.h"
#include "Math/Vector.h"
#include "RenderGraph.h"
#include "Window/Window.h"

using namespace Seele;

BasePass::BasePass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics, scene) {
    basePassLayout = graphics->createPipelineLayout("BasePassLayout");

    basePassLayout->addDescriptorLayout(viewParamsLayout);
    basePassLayout->addDescriptorLayout(scene->getLightEnvironment()->getDescriptorLayout());
    basePassLayout->addDescriptorLayout(Material::getDescriptorLayout());

    lightCullingLayout = graphics->createDescriptorLayout("pLightCullingData");
    // oLightIndexList
    lightCullingLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    // oLightGrid
    lightCullingLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    });
    lightCullingLayout->create();

    basePassLayout->addDescriptorLayout(lightCullingLayout);
    basePassLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT | Gfx::SE_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(VertexData::DrawCallOffsets),
    });

    if (graphics->supportMeshShading()) {
        graphics->getShaderCompiler()->registerRenderPass("BasePass", Gfx::PassConfig{
                                                                          .baseLayout = basePassLayout,
                                                                          .taskFile = "DrawListTask",
                                                                          .mainFile = "DrawListMesh",
                                                                          .fragmentFile = "BasePass",
                                                                          .hasFragmentShader = true,
                                                                          .useMeshShading = true,
                                                                          .hasTaskShader = true,
                                                                          .useMaterial = true,
                                                                          .useVisibility = false,
                                                                      });
    } else {
        graphics->getShaderCompiler()->registerRenderPass("BasePass", Gfx::PassConfig{
                                                                          .baseLayout = basePassLayout,
                                                                          .taskFile = "",
                                                                          .mainFile = "LegacyPass",
                                                                          .fragmentFile = "BasePass",
                                                                          .hasFragmentShader = true,
                                                                          .useMeshShading = false,
                                                                          .hasTaskShader = false,
                                                                          .useMaterial = true,
                                                                          .useVisibility = false,
                                                                      });
    }
}

BasePass::~BasePass() {}

void BasePass::beginFrame(const Component::Camera& cam) {
    RenderPass::beginFrame(cam);

    cameraPos = cam.getCameraPosition();
    cameraForward = cam.getCameraForward();

    lightCullingLayout->reset();
    opaqueCulling = lightCullingLayout->allocateDescriptorSet();
    transparentCulling = lightCullingLayout->allocateDescriptorSet();
}

void BasePass::render() {
    opaqueCulling->updateBuffer(0, oLightIndexList);
    opaqueCulling->updateTexture(1, oLightGrid);
    transparentCulling->updateBuffer(0, tLightIndexList);
    transparentCulling->updateTexture(1, tLightGrid);
    opaqueCulling->writeChanges();
    transparentCulling->writeChanges();

    query->beginQuery();
    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;

    Gfx::ShaderPermutation permutation = graphics->getShaderCompiler()->getTemplate("BasePass");
    permutation.setDepthCulling(true); // always use the culling info
    permutation.setPositionOnly(false);
    Array<VertexData::TransparentDraw> transparentData;
    for (VertexData* vertexData : VertexData::getList()) {
        transparentData.addAll(vertexData->getTransparentData());
        vertexData->getInstanceDataSet()->updateBuffer(6, cullingBuffer);
        vertexData->getInstanceDataSet()->writeChanges();
        permutation.setVertexData(vertexData->getTypeName());
        const auto& materials = vertexData->getMaterialData();
        for (const auto& materialData : materials) {
            // material not used for any active meshes, skip
            if (materialData.instances.size() == 0)
                continue;
            // Create Pipeline(Material, VertexData)
            // Descriptors:
            // ViewData => global, static
            // VertexData => per meshtype
            // SceneData => per material instance
            // LightEnv => provided by scene
            // Material => per material
            // LightCulling => calculated by pass
            permutation.setMaterial(materialData.material->getName());
            Gfx::PermutationId id(permutation);

            Gfx::ORenderCommand command = graphics->createRenderCommand("BaseRender");
            command->setViewport(viewport);

            const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(id);
            assert(collection != nullptr);

            bool twoSided = materialData.material->isTwoSided();

            if (graphics->supportMeshShading()) {
                Gfx::MeshPipelineCreateInfo pipelineInfo = {
                    .taskShader = collection->taskShader,
                    .meshShader = collection->meshShader,
                    .fragmentShader = collection->fragmentShader,
                    .renderPass = renderPass,
                    .pipelineLayout = collection->pipelineLayout,
                    .multisampleState =
                        {
                            .samples = viewport->getSamples(),
                        },
                    .rasterizationState =
                        {
                            .cullMode = Gfx::SeCullModeFlags(twoSided ? Gfx::SE_CULL_MODE_NONE : Gfx::SE_CULL_MODE_BACK_BIT),
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
                    .multisampleState =
                        {
                            .samples = viewport->getSamples(),
                        },
                    .rasterizationState =
                        {
                            .cullMode = Gfx::SeCullModeFlags(twoSided ? Gfx::SE_CULL_MODE_NONE : Gfx::SE_CULL_MODE_BACK_BIT),
                        },
                    .colorBlend =
                        {
                            .attachmentCount = 1,
                        },
                };
                Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                command->bindPipeline(pipeline);
            }
            command->bindDescriptor({viewParamsSet, vertexData->getVertexDataSet(), vertexData->getInstanceDataSet(),
                                     scene->getLightEnvironment()->getDescriptorSet(), Material::getDescriptorSet(), opaqueCulling});
            for (const auto& drawCall : materialData.instances) {
                command->pushConstants(Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT |
                                           Gfx::SE_SHADER_STAGE_FRAGMENT_BIT,
                                       0, sizeof(VertexData::DrawCallOffsets), &drawCall.offsets);
                if (graphics->supportMeshShading()) {
                    command->drawMesh(drawCall.instanceMeshData.size(), 1, 1);
                } else {
                    command->bindIndexBuffer(vertexData->getIndexBuffer());
                    for (const auto& meshData : drawCall.instanceMeshData) {
                        // all meshlets of a mesh share the same indices offset
                        command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex,
                                             vertexData->getIndicesOffset(meshData.meshletOffset), 0);
                    }
                }
            }
            commands.add(std::move(command));
        }
    }
    graphics->executeCommands(std::move(commands));

    Map<float, VertexData::TransparentDraw> sortedDraws;
    for (const auto& t : transparentData) {
        Vector toCenter = Vector(t.worldPosition) - cameraPos;
        float dist = glm::length(toCenter) * glm::dot(glm::normalize(toCenter), cameraForward);
        sortedDraws[dist] = t;
    }
    Gfx::ORenderCommand command = graphics->createRenderCommand("TransparentDraw");
    command->setViewport(viewport);
    for (const auto& [_, t] : sortedDraws) {
        permutation.setVertexData(t.vertexData->getTypeName());
        permutation.setMaterial(t.matInst->getBaseMaterial()->getName());
        Gfx::PermutationId id(permutation);

        const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(id);
        assert(collection != nullptr);

        bool twoSided = t.matInst->getBaseMaterial()->isTwoSided();

        if (graphics->supportMeshShading()) {
            Gfx::MeshPipelineCreateInfo pipelineInfo = {
                .taskShader = collection->taskShader,
                .meshShader = collection->meshShader,
                .fragmentShader = collection->fragmentShader,
                .renderPass = renderPass,
                .pipelineLayout = collection->pipelineLayout,
                .multisampleState =
                    {
                        .samples = viewport->getSamples(),
                    },
                .rasterizationState =
                    {
                        .cullMode = Gfx::SeCullModeFlags(twoSided ? Gfx::SE_CULL_MODE_NONE : Gfx::SE_CULL_MODE_BACK_BIT),
                    },
                .depthStencilState =
                    {
                        .depthCompareOp = Gfx::SE_COMPARE_OP_GREATER_OR_EQUAL,
                    },
                .colorBlend =
                    {
                        .attachmentCount = 1,
                        .blendAttachments =
                            {
                                Gfx::ColorBlendState::BlendAttachment{
                                    .blendEnable = true,
                                },
                            },
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
                .multisampleState =
                    {
                        .samples = viewport->getSamples(),
                    },
                .rasterizationState =
                    {
                        .cullMode = Gfx::SeCullModeFlags(twoSided ? Gfx::SE_CULL_MODE_NONE : Gfx::SE_CULL_MODE_BACK_BIT),
                    },
                .depthStencilState =
                    {
                        .depthCompareOp = Gfx::SE_COMPARE_OP_GREATER_OR_EQUAL,
                    },
                .colorBlend =
                    {
                        .attachmentCount = 1,
                        .blendAttachments =
                            {
                                Gfx::ColorBlendState::BlendAttachment{
                                    .blendEnable = true,
                                },
                            },
                    },
            };
            Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
            command->bindPipeline(pipeline);
        }
        command->bindDescriptor({viewParamsSet, t.vertexData->getVertexDataSet(), t.vertexData->getInstanceDataSet(),
                                 scene->getLightEnvironment()->getDescriptorSet(), Material::getDescriptorSet(), transparentCulling});
        command->pushConstants(Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT | Gfx::SE_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(VertexData::DrawCallOffsets), &t.offsets);
        if (graphics->supportMeshShading()) {
            command->drawMesh(1, 1, 1);
        } else {
            // command->bindIndexBuffer(t.vertexData->getIndexBuffer());
            // for (const auto& meshData : drawCall.instanceMeshData) {
            //     // all meshlets of a mesh share the same indices offset
            //     command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex, vertexData->getIndicesOffset(meshData.meshletOffset),
            //     0);
            // }
        }
    }

    graphics->endRenderPass();
    query->endQuery();
}

void BasePass::endFrame() {}

void BasePass::publishOutputs() {
    TextureCreateInfo depthBufferInfo = {
        .format = Gfx::SE_FORMAT_D32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .samples = viewport->getSamples(),
        .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };
    basePassDepth = graphics->createTexture2D(depthBufferInfo);

    colorAttachment = Gfx::RenderTargetAttachment(viewport, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                  Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    resources->registerRenderPassOutput("BASEPASS_COLOR", colorAttachment);
    depthAttachment =
        Gfx::RenderTargetAttachment(basePassDepth, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    resources->registerRenderPassOutput("BASEPASS_DEPTH", depthAttachment);

    query = graphics->createPipelineStatisticsQuery();
    resources->registerQueryOutput("BASEPASS_QUERY", query);
}

void BasePass::createRenderPass() {
    cullingBuffer = resources->requestBuffer("CULLINGBUFFER");

    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{
        .colorAttachments = {colorAttachment},
        .depthAttachment = depthAttachment,
    };
    Array<Gfx::SubPassDependency> dependency = {
        {
            .srcSubpass = ~0U,
            .dstSubpass = 0,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
        {
            .srcSubpass = 0,
            .dstSubpass = ~0U,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
    };
    renderPass = graphics->createRenderPass(std::move(layout), std::move(dependency), viewport);
    oLightIndexList = resources->requestBuffer("LIGHTCULLING_OLIGHTLIST");
    tLightIndexList = resources->requestBuffer("LIGHTCULLING_TLIGHTLIST");
    oLightGrid = resources->requestTexture("LIGHTCULLING_OLIGHTGRID");
    tLightGrid = resources->requestTexture("LIGHTCULLING_TLIGHTGRID");
}
