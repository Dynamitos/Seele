#include "GameView.h"
#include "Actor/CameraActor.h"
#include "Asset/AssetRegistry.h"
#include "Component/KeyboardInput.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/Query.h"
#include "Graphics/RenderPass/BasePass.h"
#include "Graphics/RenderPass/CachedDepthPass.h"
#include "Graphics/RenderPass/DepthCullingPass.h"
#include "Graphics/RenderPass/LightCullingPass.h"
#include "Graphics/RenderPass/RayTracingPass.h"
#include "Graphics/RenderPass/RenderGraphResources.h"
#include "Graphics/RenderPass/VisibilityPass.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/Resources.h"
#include "Graphics/StaticMeshVertexData.h"
#include "System/CameraUpdater.h"
#include "System/LightGather.h"
#include "Component/PointLight.h"
#include "Component/DirectionalLight.h"
#include "System/MeshUpdater.h"
#include "Window/Window.h"
#include "Graphics/Mesh.h"
#include "Graphics/Metal/Descriptor.h"
#include "Graphics/Metal/Shader.h"
#include <fstream>
#include <thread>

using namespace Seele;

GameView::GameView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::string dllPath)
    : View(graphics, window, createInfo, "Game"), scene(new Scene(graphics)), gameInterface(dllPath) {
    reloadGame();
    // renderGraph.addPass(new CachedDepthPass(graphics, scene));
    // renderGraph.addPass(new DepthCullingPass(graphics, scene));
    // renderGraph.addPass(new VisibilityPass(graphics, scene));
    // renderGraph.addPass(new LightCullingPass(graphics, scene));
    // renderGraph.addPass(new BasePass(graphics, scene));

    // renderGraph.addPass(new RayTracingPass(graphics, scene));

    renderPass = graphics->createRenderPass(
        Gfx::RenderTargetLayout{
            .colorAttachments = {Gfx::RenderTargetAttachment(viewport, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                             Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE)}},
        {}, viewport);
    
    viewParamsLayout = graphics->createDescriptorLayout("pViewParams");
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    });
    UniformBufferCreateInfo uniformInitializer = {
        .sourceData =
            {
                .size = sizeof(ViewParameter),
                .data = (uint8*)&viewParams,
            },
        .dynamic = true,
        .name = "viewParamsBuffer",
    };
    viewParamsBuffer = graphics->createUniformBuffer(uniformInitializer);
    viewParamsLayout->create();
    layout = graphics->createPipelineLayout();
    layout->addDescriptorLayout(viewParamsLayout);
    layout->addDescriptorLayout(StaticMeshVertexData::getInstance()->getVertexDataLayout());
    layout->addDescriptorLayout(StaticMeshVertexData::getInstance()->getInstanceDataLayout());
    layout->addDescriptorLayout(scene->getLightEnvironment()->getDescriptorLayout());
    layout->addDescriptorLayout(Material::getDescriptorLayout());
    layout->addPushConstants(Gfx::SePushConstantRange{
        .size = sizeof(VertexData::DrawCallOffsets),
        .name = "pOffsets",
        .offset = 0,
    });
    ShaderCompilationInfo s{
        .name = "MetalTest",
        .defines = {{"MATERIAL_FILE_NAME", "PlaceholderMaterial"}},
        .modules = {"DrawListTask", "DrawListMesh", "LegacyPass", "BasePass", "StaticMeshVertexData", "PlaceholderMaterial"},
        .entryPoints = {{"taskMain", "DrawListTask"}, {"meshMain", "DrawListMesh"}, {"vertexMain", "LegacyPass"}, {"fragmentMain", "BasePass"}},
        .dumpIntermediate = true,
        .rootSignature = layout,
    };
    graphics->beginShaderCompilation(s);
    layout->create();
    task = graphics->createTaskShader({0});
    mesh = graphics->createMeshShader({1});
    vert = graphics->createVertexShader({2});
    frag = graphics->createFragmentShader({3});
    pipeline = graphics->createGraphicsPipeline(Gfx::MeshPipelineCreateInfo{
        .taskShader = task,
        .meshShader = mesh,
        .fragmentShader = frag,
        .pipelineLayout = layout,
        .renderPass = renderPass,
    });
        ((Metal::DescriptorLayout*)Material::getDescriptorLayout().getHandle())->setFunction(((Metal::FragmentShader*)*frag)->getFunction(), 6);
        ((Metal::DescriptorLayout*)scene->getLightEnvironment()->getDescriptorLayout().getHandle())->setFunction(((Metal::FragmentShader*)*frag)->getFunction(), 5);
        ((Metal::DescriptorLayout*)StaticMeshVertexData::getInstance()->getInstanceDataLayout().getHandle())->setFunction(((Metal::MeshShader*)*mesh)->getFunction(), 4);
        ((Metal::DescriptorLayout*)*viewParamsLayout)->setFunction(((Metal::MeshShader*)*mesh)->getFunction(), 2);
        ((Metal::DescriptorLayout*)StaticMeshVertexData::getInstance()->getVertexDataLayout().getHandle())->setFunction(((Metal::MeshShader*)*mesh)->getFunction(), 1);
        StaticMeshVertexData::getInstance()->commitMeshes();
    StaticMeshVertexData::getInstance()->addCullingMapping(AssetRegistry::getInstance()->findMesh("", "cube")->meshes[0]->id);
    scene->getLightEnvironment()->reset();
    scene->getLightEnvironment()->addPointLight({Vector4(0, 1, 0, 1), Vector4(0, 1, 0, 1)});
    scene->getLightEnvironment()->addDirectionalLight({Vector4(0, 1, 0, 1), Vector4(0, 1, 0, 1)});
    Component::Camera cam;
    cam.viewMatrix = glm::lookAt(Vector(0, 0, -10), Vector(0, 0, 0), Vector(0, 1, 0));
    viewParams = {
        .viewMatrix = cam.getViewMatrix(),
        .inverseViewMatrix = glm::inverse(cam.getViewMatrix()),
        .projectionMatrix = viewport->getProjectionMatrix(),
        .inverseProjection = glm::inverse(viewport->getProjectionMatrix()),
        .cameraPosition = Vector4(cam.getCameraPosition(), 1),
        .screenDimensions = Vector2(static_cast<float>(viewport->getWidth()), static_cast<float>(viewport->getHeight())),
    };
    viewParamsBuffer->rotateBuffer(sizeof(ViewParameter));
    viewParamsBuffer->updateContents(0, sizeof(ViewParameter), &viewParams);
    viewParamsBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                      Gfx::SE_ACCESS_UNIFORM_READ_BIT | Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                      Gfx::SE_PIPELINE_STAGE_TASK_SHADER_BIT_EXT | Gfx::SE_PIPELINE_STAGE_MESH_SHADER_BIT_EXT |
                                          Gfx::SE_PIPELINE_STAGE_VERTEX_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                          Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    viewParamsLayout->reset();
    viewParamsSet = viewParamsLayout->allocateDescriptorSet();
    viewParamsSet->updateBuffer(0, viewParamsBuffer);
    viewParamsSet->writeChanges();
    // input = graphics->createVertexInput(VertexInputStateCreateInfo{
    //     .attributes = {},
    //     .bindings = {},
    // });
    // pipeline = graphics->createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo{
    //     .vertexInput = input,
    //     .vertexShader = vert,
    //     .fragmentShader = frag,
    //     .pipelineLayout = layout,
    //     .renderPass = renderPass,
    // });
    renderGraph.setViewport(viewport);
    renderGraph.createRenderPass();
}

GameView::~GameView() {}

void GameView::beginUpdate() {}

void GameView::update() {
    // static auto startTime = std::chrono::high_resolution_clock::now();
    for (VertexData* vd : VertexData::getList()) {
        vd->resetMeshData();
        vd->updateMesh(0, AssetRegistry::findMesh("", "cube")->meshes[0], Component::Transform());
    }
    // systemGraph->run(updateTime);
    // scene->update(updateTime);
    for (VertexData* vd : VertexData::getList()) {
        vd->createDescriptors();
    }
    scene->getLightEnvironment()->commit();
    // auto endTime = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<float> duration = (endTime - startTime);
    // updateTime = duration.count();
    // startTime = endTime;
    graphics->beginRenderPass(renderPass);
    Gfx::ORenderCommand cmd = graphics->createRenderCommand("TestCommand");
    cmd->bindPipeline(pipeline);
    cmd->bindDescriptor({viewParamsSet, StaticMeshVertexData::getInstance()->getVertexDataSet(),
                         StaticMeshVertexData::getInstance()->getInstanceDataSet(), scene->getLightEnvironment()->getDescriptorSet()});
    cmd->setViewport(viewport);
    VertexData::DrawCallOffsets offsets = {
        .floatOffset = 0,
        .instanceOffset = 0,
        .samplerOffset = 0,
        .textureOffset = 0,
    };
    cmd->pushConstants(Gfx::SE_SHADER_STAGE_ALL, 0, sizeof(VertexData::DrawCallOffsets), &offsets);
    // cmd->draw(3, 1, 0, 0);
    cmd->drawMesh(1, 1, 1);
    Array<Gfx::ORenderCommand> temp;
    temp.add(std::move(cmd));
    graphics->executeCommands(std::move(temp));
    graphics->endRenderPass();
}

void GameView::commitUpdate() {}

void GameView::prepareRender() {}

void GameView::render() {
    Component::Camera cam;
    scene->view<Component::Camera>([&cam](Component::Camera& c) {
        if (c.mainCamera)
            cam = c;
    });
    renderGraph.render(cam);
}

void GameView::applyArea(URect) { renderGraph.setViewport(viewport); }

void GameView::reloadGame() {
    gameInterface.reload();

    systemGraph = new SystemGraph();
    gameInterface.getGame()->setupScene(scene, systemGraph);
    System::OKeyboardInput keyInput = new System::KeyboardInput(scene);
    keyboardSystem = keyInput;
    systemGraph->addSystem(std::move(keyInput));
    systemGraph->addSystem(new System::LightGather(scene));
    systemGraph->addSystem(new System::MeshUpdater(scene));
    systemGraph->addSystem(new System::CameraUpdater(scene));
}

void GameView::keyCallback(KeyCode code, InputAction action, KeyModifier modifier) { keyboardSystem->keyCallback(code, action, modifier); }

void GameView::mouseMoveCallback(double xPos, double yPos) { keyboardSystem->mouseCallback(xPos, yPos); }

void GameView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) {
    keyboardSystem->mouseButtonCallback(button, action, modifier);
}

void GameView::scrollCallback(double xScroll, double yScroll) { keyboardSystem->scrollCallback(xScroll, yScroll); }

void GameView::fileCallback(int, const char**) {}
