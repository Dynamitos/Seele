#include "Window/WindowManager.h"
#include "Window/SceneView.h"
#include "Window/GameView.h"
#include "Window/InspectorView.h"
#include "Asset/AssetRegistry.h"
#include "Asset/TextureLoader.h"
#include "Graphics/Vulkan/VulkanGraphics.h"

using namespace Seele;
using namespace Seele::Editor;

int main()
{
    Gfx::PGraphics graphics = new Vulkan::Graphics();

    GraphicsInitializer initializer;
    graphics->init(initializer);
    PWindowManager windowManager = new WindowManager();
    AssetRegistry::init(std::string("C:\\Users\\Dynamitos\\TrackClear\\Assets"), graphics);

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
    PGameView sceneView = new GameView(graphics, window, sceneViewInfo);
    
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
    std::string outputPath = "C:\\Users\\Dynamitos\\TrackClearGame";

    return 0;
}