#include "Window/WindowManager.h"
#include "Window/SceneView.h"
#include "Window/PlayView.h"
#include "Window/InspectorView.h"
#include "Asset/AssetRegistry.h"
#include "Asset/AssetImporter.h"
#include "Asset/TextureLoader.h"
#include "Graphics/Vulkan/VulkanGraphics.h"
#include "Asset/MeshLoader.h"
#include "Asset/TextureLoader.h"
#include "Asset/MaterialLoader.h"
#include "Asset/FontLoader.h"
#include "Asset/AssetImporter.h"

using namespace Seele;
using namespace Seele::Editor;

extern AssetRegistry* instance;

int main()
{
    std::string outputPath = "C:/Users/Dynamitos/TrackClearGame";
    std::string cmakePath = "C:/Users/Dynamitos/TrackClearGame/cmake";
    std::string gameName = "TrackClear";
    std::string sourcePath = "C:/Users/Dynamitos/TrackClear/";
    std::string binaryPath = "C:/Users/Dynamitos/TrackClear/bin/TrackClear.dll";

    Gfx::PGraphics graphics = new Vulkan::Graphics();

    GraphicsInitializer initializer;
    graphics->init(initializer);
    PWindowManager windowManager = new WindowManager();
    AssetRegistry::init(std::string("C:/Users/Dynamitos/TrackClear/Assets"), graphics);
    AssetImporter::init(graphics, instance);
    AssetImporter::importFont(FontImportArgs{
        .filePath = "./fonts/Calibri.ttf",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/arena.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/train.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/bird.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/cube.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/flameThrower.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/player.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/shotgun.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/track.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/models/zombie.fbx",
        });

    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Dirt.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/DirtGrass.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Grass.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Ice.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Lava.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Obsidian.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Rocks.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Sand.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Water.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/Wood.png",
        });

    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level0/blendMap.png",
        .importPath = "level0",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level0/heightMap.png",
        .importPath = "level0",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level0/trackMap.png",
        .importPath = "level0",
        });
    AssetImporter::importMaterial(MaterialImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/shaders/TerrainMaterial.json",
        .importPath = "level0",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level1/blendMap.png",
        .importPath = "level1",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level1/heightMap.png",
        .importPath = "level1",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level1/trackMap.png",
        .importPath = "level1",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level2/blendMap.png",
        .importPath = "level2",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level2/heightMap.png",
        .importPath = "level2",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level2/trackMap.png",
        .importPath = "level2",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level3/blendMap.png",
        .importPath = "level3",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level3/heightMap.png",
        .importPath = "level3",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level3/trackMap.png",
        .importPath = "level3",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level4/blendMap.png",
        .importPath = "level4",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level4/heightMap.png",
        .importPath = "level4",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level4/trackMap.png",
        .importPath = "level4",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level5/blendMap.png",
        .importPath = "level5",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level5/heightMap.png",
        .importPath = "level5",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level5/trackMap.png",
        .importPath = "level5",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level6/blendMap.png",
        .importPath = "level6",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level6/heightMap.png",
        .importPath = "level6",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level6/trackMap.png",
        .importPath = "level6",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level7/blendMap.png",
        .importPath = "level7",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level7/heightMap.png",
        .importPath = "level7",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/textures/level7/trackMap.png",
        .importPath = "level7",
        });

    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/skyboxes/FS000_Day_01.png",
        .type = TextureImportType::TEXTURE_CUBEMAP,
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath = "C:/Users/Dynamitos/TrackClear/old_resources/skyboxes/FS000_Night_01.png",
        .type = TextureImportType::TEXTURE_CUBEMAP,
        });

    WindowCreateInfo mainWindowInfo;
    mainWindowInfo.title = "SeeleEngine";
    mainWindowInfo.width = 1280;
    mainWindowInfo.height = 720;
    mainWindowInfo.bFullscreen = false;
    mainWindowInfo.numSamples = 1;
    mainWindowInfo.pixelFormat = Gfx::SE_FORMAT_B8G8R8A8_UNORM;
    auto window = windowManager->addWindow(graphics, mainWindowInfo);
    ViewportCreateInfo sceneViewInfo;
    sceneViewInfo.dimensions.size.x = 1280;
    sceneViewInfo.dimensions.size.y = 720;
    sceneViewInfo.dimensions.offset.x = 0;
    sceneViewInfo.dimensions.offset.y = 0;
    PGameView sceneView = new Editor::PlayView(graphics, window, sceneViewInfo, binaryPath);
    
    //ViewportCreateInfo inspectorViewInfo;
    //inspectorViewInfo.dimensions.size.x = 640;
    //inspectorViewInfo.dimensions.size.y = 720;
    //inspectorViewInfo.dimensions.offset.x = 640;
    //inspectorViewInfo.dimensions.offset.y = 0;
    //PInspectorView inspectorView = new InspectorView(graphics, window, inspectorViewInfo);
    //window->addView(inspectorView);
    sceneView->setFocused();

    window->render();
    //export game

    std::filesystem::create_directories(outputPath);
    std::system(std::format("cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DGAME_TITLE=\"{}\" -DGAME_DESTINATION=\"{}\" -DGAME_BINARY=\"{}\" -P ./cmake/ExportProject.cmake", gameName, outputPath, binaryPath).c_str());
    std::system(std::format("cmake -S {} -B {}", cmakePath, cmakePath).c_str());
    std::system(std::format("cmake --build {}", cmakePath).c_str());
    std::filesystem::copy(std::filesystem::path(sourcePath) / "Assets", std::filesystem::path(outputPath) / "Assets", std::filesystem::copy_options::recursive);
    std::filesystem::copy("shaders", std::filesystem::path(outputPath) / "shaders", std::filesystem::copy_options::recursive);
    std::filesystem::copy("textures", std::filesystem::path(outputPath) / "textures", std::filesystem::copy_options::recursive);
    std::filesystem::copy_file("assimp-vc143-mt.dll", std::filesystem::path(outputPath) / "assimp-vc143-mt.dll");
    std::filesystem::copy_file("slang.dll", std::filesystem::path(outputPath) / "slang.dll");
    std::filesystem::copy_file("slang-glslang.dll", std::filesystem::path(outputPath) / "slang-glslang.dll");
    std::filesystem::copy_file("slang-llvm.dll", std::filesystem::path(outputPath) / "slang-llvm.dll");

    return 0;
}