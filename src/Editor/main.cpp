#include "Asset/AssetImporter.h"
#include "Asset/AssetRegistry.h"
#include "Asset/FontLoader.h"
#include "Asset/MaterialLoader.h"
#include "Asset/MeshLoader.h"
#include "Asset/TextureLoader.h"
#include "Graphics/Initializer.h"
#ifdef __APPLE__
#include "Graphics/Metal/Graphics.h"
#else
#include "Graphics/Vulkan/Graphics.h"
#endif
#include "Graphics/StaticMeshVertexData.h"
#include "Window/PlayView.h"
#include "Window/WindowManager.h"
#include <fmt/core.h>
#include <random>

using namespace Seele;
using namespace Seele::Editor;

// make it global so it gets deleted last and automatically
static Gfx::OGraphics graphics;

struct Halfedge {
    uint32 vertexID;
    uint32 nextID;
    uint32 prevID;
    uint32 twinID;
};
Array<Halfedge> generateEdges() {
    Array<UVector> indices = {UVector(0, 1, 4),    UVector(4, 1, 5),    UVector(1, 2, 5),   UVector(5, 2, 6),   UVector(2, 3, 6),
                              UVector(6, 3, 7),    UVector(4, 5, 8),    UVector(8, 5, 9),   UVector(5, 6, 9),   UVector(9, 6, 10),
                              UVector(6, 7, 10),   UVector(10, 7, 11),  UVector(8, 9, 12),  UVector(12, 9, 13), UVector(9, 10, 13),
                              UVector(13, 10, 14), UVector(10, 11, 14), UVector(14, 11, 15)};
    Array<Halfedge> edges;
    for (auto ind : indices) {
        uint32 baseIndex = edges.size();
        edges.add(Halfedge{ind.x, baseIndex+1, baseIndex+2, UINT32_MAX});
        edges.add(Halfedge{ind.y, baseIndex+2, baseIndex, UINT32_MAX});
        edges.add(Halfedge{ind.z, baseIndex, baseIndex+1, UINT32_MAX});
    }
    for (uint32 i = 0; i < edges.size(); ++i) {
        if (edges[i].twinID == UINT32_MAX) {
            for (uint32 j = 0; j < edges.size(); ++j) {
                if (edges[i].vertexID == edges[edges[j].nextID].vertexID && edges[edges[i].nextID].vertexID == edges[j].vertexID) {
                    edges[i].twinID = j;
                    edges[j].twinID = i;
                }
            }
        }
    }
    return edges;
}

int main() {
    std::string gameName = "MeshShadingDemo";
#ifdef WIN32
    std::filesystem::path outputPath = "C:/Users/Dynamitos/MeshShadingDemoGame";
    std::filesystem::path sourcePath = "C:/Users/Dynamitos/MeshShadingDemo";
    std::filesystem::path binaryPath = sourcePath / "bin" / "MeshShadingDemo.dll";
#elif __APPLE__
    std::filesystem::path outputPath = "/Users/dynamitos/MeshShadingDemoGame";
    std::filesystem::path sourcePath = "/Users/dynamitos/MeshShadingDemo";
    std::filesystem::path binaryPath = sourcePath / "cmake" / "libMeshShadingDemo.dylib";
#else
    std::filesystem::path outputPath = "/home/dynamitos/MeshShadingDemoGame";
    std::filesystem::path sourcePath = "/home/dynamitos/MeshShadingDemo";
    std::filesystem::path binaryPath = sourcePath / "cmake" / "libMeshShadingDemo.so";
#endif
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
        // AssetImporter::importFont(FontImportArgs{
        //     .filePath = "./fonts/Calibri.ttf",
        // });
        AssetImporter::importMesh(MeshImportArgs{
            .filePath = sourcePath / "import/models/cube.fbx",
        });
        // AssetImporter::importMesh(MeshImportArgs{
        //    .filePath = sourcePath / "import/models/culling.fbx",
        //});
        AssetImporter::importTexture(TextureImportArgs{
            .filePath = sourcePath / "import/textures/skybox.jpg",
            .type = TextureImportType::TEXTURE_CUBEMAP,
        });
        AssetImporter::importTexture(TextureImportArgs{
            .filePath = sourcePath / "import/textures/brickwall.jpg",
        });
        AssetImporter::importTexture(TextureImportArgs{
            .filePath = sourcePath / "import/textures/brickwall_normal.jpg",
            .type = TextureImportType::TEXTURE_NORMAL,
        });
        // AssetImporter::importMesh(MeshImportArgs{
        //     .filePath = sourcePath / "import/models/ship.fbx",
        //     .importPath = "ship",
        // });
        //AssetImporter::importTexture(TextureImportArgs{
        //    .filePath = sourcePath / "import/textures/azeroth.png",
        //});
        //AssetImporter::importTexture(TextureImportArgs{
        //    .filePath = sourcePath / "import/textures/azeroth_height.png",
        //});
        //AssetImporter::importTexture(TextureImportArgs{
        //    .filePath = sourcePath / "import/textures/wgen.png",
        //});
        AssetImporter::importMesh(MeshImportArgs{
           .filePath = sourcePath / "import/models/after-the-rain-vr-sound/source/Whitechapel.glb",
           .importPath = "Whitechapel",
        });
        //AssetImporter::importMesh(MeshImportArgs{
        //    .filePath = sourcePath / "import/models/plane.obj",
        //    .importPath = "",
        //});
        //  AssetImporter::importMesh(MeshImportArgs{
        //      .filePath = sourcePath / "import/models/city-suburbs/source/city-suburbs.obj",
        //      .importPath = "suburbs",
        //  });
        //  AssetImporter::importMesh(MeshImportArgs{
        //      .filePath = sourcePath / "import/models/minecraft-medieval-city.fbx",
        //      .importPath = "minecraft",
        //  });
        // AssetImporter::importMesh(MeshImportArgs{
        //     .filePath = sourcePath / "import/models/Volvo/Volvo.fbx",
        //     .importPath = "Volvo",
        // });
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
            .numSamples = Gfx::SE_SAMPLE_COUNT_4_BIT,
        };
        OGameView sceneView = new Editor::PlayView(graphics, window, sceneViewInfo, binaryPath.generic_string());
        sceneView->setFocused();

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
