#include "GameView.h"
#include "Graphics/Graphics.h"
#include "Window/Window.h"
#include "Component/KeyboardInput.h"
#include "Actor/CameraActor.h"
#include "Asset/AssetRegistry.h"
#include "Asset/MeshLoader.h"
#include "Asset/TextureLoader.h"
#include "Asset/MaterialLoader.h"

using namespace Seele;

AssetRegistry* instance = new AssetRegistry();

GameView::GameView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo)
    : View(graphics, window, createInfo, "Game")
    , gameInterface("C:\\Users\\Dynamitos\\TrackClear\\bin\\TrackCleard.dll")
    , renderGraph(RenderGraphBuilder::build(
        DepthPrepass(graphics),
        LightCullingPass(graphics),
        BasePass(graphics),
#ifdef EDITOR
        DebugPass(graphics),
#endif
        SkyboxRenderPass(graphics)
    ))
{
    scene = new Scene(graphics);
    gameInterface.reload(instance);
    AssetRegistry::importMesh(MeshImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/arena.fbx",
    });
    AssetRegistry::importMesh(MeshImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/train.fbx",
    });
    AssetRegistry::importMesh(MeshImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/bird.fbx",
    });
    AssetRegistry::importMesh(MeshImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/cube.fbx",
    });
    AssetRegistry::importMesh(MeshImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/flameThrower.fbx",
    });
    AssetRegistry::importMesh(MeshImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/player.fbx",
    });
    AssetRegistry::importMesh(MeshImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/shotgun.fbx",
    });
    AssetRegistry::importMesh(MeshImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/track.fbx",
    });
    AssetRegistry::importMesh(MeshImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/zombie.fbx",
    });

    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Dirt.png",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/DirtGrass.png",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Grass.png",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Ice.png",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Lava.png",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Obsidian.png",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Rocks.png",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Sand.png",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Water.png",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Wood.png",
    });

    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level0/blendMap.png",
        .importPath = "level0",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level0/heightMap.png",
        .importPath = "level0",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level0/trackMap.png",
        .importPath = "level0",
    });
    AssetRegistry::importMaterial(MaterialImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level0/TerrainLevel0.asset",
        .importPath = "level0",
        });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level1/blendMap.png",
        .importPath = "level1",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level1/heightMap.png",
        .importPath = "level1",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level1/trackMap.png",
        .importPath = "level1",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level2/blendMap.png",
        .importPath = "level2",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level2/heightMap.png",
        .importPath = "level2",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level2/trackMap.png",
        .importPath = "level2",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level3/blendMap.png",
        .importPath = "level3",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level3/heightMap.png",
        .importPath = "level3",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level3/trackMap.png",
        .importPath = "level3",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level4/blendMap.png",
        .importPath = "level4",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level4/heightMap.png",
        .importPath = "level4",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level4/trackMap.png",
        .importPath = "level4",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level5/blendMap.png",
        .importPath = "level5",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level5/heightMap.png",
        .importPath = "level5",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level5/trackMap.png",
        .importPath = "level5",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level6/blendMap.png",
        .importPath = "level6",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level6/heightMap.png",
        .importPath = "level6",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level6/trackMap.png",
        .importPath = "level6",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level7/blendMap.png",
        .importPath = "level7",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level7/heightMap.png",
        .importPath = "level7",
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level7/trackMap.png",
        .importPath = "level7",
    });

    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/skyboxes/FS000_Day_01.png",
        .type = TextureImportType::TEXTURE_CUBEMAP,
    });
    AssetRegistry::importTexture(TextureImportArgs {
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/skyboxes/FS000_Night_01.png",
        .type = TextureImportType::TEXTURE_CUBEMAP,
    });

    AssetRegistry::saveRegistry();
    systemGraph = new SystemGraph();
    gameInterface.getGame()->setupScene(scene, systemGraph);
    renderGraph.updateViewport(viewport);
}

GameView::~GameView()
{
    
}

void GameView::beginUpdate()
{
}

void GameView::update()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    systemGraph->run(threadPool, updateTime);
    scene->update(updateTime);
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = (endTime - startTime);
    updateTime = duration.count();
    startTime = endTime;
}

void GameView::commitUpdate()
{
    depthPrepassData.staticDrawList = scene->getStaticMeshes();
    depthPrepassData.sceneDataBuffer = scene->getSceneDataBuffer();
    lightCullingData.lightEnv = scene->getLightBuffer();
    basePassData.staticDrawList = scene->getStaticMeshes();
    basePassData.sceneDataBuffer = scene->getSceneDataBuffer();
    skyboxData.skybox = scene->getSkybox();
#ifdef EDITOR
    if(showDebug)
    {
        debugPassData.vertices = Seele::gDebugVertices;
    }
#endif
    Seele::gDebugVertices.clear();
}

void GameView::prepareRender()
{
#ifdef EDITOR
    renderGraph.updatePassData(depthPrepassData, lightCullingData, basePassData, debugPassData, skyboxData);
#else
    renderGraph.updatePassData(depthPrepassData, lightCullingData, basePassData, skyboxData);
#endif
}

void GameView::render()
{
    scene->view<Component::Camera>([&](Component::Camera& cam)
    {
        if(cam.mainCamera)
        {
            renderGraph.render(cam);
        }
    });
}

void GameView::keyCallback(KeyCode code, InputAction action, KeyModifier)
{
    scene->view<Component::KeyboardInput>([=](Component::KeyboardInput& input)
    {
        //if(code == KeyCode::KEY_R && action == InputAction::PRESS)
        //{
        //    auto cubeMesh = AssetRegistry::findMesh("cube");
        //    PEntity cube2 = new Entity(scene);
        //    Component::Transform& cube2Transform = cube2->attachComponent<Component::Transform>();
        //    cube2Transform.setPosition(Vector(0, 20, 0));
        //    cube2Transform.setRotation(Quaternion(Vector(0.f, 90.f, 90.f)));
        //    cube2Transform.setScale(Vector(2.f, 2.f, 2.f));
        //    cubeMesh->physicsMesh.type = Component::ColliderType::DYNAMIC;
        //    cube2->attachComponent<Component::Collider>(cubeMesh->physicsMesh);
        //    cube2->attachComponent<Component::StaticMesh>(cubeMesh);
        //    Component::RigidBody& physics2 = cube2->attachComponent<Component::RigidBody>();
        //    physics2.linearMomentum = Vector(0, -10, 0);
        //    physics2.angularMomentum = Vector(0, 0, 0);
        //}
        //
        //if(code == KeyCode::KEY_B && action == InputAction::PRESS)
        //{
        //    showDebug = !showDebug;
        //    debugPassData.vertices.clear();
        //}

        input.keys[code] = action != InputAction::RELEASE;
    });
}

void GameView::mouseMoveCallback(double xPos, double yPos)
{
    scene->view<Component::KeyboardInput>([=](Component::KeyboardInput& input)
    {
        input.mouseX = static_cast<float>(xPos);
        input.mouseY = static_cast<float>(yPos);
    });
}

void GameView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier)
{
    scene->view<Component::KeyboardInput>([=](Component::KeyboardInput& input)
    {
        if (button == MouseButton::MOUSE_BUTTON_1)
        {
            input.mouse1 = action != InputAction::RELEASE;
        }
        if (button == MouseButton::MOUSE_BUTTON_2)
        {
            input.mouse2 = action != InputAction::RELEASE;
        }
    });
}

void GameView::scrollCallback(double, double)
{
    
}

void GameView::fileCallback(int, const char**)
{
    
}
