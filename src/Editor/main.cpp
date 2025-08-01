#include "Asset/AssetImporter.h"
#include "Asset/AssetRegistry.h"
#include "Asset/EnvironmentLoader.h"
#include "Asset/FontLoader.h"
#include "Asset/MaterialLoader.h"
#include "Asset/MeshLoader.h"
#include "Asset/TextureLoader.h"
#include "Graphics/Initializer.h"
#ifdef __APPLE__
#include "Graphics/Metal/Graphics.h"
#else
#endif
#include "Graphics/StaticMeshVertexData.h"
#include "Graphics/Vulkan/Graphics.h"
#include "Window/InspectorView.h"
#include "Window/PlayView.h"
#include "Window/WindowManager.h"
#include <fmt/core.h>
#include <random>

using namespace Seele;
using namespace Seele::Editor;

// make it global so it gets deleted last and automatically
static Gfx::OGraphics graphics;

int main() {
    std::string gameName = "MeshShadingDemo";
    std::filesystem::path outputPath = fmt::format("../../../../{0}Game/", gameName);
    std::filesystem::path sourcePath = fmt::format("../../../../{0}/", gameName);
#ifdef WIN32
    std::string libraryEnding = "dll";
#elif __APPLE__
    std::string libraryEnding = "dylib";
#else
    std::string libraryEnding = "so";
#endif
    std::filesystem::path binaryPath = sourcePath / "bin" / fmt::format("{}.{}", gameName, libraryEnding);
    std::filesystem::path cmakePath = outputPath / "cmake";
    if (true) {
#ifdef __APPLE__
        graphics = new Metal::Graphics();
#else
        graphics = new Vulkan::Graphics();
#endif
        GraphicsInitializer initializer;
        graphics->init(initializer);
        StaticMeshVertexData* vd = StaticMeshVertexData::getInstance();
        vd->init(graphics);

        OWindowManager windowManager = new WindowManager();
        AssetRegistry::init(sourcePath / "Assets", graphics);
        AssetImporter::init(graphics);
        AssetImporter::importFont(FontImportArgs{
            .filePath = "./fonts/arial.ttf",
        });
        AssetImporter::importEnvironmentMap(EnvironmentImportArgs{
            .filePath = sourcePath / "import" / "textures" / "newport_loft.hdr",
        });
        // AssetImporter::importTexture(TextureImportArgs{
        //     .filePath = sourcePath / "import" / "textures" / "grass_block_side.png",
        //     .importPath = "",
        // });
        //AssetImporter::importMesh(MeshImportArgs{
        //    .filePath = sourcePath / "import" / "models" / "main1_sponza" / "sponza.gltf",
        //    .importPath = "sponza",
        //});
        AssetImporter::importMesh(MeshImportArgs{
            .filePath = sourcePath / "import" / "models" / "rttest.gltf",
            .importPath = "rttest",
        });

        getThreadPool().waitIdle();
        vd->commitMeshes();
        WindowCreateInfo mainWindowInfo = {
            .width = 1920,
            .height = 1080,
            .title = "SeeleEngine",
            .preferredFormat = Gfx::SE_FORMAT_B8G8R8A8_SRGB,
        };
        auto window = windowManager->addWindow(graphics, mainWindowInfo);
        ViewportCreateInfo sceneViewInfo = {
            .dimensions =
                {
                    .size = {1920, 1080},
                    .offset = {0, 0},
                },
        };
        OGameView sceneView = new Editor::PlayView(graphics, window, sceneViewInfo, binaryPath);
        sceneView->setFocused();
        // OInspectorView inspectorView = new Editor::InspectorView(graphics, window,
        //                                                          ViewportCreateInfo{
        //                                                              .dimensions =
        //                                                                  {
        //                                                                      .size = {1920, 1080},
        //                                                                      .offset = {0, 0},
        //                                                                  },
        //                                                              .fieldOfView = 0,
        //                                                              .numSamples = Gfx::SE_SAMPLE_COUNT_1_BIT,
        //                                                          });
        //
        window->show();
        while (windowManager->isActive() && getGlobals().running) {
            windowManager->render();
        }
        graphics->waitDeviceIdle();
        Material::destroy();
        vd->destroy();
        // export game

    } else {
        std::filesystem::create_directories(outputPath);
        std::system(fmt::format("cmake -DCMAKE_BUILD_TYPE=Release -DGAME_TITLE=\"{}\" -DGAME_DESTINATION=\"{}\" "
                                "-DGAME_BINARY=\"{}\" -P ./cmake/ExportProject.cmake",
                                gameName, outputPath.generic_string(), binaryPath.generic_string())
                        .c_str());
        std::system(fmt::format("cmake -S {} -B {}", cmakePath.generic_string(), outputPath.generic_string()).c_str());
        std::system(fmt::format("cmake --build {}", cmakePath.generic_string()).c_str());
        std::filesystem::copy(binaryPath, outputPath, std::filesystem::copy_options::recursive);
        std::filesystem::copy(sourcePath / "Assets", outputPath / "Assets", std::filesystem::copy_options::recursive);
        std::filesystem::copy("shaders", outputPath / "shaders", std::filesystem::copy_options::recursive);
        std::filesystem::copy("textures", outputPath / "textures", std::filesystem::copy_options::recursive);
#ifdef WIN32
        std::filesystem::copy_file("assimp-vc143-mt.dll", outputPath / "assimp-vc143-mt.dll");
        std::filesystem::copy_file("slang.dll", outputPath / "slang.dll");
        std::filesystem::copy_file("slang-glslang.dll", outputPath / "slang-glslang.dll");
#endif
    }
    return 0;
}
