#pragma once
#include "Graphics/Enums.h"
#include "Resources.h"

namespace Seele
{
namespace Metal
{
MTL::PixelFormat cast(Gfx::SeFormat format);
Gfx::SeFormat cast(MTL::PixelFormat format);
MTL::LoadAction cast(Gfx::SeAttachmentLoadOp loadOp);
Gfx::SeAttachmentLoadOp cast(MTL::LoadAction loadOp);
MTL::StoreAction cast(Gfx::SeAttachmentStoreOp storeOp);
Gfx::SeAttachmentStoreOp cast(MTL::StoreAction storeOp);
}
}