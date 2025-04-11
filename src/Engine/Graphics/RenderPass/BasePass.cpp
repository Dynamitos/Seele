#include "BasePass.h"
#include "Actor/CameraActor.h"
#include "Asset/AssetRegistry.h"
#include "Asset/EnvironmentMapAsset.h"
#include "Component/Camera.h"
#include "Component/Mesh.h"
#include "Component/WaterTile.h"
#include "Graphics/Command.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Shader.h"
#include "Graphics/StaticMeshVertexData.h"
#include "Material/MaterialInstance.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "RenderGraph.h"
#include "Window/Window.h"

using namespace Seele;

Array<DebugVertex> gDebugVertices;

void Seele::addDebugVertex(DebugVertex vert) { gDebugVertices.add(vert); }

void Seele::addDebugVertices(Array<DebugVertex> verts) { gDebugVertices.addAll(verts); }

BasePass::BasePass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics), scene(scene) {
    //waterRenderer = new WaterRenderer(graphics, scene, viewParamsLayout);
    basePassLayout = graphics->createPipelineLayout("BasePassLayout");

    basePassLayout->addDescriptorLayout(viewParamsLayout);
    basePassLayout->addDescriptorLayout(scene->getLightEnvironment()->getDescriptorLayout());
    basePassLayout->addDescriptorLayout(Material::getDescriptorLayout());

    lightCullingLayout = graphics->createDescriptorLayout("pLightCullingData");
    lightCullingLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = LIGHTINDEX_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    lightCullingLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = LIGHTGRID_NAME,
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
        .day = scene->getLightEnvironment()->getEnvironmentMap()->getSkybox(),
        .night = scene->getLightEnvironment()->getEnvironmentMap()->getSkybox(),
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

    //waterRenderer->beginFrame();
    //terrainRenderer->beginFrame(viewParamsSet, cam);

    // Debug vertices
    {
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
    }
    // Skybox
    {
        skyboxDataLayout->reset();
        textureLayout->reset();
        skyboxData.transformMatrix = glm::rotate(skyboxData.transformMatrix, (float)(Gfx::getCurrentFrameDelta()), Vector(0, 1, 0));
        
        skyboxDataSet = skyboxDataLayout->allocateDescriptorSet();
        skyboxDataSet->updateConstants("transformMatrix", 0, &skyboxData.transformMatrix);
        skyboxDataSet->updateConstants("fogBlend", 0, &skyboxData.fogColor);
        skyboxDataSet->writeChanges();
        textureSet = textureLayout->allocateDescriptorSet();
        textureSet->updateTexture(SKYBOXDAY_NAME, 0, skybox.day);
        textureSet->updateTexture(SKYBOXNIGHT_NAME, 0, skybox.night);
        textureSet->updateSampler(SKYBOXSAMPLER_NAME, 0, skyboxSampler);
        textureSet->writeChanges();
    }
}

void BasePass::render() {
    opaqueCulling->updateBuffer(LIGHTINDEX_NAME, 0, oLightIndexList);
    opaqueCulling->updateTexture(LIGHTGRID_NAME, 0, oLightGrid);
    transparentCulling->updateBuffer(LIGHTINDEX_NAME, 0, tLightIndexList);
    transparentCulling->updateTexture(LIGHTGRID_NAME, 0, tLightGrid);
    opaqueCulling->writeChanges();
    transparentCulling->writeChanges();

    query->beginQuery();
    timestamps->write(Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "BaseBegin");
    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;
    Gfx::ShaderPermutation permutation = graphics->getShaderCompiler()->getTemplate("BasePass");
    permutation.setDepthCulling(true); // always use the culling info
    permutation.setPositionOnly(false);
    Array<VertexData::TransparentDraw> transparentData;
    // Base Rendering
    for (VertexData* vertexData : VertexData::getList()) {
        transparentData.addAll(vertexData->getTransparentData());
        vertexData->getInstanceDataSet()->updateBuffer(VertexData::CULLINGDATA_NAME, 0, cullingBuffer);
        vertexData->getInstanceDataSet()->writeChanges();
        permutation.setVertexData(vertexData->getTypeName());
        for (const auto& materialData : vertexData->getMaterialData()) {
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
            permutation.setMaterial(materialData.material->getName(), materialData.material->getProfile());
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
                            .samples = msColorAttachment.getNumSamples(),
                        },
                    .rasterizationState =
                        {
                            .cullMode = Gfx::SE_CULL_MODE_BACK_BIT,
                        },
                    .colorBlend =
                        {
                            .attachmentCount = 1,
                        },
                };
                command->bindPipeline(graphics->createGraphicsPipeline(std::move(pipelineInfo)));
            } else {
                Gfx::LegacyPipelineCreateInfo pipelineInfo = {
                    .vertexShader = collection->vertexShader,
                    .fragmentShader = collection->fragmentShader,
                    .renderPass = renderPass,
                    .pipelineLayout = collection->pipelineLayout,
                    .multisampleState =
                        {
                            .samples = msColorAttachment.getNumSamples(),
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
                command->bindPipeline(graphics->createGraphicsPipeline(std::move(pipelineInfo)));
            }
            command->bindDescriptor({viewParamsSet, vertexData->getVertexDataSet(), vertexData->getInstanceDataSet(),
                                     scene->getLightEnvironment()->getDescriptorSet(), Material::getDescriptorSet(), opaqueCulling});
            for (const auto& drawCall : materialData.instances) {
                command->pushConstants(Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT |
                                           Gfx::SE_SHADER_STAGE_FRAGMENT_BIT,
                                       0, sizeof(VertexData::DrawCallOffsets), &drawCall.offsets);
                if (graphics->supportMeshShading()) {
                    command->drawMesh((uint32)drawCall.instanceMeshData.size(), 1, 1);
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
    
    //commands.add(waterRenderer->render(viewParamsSet));
    //commands.add(terrainRenderer->render(viewParamsSet));
    
    // Skybox
    {
        Gfx::ORenderCommand skyboxCommand = graphics->createRenderCommand("SkyboxRender");
        skyboxCommand->setViewport(viewport);
        skyboxCommand->bindPipeline(skyboxPipeline);
        skyboxCommand->bindDescriptor({viewParamsSet, skyboxDataSet, textureSet});
        skyboxCommand->draw(36, 1, 0, 0);
        graphics->executeCommands(std::move(skyboxCommand));
    }
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
            permutation.setMaterial(t.matInst->getBaseMaterial()->getName(), t.matInst->getBaseMaterial()->getProfile());
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
                            .samples = msColorAttachment.getNumSamples(),
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
                            .samples = msColorAttachment.getNumSamples(),
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
    timestamps->write(Gfx::SE_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, "BaseEnd");
    gDebugVertices.clear();
}

void BasePass::endFrame() {}

void BasePass::publishOutputs() {
    basePassDepth = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_D32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    });

    basePassColor = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    });

    msBasePassDepth = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_D32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .samples = Gfx::SE_SAMPLE_COUNT_4_BIT,
        .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | Gfx::SE_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
    });
    
    msBasePassColor = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .samples = Gfx::SE_SAMPLE_COUNT_4_BIT, // todo: configure
        .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | Gfx::SE_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
    });

    depthAttachment =
        Gfx::RenderTargetAttachment(Gfx::PTexture2D(basePassDepth), Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_DONT_CARE, Gfx::SE_ATTACHMENT_STORE_OP_STORE);

    msDepthAttachment =
        Gfx::RenderTargetAttachment(msBasePassDepth, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_DONT_CARE);
    msDepthAttachment.clear.depthStencil.depth = 0.0f;

    colorAttachment = Gfx::RenderTargetAttachment(basePassColor, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                  Gfx::SE_ATTACHMENT_LOAD_OP_DONT_CARE, Gfx::SE_ATTACHMENT_STORE_OP_STORE);

    msColorAttachment =
        Gfx::RenderTargetAttachment(msBasePassColor, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_DONT_CARE);
    msColorAttachment.clear.color.float32[0] = 0;
    msColorAttachment.clear.color.float32[1] = 1;
    msColorAttachment.clear.color.float32[2] = 0;
    msColorAttachment.clear.color.float32[3] = 1;

    resources->registerRenderPassOutput("BASEPASS_COLOR", colorAttachment);
    resources->registerRenderPassOutput("BASEPASS_DEPTH", depthAttachment);

    timestamps = graphics->createTimestampQuery(2, "BaseTS");
    query = graphics->createPipelineStatisticsQuery("BasePassPipelineStatistics");
    resources->registerQueryOutput("BASEPASS_QUERY", query);
}

void BasePass::createRenderPass() {
    //RenderPass::beginFrame(Component::Camera());
    //terrainRenderer = new TerrainRenderer(graphics, scene, viewParamsLayout, viewParamsSet);
    cullingBuffer = resources->requestBuffer("CULLINGBUFFER");
    timestamps = resources->requestTimestampQuery("TIMESTAMPS");

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
    renderPass = graphics->createRenderPass(std::move(layout), std::move(dependency), "BasePass");
    oLightIndexList = resources->requestBuffer("LIGHTCULLING_OLIGHTLIST");
    tLightIndexList = resources->requestBuffer("LIGHTCULLING_TLIGHTLIST");
    oLightGrid = resources->requestTexture("LIGHTCULLING_OLIGHTGRID");
    tLightGrid = resources->requestTexture("LIGHTCULLING_TLIGHTGRID");

    //waterRenderer->setViewport(viewport, renderPass);
    //terrainRenderer->setViewport(viewport, renderPass);

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
                    .samples = msColorAttachment.getNumSamples(),
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
            .name = "transformMatrix",
            .uniformLength = sizeof(Matrix4)
        });
        skyboxDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
            .name = "fogBlend",
            .uniformLength = sizeof(Vector4)
        });
        skyboxDataLayout->create();
        textureLayout = graphics->createDescriptorLayout("pSkyboxTextures");
        textureLayout->addDescriptorBinding(Gfx::DescriptorBinding{
            .name = SKYBOXDAY_NAME,
            .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .access = Gfx::SE_DESCRIPTOR_ACCESS_SAMPLE_BIT,
        });
        textureLayout->addDescriptorBinding(Gfx::DescriptorBinding{
            .name = SKYBOXNIGHT_NAME,
            .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .access = Gfx::SE_DESCRIPTOR_ACCESS_SAMPLE_BIT,
        });
        textureLayout->addDescriptorBinding(Gfx::DescriptorBinding{
            .name = SKYBOXSAMPLER_NAME,
            .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
        });
        textureLayout->create();

        skyboxSampler = graphics->createSampler({});

        skyboxData.transformMatrix = Matrix4(1);
        skyboxData.fogColor = skybox.fogColor;
        skyboxData.blendFactor = skybox.blendFactor;

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
                    .samples = msColorAttachment.getNumSamples(),
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
        skyboxPipeline = graphics->createGraphicsPipeline(std::move(gfxInfo));
    }
}
