#include "BasePass.h"
#include "Actor/CameraActor.h"
#include "Asset/AssetRegistry.h"
#include "Component/Camera.h"
#include "Component/Mesh.h"
#include "Graphics/Command.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Shader.h"
#include "Graphics/StaticMeshVertexData.h"
#include "Material/MaterialInstance.h"
#include "Math/Vector.h"
#include "RenderGraph.h"
#include "Window/Window.h"

using namespace Seele;

Array<DebugVertex> gDebugVertices;

void Seele::addDebugVertex(DebugVertex vert) { gDebugVertices.add(vert); }

void Seele::addDebugVertices(Array<DebugVertex> verts) { gDebugVertices.addAll(verts); }

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
    skybox = Seele::Component::Skybox{
        .day = AssetRegistry::findTexture("", "skyboxsun5deg_tn")->getTexture().cast<Gfx::TextureCube>(),
        .night = AssetRegistry::findTexture("", "skyboxsun5deg_tn")->getTexture().cast<Gfx::TextureCube>(),
        .fogColor = Vector(0.1, 0.1, 0.8),
        .blendFactor = 0,
    };
}

BasePass::~BasePass() {}

void BasePass::beginFrame(const Component::Camera& cam) {
    RenderPass::beginFrame(cam);

    cameraPos = cam.getCameraPosition();
    cameraForward = cam.getCameraForward();

    lightCullingLayout->reset();
    opaqueCulling = lightCullingLayout->allocateDescriptorSet();
    transparentCulling = lightCullingLayout->allocateDescriptorSet();

    // Debug vertices
    VertexBufferCreateInfo vertexBufferInfo = {
        .sourceData =
            {
                .size = sizeof(DebugVertex) * gDebugVertices.size(),
                .data = (uint8*)gDebugVertices.data(),
            },
        .vertexSize = sizeof(DebugVertex),
        .numVertices = (uint32)gDebugVertices.size(),
        .name = "DebugVertices",
    };
    debugVertices = graphics->createVertexBuffer(vertexBufferInfo);
    debugVertices->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                   Gfx::SE_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, Gfx::SE_PIPELINE_STAGE_VERTEX_INPUT_BIT);

    // Skybox
    skyboxDataLayout->reset();
    textureLayout->reset();
    skyboxData.transformMatrix = glm::rotate(skyboxData.transformMatrix, (float)(Gfx::getCurrentFrameDelta()), Vector(0, 1, 0));
    skyboxBuffer->rotateBuffer(sizeof(SkyboxData));
    skyboxBuffer->updateContents(0, sizeof(SkyboxData), &skyboxData);
    skyboxBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_UNIFORM_READ_BIT,
                                  Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    skyboxDataSet = skyboxDataLayout->allocateDescriptorSet();
    skyboxDataSet->updateBuffer(0, skyboxBuffer);
    skyboxDataSet->writeChanges();
    textureSet = textureLayout->allocateDescriptorSet();
    textureSet->updateTexture(0, skybox.day);
    textureSet->updateTexture(1, skybox.night);
    textureSet->updateSampler(2, skyboxSampler);
    textureSet->writeChanges();
}

void BasePass::render() {
    opaqueCulling->updateBuffer(0, oLightIndexList);
    opaqueCulling->updateTexture(1, oLightGrid);
    transparentCulling->updateBuffer(0, tLightIndexList);
    transparentCulling->updateTexture(1, tLightGrid);
    opaqueCulling->writeChanges();
    transparentCulling->writeChanges();

    query->beginQuery();
    timestamps->write(Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "BASEPASS");
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

    Gfx::ORenderCommand skyboxCommand = graphics->createRenderCommand("SkyboxRender");
    skyboxCommand->setViewport(viewport);
    skyboxCommand->bindPipeline(pipeline);
    skyboxCommand->bindDescriptor({viewParamsSet, skyboxDataSet, textureSet});
    skyboxCommand->draw(36, 1, 0, 0);
    commands.add(std::move(skyboxCommand));

    // Transparent rendering
    {
        permutation.setDepthCulling(false); // ignore visibility infos for transparency
        Map<float, VertexData::TransparentDraw> sortedDraws;
        for (const auto& t : transparentData) {
            Vector toCenter = Vector(t.worldPosition) - cameraPos;
            float dist = glm::length(toCenter) * glm::dot(glm::normalize(toCenter), cameraForward);
            sortedDraws[dist] = t;
        }
        Gfx::ORenderCommand transparentCommand = graphics->createRenderCommand("TransparentDraw");
        transparentCommand->setViewport(viewport);
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
                transparentCommand->bindPipeline(pipeline);
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
                transparentCommand->bindPipeline(pipeline);
            }
            transparentCommand->bindDescriptor({viewParamsSet, t.vertexData->getVertexDataSet(), t.vertexData->getInstanceDataSet(),
                                                scene->getLightEnvironment()->getDescriptorSet(), Material::getDescriptorSet(),
                                                transparentCulling});
            transparentCommand->pushConstants(Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT |
                                                  Gfx::SE_SHADER_STAGE_FRAGMENT_BIT,
                                              0, sizeof(VertexData::DrawCallOffsets), &t.offsets);
            if (graphics->supportMeshShading()) {
                transparentCommand->drawMesh(1, 1, 1);
            } else {
                // command->bindIndexBuffer(t.vertexData->getIndexBuffer());
                // for (const auto& meshData : drawCall.instanceMeshData) {
                //     // all meshlets of a mesh share the same indices offset
                //     command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex,
                //     vertexData->getIndicesOffset(meshData.meshletOffset), 0);
                // }
            }
        }
        commands.add(std::move(transparentCommand));
    }
    // Debug vertices
    if (gDebugVertices.size() > 0) {
        Gfx::ORenderCommand debugCommand = graphics->createRenderCommand("DebugRender");
        debugCommand->setViewport(viewport);
        debugCommand->bindPipeline(debugPipeline);
        debugCommand->bindDescriptor(viewParamsSet);
        debugCommand->bindVertexBuffer({debugVertices});
        debugCommand->draw((uint32)gDebugVertices.size(), 1, 0, 0);
        commands.add(std::move(debugCommand));
    }

    graphics->executeCommands(std::move(commands));
    graphics->endRenderPass();
    query->endQuery();
    timestamps->write(Gfx::SE_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, "END");
    timestamps->end();
    gDebugVertices.clear();
}

void BasePass::endFrame() {}

void BasePass::publishOutputs() {
    TextureCreateInfo depthBufferInfo = {
        .format = Gfx::SE_FORMAT_D32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };
    basePassDepth = graphics->createTexture2D(depthBufferInfo);

    TextureCreateInfo msDepthInfo = {
        .format = Gfx::SE_FORMAT_D32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .samples = viewport->getSamples(),
        .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | Gfx::SE_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
    };
    msBasePassDepth = graphics->createTexture2D(msDepthInfo);

    TextureCreateInfo msBaseColorInfo = {
        .format = viewport->getOwner()->getSwapchainFormat(),
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .samples = viewport->getSamples(),
        .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | Gfx::SE_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
    };
    msBasePassColor = graphics->createTexture2D(msBaseColorInfo);

    depthAttachment =
        Gfx::RenderTargetAttachment(basePassDepth, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_DONT_CARE, Gfx::SE_ATTACHMENT_STORE_OP_STORE);

    msDepthAttachment =
        Gfx::RenderTargetAttachment(msBasePassDepth, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_DONT_CARE);

    colorAttachment = Gfx::RenderTargetAttachment(viewport, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                  Gfx::SE_ATTACHMENT_LOAD_OP_DONT_CARE, Gfx::SE_ATTACHMENT_STORE_OP_STORE);

    msColorAttachment =
        Gfx::RenderTargetAttachment(msBasePassColor, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_DONT_CARE);

    resources->registerRenderPassOutput("BASEPASS_COLOR", colorAttachment);
    resources->registerRenderPassOutput("BASEPASS_DEPTH", depthAttachment);

    query = graphics->createPipelineStatisticsQuery("BasePassPipelineStatistics");
    resources->registerQueryOutput("BASEPASS_QUERY", query);
}

void BasePass::createRenderPass() {
    timestamps = resources->requestTimestampQuery("TIMESTAMP");
    cullingBuffer = resources->requestBuffer("CULLINGBUFFER");

    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{
        .colorAttachments = {msColorAttachment},
        .resolveAttachments = {colorAttachment},
        .depthAttachment = msDepthAttachment,
        .depthResolveAttachment = {depthAttachment},
    };
    Array<Gfx::SubPassDependency> dependency = {
        {
            .srcSubpass = ~0U,
            .dstSubpass = 0,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
        {
            .srcSubpass = 0,
            .dstSubpass = ~0U,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
    };
    renderPass = graphics->createRenderPass(std::move(layout), std::move(dependency), viewport, "BasePass");
    oLightIndexList = resources->requestBuffer("LIGHTCULLING_OLIGHTLIST");
    tLightIndexList = resources->requestBuffer("LIGHTCULLING_TLIGHTLIST");
    oLightGrid = resources->requestTexture("LIGHTCULLING_OLIGHTGRID");
    tLightGrid = resources->requestTexture("LIGHTCULLING_TLIGHTGRID");

    // Debug rendering
    {
        debugPipelineLayout = graphics->createPipelineLayout("DebugPassLayout");
        debugPipelineLayout->addDescriptorLayout(viewParamsLayout);

        ShaderCompilationInfo createInfo = {
            .name = "DebugVertex",
            .modules = {"Debug"},
            .entryPoints =
                {
                    {"vertexMain", "Debug"},
                    {"fragmentMain", "Debug"},
                },
            .rootSignature = debugPipelineLayout,
        };
        graphics->beginShaderCompilation(createInfo);
        debugVertexShader = graphics->createVertexShader({0});
        debugFragmentShader = graphics->createFragmentShader({1});
        debugPipelineLayout->create();

        VertexInputStateCreateInfo inputCreate = {
            .bindings =
                {
                    VertexInputBinding{
                        .binding = 0,
                        .stride = sizeof(DebugVertex),
                        .inputRate = Gfx::SE_VERTEX_INPUT_RATE_VERTEX,
                    },
                },
            .attributes =
                {
                    VertexInputAttribute{
                        .location = 0,
                        .binding = 0,
                        .format = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
                        .offset = 0,
                    },
                    VertexInputAttribute{
                        .location = 1,
                        .binding = 0,
                        .format = Gfx::SE_FORMAT_R32G32B32_SFLOAT,
                        .offset = sizeof(Vector),
                    },
                },
        };
        debugVertexInput = graphics->createVertexInput(inputCreate);
        Gfx::LegacyPipelineCreateInfo gfxInfo = {
            .topology = Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_LIST,
            .vertexInput = debugVertexInput,
            .vertexShader = debugVertexShader,
            .fragmentShader = debugFragmentShader,
            .renderPass = renderPass,
            .pipelineLayout = debugPipelineLayout,
            .multisampleState =
                {
                    .samples = viewport->getSamples(),
                },
            .rasterizationState =
                {
                    .polygonMode = Gfx::SE_POLYGON_MODE_LINE,
                    .lineWidth = 5.f,
                },
            .colorBlend =
                {
                    .attachmentCount = 1,
                },
        };
        debugPipeline = graphics->createGraphicsPipeline(std::move(gfxInfo));
    }

    // Skybox
    {
        skyboxDataLayout = graphics->createDescriptorLayout("pSkyboxData");
        skyboxDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
            .binding = 0,
            .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        });
        skyboxDataLayout->create();
        textureLayout = graphics->createDescriptorLayout("pSkyboxTextures");
        textureLayout->addDescriptorBinding(Gfx::DescriptorBinding{
            .binding = 0,
            .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        });
        textureLayout->addDescriptorBinding(Gfx::DescriptorBinding{
            .binding = 1,
            .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        });
        textureLayout->addDescriptorBinding(Gfx::DescriptorBinding{
            .binding = 2,
            .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
        });
        textureLayout->create();

        skyboxSampler = graphics->createSampler({});

        skyboxData.transformMatrix = Matrix4(1);
        skyboxData.fogColor = skybox.fogColor;
        skyboxData.blendFactor = skybox.blendFactor;

        skyboxBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
            .sourceData =
                {
                    .size = sizeof(SkyboxData),
                    .data = (uint8*)&skyboxData,
                },
            .dynamic = true,
        });

        pipelineLayout = graphics->createPipelineLayout("SkyboxLayout");
        pipelineLayout->addDescriptorLayout(viewParamsLayout);
        pipelineLayout->addDescriptorLayout(skyboxDataLayout);
        pipelineLayout->addDescriptorLayout(textureLayout);

        ShaderCompilationInfo createInfo = {
            .name = "SkyboxVertex",
            .modules = {"Skybox"},
            .entryPoints = {{"vertexMain", "Skybox"}, {"fragmentMain", "Skybox"}},
            .rootSignature = pipelineLayout,
        };
        graphics->beginShaderCompilation(createInfo);
        vertexShader = graphics->createVertexShader({0});
        fragmentShader = graphics->createFragmentShader({1});

        pipelineLayout->create();

        Gfx::LegacyPipelineCreateInfo gfxInfo = {
            .topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .vertexShader = vertexShader,
            .fragmentShader = fragmentShader,
            .renderPass = renderPass,
            .pipelineLayout = pipelineLayout,
            .multisampleState =
                {
                    .samples = viewport->getSamples(),
                },
            .rasterizationState =
                {
                    .polygonMode = Gfx::SE_POLYGON_MODE_FILL,
                },
            .colorBlend =
                {
                    .attachmentCount = 1,
                },
        };
        pipeline = graphics->createGraphicsPipeline(std::move(gfxInfo));
    }
}
