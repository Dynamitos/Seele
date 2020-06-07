#include "Graphics/RenderCore.h"
#include "Asset/AssetRegistry.h"
using namespace Seele;
int main()
{
	RenderCore core;
	core.init();
	AssetRegistry::init("D:\\Private\\Programming\\C++\\TestSeeleProject");
	AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Arissa\\Arissa.fbx");
	core.renderLoop();
	core.shutdown();
	return 0;
}