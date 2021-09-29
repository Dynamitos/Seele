#include "Window/WindowManager.h"
#include "Scene/Components/PrimitiveComponent.h"
#include "Window/SceneView.h"
#include "Window/InspectorView.h"
#include "Asset/AssetRegistry.h"

using namespace Seele;

int main()
{
	PWindowManager windowManager = new WindowManager();
	WindowCreateInfo mainWindowInfo;
	mainWindowInfo.title = "SeeleEngine";
	mainWindowInfo.width = 1280;
	mainWindowInfo.height = 720;
	mainWindowInfo.bFullscreen = false;
	mainWindowInfo.numSamples = 1;
	mainWindowInfo.pixelFormat = Gfx::SE_FORMAT_B8G8R8A8_UNORM;
	auto window = windowManager->addWindow(mainWindowInfo);
	ViewportCreateInfo sceneViewInfo;
	sceneViewInfo.sizeX = 640;
	sceneViewInfo.sizeY = 720;
	sceneViewInfo.offsetX = 0;
	sceneViewInfo.offsetY = 0;
	PSceneView sceneView = new SceneView(windowManager->getGraphics(), window, sceneViewInfo);
	window->addView(sceneView);
	
	ViewportCreateInfo inspectorViewInfo;
	inspectorViewInfo.sizeX = 640;
	inspectorViewInfo.sizeY = 720;
	inspectorViewInfo.offsetX = 640;
	inspectorViewInfo.offsetY = 0;
	PInspectorView inspectorView = new InspectorView(windowManager->getGraphics(), window, inspectorViewInfo);
	window->addView(inspectorView);
	sceneView->setFocused();
	AssetRegistry::init("D:\\Private\\Programming\\C++\\TestSeeleProject\\");
	AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Ely\\Ely.fbx");
	AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Cube\\cube.obj");
	AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Plane\\plane.fbx");
	PPrimitiveComponent plane = new PrimitiveComponent(AssetRegistry::findMesh("plane"));
	plane->setWorldScale(Vector(100, 100, 100));
	PPrimitiveComponent arissa = new PrimitiveComponent(AssetRegistry::findMesh("Ely"));
	arissa->addWorldTranslation(Vector(0, 0, 100));
	arissa->setWorldScale(Vector(0.1f, 0.1f, 0.1f));
	while (windowManager->isActive())
	{
		windowManager->render();
	}
	return 0;
}