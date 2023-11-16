#include "Window/WindowManager.h"
#include "Window/SceneView.h"
#include "Window/PlayView.h"
#include "Window/InspectorView.h"
#include "Asset/AssetRegistry.h"
#include "Asset/AssetImporter.h"
#include "Asset/TextureLoader.h"
#include "Graphics/Vulkan/Graphics.h"
#include "Asset/MeshLoader.h"
#include "Asset/TextureLoader.h"
#include "Asset/MaterialLoader.h"
#include "Asset/FontLoader.h"
#include "Asset/AssetImporter.h"
#include "Graphics/StaticMeshVertexData.h"
#include <format>

using namespace Seele;
using namespace Seele::Editor;

int main()
{
#ifdef WIN32
    std::filesystem::path outputPath = "C:/Users/Dynamitos/TrackClearGame";
    std::filesystem::path sourcePath= "C:/Users/Dynamitos/TrackClear";
    std::filesystem::path binaryPath = sourcePath / "bin" / "TrackClear.dll";
#else
    std::filesystem::path outputPath = "/home/dynamitos/TrackClearGame";
    std::filesystem::path sourcePath= "/home/dynamitos/TrackClear";
    std::filesystem::path binaryPath = sourcePath / "cmake" / "libTrackClear.so";
#endif
    std::string gameName = "TrackClear";
    std::filesystem::path cmakePath = outputPath / "cmake";

    Gfx::OGraphics graphics = new Vulkan::Graphics();

    GraphicsInitializer initializer;
    graphics->init(initializer);
    StaticMeshVertexData* vd = StaticMeshVertexData::getInstance();
    vd->init(graphics);
    PWindowManager windowManager = new WindowManager();
    AssetRegistry::init(sourcePath / "Assets", graphics);
    AssetImporter::init(graphics);
    AssetImporter::importFont(FontImportArgs{
        .filePath = "./fonts/Calibri.ttf",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath = sourcePath / "old_resources/models/arena.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath = sourcePath / "old_resources/models/train.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath= sourcePath / "old_resources/models/bird.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath= sourcePath / "old_resources/models/cube.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath= sourcePath / "old_resources/models/flameThrower.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath= sourcePath / "old_resources/models/player.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath= sourcePath / "old_resources/models/shotgun.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath= sourcePath / "old_resources/models/track.fbx",
        });
    AssetImporter::importMesh(MeshImportArgs{
        .filePath= sourcePath / "old_resources/models/zombie.fbx",
        });

    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/Dirt.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/DirtGrass.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/Grass.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/Ice.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/Lava.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/Obsidian.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/Rocks.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/Sand.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/Water.png",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/Wood.png",
        });

    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level0/blendMap.png",
        .importPath = "level0",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level0/heightMap.png",
        .importPath = "level0",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level0/trackMap.png",
        .importPath = "level0",
        });
    AssetImporter::importMaterial(MaterialImportArgs{
        .filePath= sourcePath / "old_resources/shaders/TerrainMaterial.json",
        .importPath = "level0",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level1/blendMap.png",
        .importPath = "level1",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level1/heightMap.png",
        .importPath = "level1",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level1/trackMap.png",
        .importPath = "level1",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level2/blendMap.png",
        .importPath = "level2",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level2/heightMap.png",
        .importPath = "level2",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level2/trackMap.png",
        .importPath = "level2",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level3/blendMap.png",
        .importPath = "level3",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level3/heightMap.png",
        .importPath = "level3",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level3/trackMap.png",
        .importPath = "level3",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level4/blendMap.png",
        .importPath = "level4",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level4/heightMap.png",
        .importPath = "level4",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level4/trackMap.png",
        .importPath = "level4",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level5/blendMap.png",
        .importPath = "level5",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level5/heightMap.png",
        .importPath = "level5",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level5/trackMap.png",
        .importPath = "level5",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level6/blendMap.png",
        .importPath = "level6",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level6/heightMap.png",
        .importPath = "level6",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level6/trackMap.png",
        .importPath = "level6",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level7/blendMap.png",
        .importPath = "level7",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level7/heightMap.png",
        .importPath = "level7",
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/textures/level7/trackMap.png",
        .importPath = "level7",
        });

    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/skyboxes/FS000_Day_01.png",
        .type = TextureImportType::TEXTURE_CUBEMAP,
        });
    AssetImporter::importTexture(TextureImportArgs{
        .filePath= sourcePath / "old_resources/skyboxes/FS000_Night_01.png",
        .type = TextureImportType::TEXTURE_CUBEMAP,
        });

    WindowCreateInfo mainWindowInfo;
    mainWindowInfo.title = "SeeleEngine";
    mainWindowInfo.width = 1280;
    mainWindowInfo.height = 720;
    mainWindowInfo.numSamples = 1;
    mainWindowInfo.preferredFormat = Gfx::SE_FORMAT_B8G8R8A8_SRGB;
    auto window = windowManager->addWindow(graphics, mainWindowInfo);
    ViewportCreateInfo sceneViewInfo;
    sceneViewInfo.dimensions.size.x = 1280;
    sceneViewInfo.dimensions.size.y = 720;
    sceneViewInfo.dimensions.offset.x = 0;
    sceneViewInfo.dimensions.offset.y = 0;
    OGameView sceneView = new Editor::PlayView(graphics, window, sceneViewInfo, binaryPath.generic_string());
    
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
    std::system(std::format("cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DGAME_TITLE=\"{}\" -DGAME_DESTINATION=\"{}\" -DGAME_BINARY=\"{}\" -P ./cmake/ExportProject.cmake", gameName, outputPath.generic_string(), binaryPath.generic_string()).c_str());
    std::system(std::format("cmake -S {} -B {}", cmakePath.generic_string(), cmakePath.generic_string()).c_str());
    std::system(std::format("cmake --build {}", cmakePath.generic_string()).c_str());
    std::filesystem::copy(sourcePath / "Assets", outputPath / "Assets", std::filesystem::copy_options::recursive);
    std::filesystem::copy("shaders", outputPath / "shaders", std::filesystem::copy_options::recursive);
    std::filesystem::copy("textures", outputPath / "textures", std::filesystem::copy_options::recursive);
#ifdef WIN32
    std::filesystem::copy_file("assimp-vc143-mt.dll", outputPath / "assimp-vc143-mt.dll");
    std::filesystem::copy_file("slang.dll", outputPath / "slang.dll");
    std::filesystem::copy_file("slang-glslang.dll", outputPath / "slang-glslang.dll");
    std::filesystem::copy_file("slang-llvm.dll", outputPath / "slang-llvm.dll");
#endif
    return 0;
}