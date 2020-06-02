#include "Graphics/RenderCore.h"
#include "Asset/AssetRegistry.h"
using namespace Seele;
int main()
{
	RenderCore core;
	core.init();
	AssetRegistry::init("./");
	AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\TestAssets\\Unbenannt.fbx");
	core.renderLoop();
	core.shutdown();
	return 0;
}