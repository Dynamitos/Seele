#include "Graphics/RenderCore.h"
#include "Window/SceneView.h"
#include "Asset/AssetRegistry.h"
#include "Fibers/Fibers.h"
#include <queue>

using namespace Seele;

int main()
{
	RenderCore core;
	core.init();
	WindowCreateInfo mainWindowInfo;
	mainWindowInfo.title = "SeeleEngine";
	mainWindowInfo.width = 1280;
	mainWindowInfo.height = 720;
	mainWindowInfo.bFullscreen = false;
	mainWindowInfo.numSamples = 1;
	mainWindowInfo.pixelFormat = Gfx::SE_FORMAT_B8G8R8A8_UNORM;
	auto window = core.getWindowManager()->addWindow(mainWindowInfo);
	ViewportCreateInfo sceneViewInfo;
	sceneViewInfo.sizeX = 1280;
	sceneViewInfo.sizeY = 720;
	sceneViewInfo.offsetX = 0;
	sceneViewInfo.offsetY = 0;
	PSceneView sceneView = new SceneView(core.getWindowManager()->getGraphics(), window, sceneViewInfo);
	window->addView(sceneView);
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
	sceneView->getScene()->addPrimitiveComponent(plane);
	sceneView->getScene()->addPrimitiveComponent(arissa);
	core.renderLoop();
	core.shutdown();
	return 0;
}