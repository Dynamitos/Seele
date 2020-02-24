#include "WindowManager.h"

Seele::WindowManager::WindowManager(GraphicsInitializer initializer)
{
	//TODO Parse layout file
	WindowCreateInfo mainWindowInfo;
	mainWindowInfo.title = "SeeleEngine";
	mainWindowInfo.width = 1280;
	mainWindowInfo.height = 720;
	Window* mainWindow = new Window(mainWindowInfo);
	windows.add(mainWindow);
}

Seele::WindowManager::~WindowManager()
{
}
