#include "System.h"
#include "Asset/AssetRegistry.h"
#include "Asset/TextureAsset.h"
#include "Elements/Panel.h"
#include "Graphics/Graphics.h"


using namespace Seele;
using namespace Seele::UI;

System::System() {}

System::~System() {}

void System::update() {}

void System::updateViewport(Gfx::PViewport) {
    // TODO set viewport FoV to 0
}

Component::Camera System::getVirtualCamera() const { return virtualCamera; }
