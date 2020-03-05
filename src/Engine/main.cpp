#include "Graphics/RenderCore.h"
using namespace Seele;
int main()
{
	RenderCore core;
	core.init();
	core.renderLoop();
	core.shutdown();
	return 0;
}