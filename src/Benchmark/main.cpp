#include "Asset/AssetRegistry.h"
#include "Graphics/Initializer.h"
#include "Graphics/StaticMeshVertexData.h"
#ifdef __APPLE__
#include "Graphics/Metal/Graphics.h"
#else
#include "Graphics/Vulkan/Graphics.h"
#endif
#include "PlayView.h"
#include "Window/WindowManager.h"
#include <fmt/core.h>
#include <random>

using namespace Seele;

// make it global so it gets deleted last and automatically
static Gfx::OGraphics graphics;

int main(int argc, char** argv) {
    if (argc > 2)
    {
        return -1;
    }
    bool useDepthCulling = true;
    if (argc == 2 && strcmp(argv[1], "NOCULL") == 0)
    {
        useDepthCulling = false;
    }

    std::filesystem::path binaryPath = "MeshShadingDemo.dll";

#ifdef __APPLE__
        graphics = new Metal::Graphics();
#else
        graphics = new Vulkan::Graphics();
#endif
    GraphicsInitializer initializer;
    graphics->init(initializer);
    StaticMeshVertexData* vd = StaticMeshVertexData::getInstance();
    vd->init(graphics);
    getGlobals().useRayTracing = true;

    OWindowManager windowManager = new WindowManager();
    AssetRegistry::init("Assets", graphics);
    vd->commitMeshes();
    WindowCreateInfo mainWindowInfo = {
        .width = 1920,
        .height = 1080,
        .title = "Benchmark",
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
    OGameView sceneView = new PlayView(graphics, window, sceneViewInfo, binaryPath.generic_string(), useDepthCulling);
    sceneView->setFocused();

    while (windowManager->isActive()) {
        windowManager->render();
    }
    graphics->waitDeviceIdle();
    vd->destroy();

    return 0;
}
