#pragma once
#include "Graphics/Enums.h"
#include "Metal/MTLStageInputOutputDescriptor.hpp"
#include "Resources.h"

namespace Seele {
namespace Metal {
constexpr uint64 METAL_VERTEXBUFFER_OFFSET = 6; // something about metal vertex fetch
constexpr uint64 METAL_VERTEXATTRIBUTE_OFFSET = 11;

MTL::PixelFormat cast(Gfx::SeFormat format);
Gfx::SeFormat cast(MTL::PixelFormat format);
MTL::LoadAction cast(Gfx::SeAttachmentLoadOp loadOp);
Gfx::SeAttachmentLoadOp cast(MTL::LoadAction loadOp);
MTL::StoreAction cast(Gfx::SeAttachmentStoreOp storeOp);
Gfx::SeAttachmentStoreOp cast(MTL::StoreAction storeOp);
MTL::SamplerBorderColor cast(Gfx::SeBorderColor color);
Gfx::SeBorderColor cast(MTL::SamplerBorderColor color);
MTL::CompareFunction cast(Gfx::SeCompareOp compare);
Gfx::SeCompareOp cast(MTL::CompareFunction compare);
MTL::SamplerMinMagFilter cast(Gfx::SeFilter filter);
Gfx::SeFilter cast(MTL::SamplerMinMagFilter filter);
MTL::SamplerMipFilter cast(Gfx::SeSamplerMipmapMode filter);
Gfx::SeSamplerMipmapMode cast(MTL::SamplerMipFilter filter);
MTL::SamplerAddressMode cast(Gfx::SeSamplerAddressMode mode);
Gfx::SeSamplerAddressMode cast(MTL::SamplerAddressMode mode);
MTL::PrimitiveTopologyClass cast(Gfx::SePrimitiveTopology topology);
Gfx::SePrimitiveTopology cast(MTL::PrimitiveTopologyClass topology);
MTL::IndexType cast(Gfx::SeIndexType indexType);
Gfx::SeIndexType cast(MTL::IndexType indexType);
} // namespace Metal
} // namespace Seele
