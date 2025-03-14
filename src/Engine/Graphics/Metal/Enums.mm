#include "Enums.h"
#include "Graphics/Enums.h"
#include "Metal/MTLStageInputOutputDescriptor.hpp"
#include <stdexcept>

using namespace Seele;
using namespace Seele::Metal;

MTL::PixelFormat Seele::Metal::cast(Gfx::SeFormat format) {
    switch (format) {
    case Gfx::SE_FORMAT_UNDEFINED:
        return MTL::PixelFormatInvalid;
    case Gfx::SE_FORMAT_R8_UNORM:
        return MTL::PixelFormatR8Unorm;
    case Gfx::SE_FORMAT_R8_SRGB:
        return MTL::PixelFormatR8Unorm_sRGB;
    case Gfx::SE_FORMAT_R8_SNORM:
        return MTL::PixelFormatR8Snorm;
    case Gfx::SE_FORMAT_R8_UINT:
        return MTL::PixelFormatR8Uint;
    case Gfx::SE_FORMAT_R8_SINT:
        return MTL::PixelFormatR8Sint;
    case Gfx::SE_FORMAT_R16_UNORM:
        return MTL::PixelFormatR16Unorm;
    case Gfx::SE_FORMAT_R16_SNORM:
        return MTL::PixelFormatR16Snorm;
    case Gfx::SE_FORMAT_R16_UINT:
        return MTL::PixelFormatR16Uint;
    case Gfx::SE_FORMAT_R16_SINT:
        return MTL::PixelFormatR16Sint;
    case Gfx::SE_FORMAT_R16_SFLOAT:
        return MTL::PixelFormatR16Float;
    case Gfx::SE_FORMAT_R8G8_UNORM:
        return MTL::PixelFormatRG8Unorm;
    case Gfx::SE_FORMAT_R8G8_SRGB:
        return MTL::PixelFormatRG8Unorm_sRGB;
    case Gfx::SE_FORMAT_R8G8_SNORM:
        return MTL::PixelFormatRG8Snorm;
    case Gfx::SE_FORMAT_R8G8_UINT:
        return MTL::PixelFormatRG8Uint;
    case Gfx::SE_FORMAT_R8G8_SINT:
        return MTL::PixelFormatRG8Sint;
    case Gfx::SE_FORMAT_B5G6R5_UNORM_PACK16:
        return MTL::PixelFormatB5G6R5Unorm;
    case Gfx::SE_FORMAT_R4G4B4A4_UNORM_PACK16:
        return MTL::PixelFormatABGR4Unorm;
    case Gfx::SE_FORMAT_B5G5R5A1_UNORM_PACK16:
        return MTL::PixelFormatBGR5A1Unorm;
    case Gfx::SE_FORMAT_R32_UINT:
        return MTL::PixelFormatR32Uint;
    case Gfx::SE_FORMAT_R32_SINT:
        return MTL::PixelFormatR32Sint;
    case Gfx::SE_FORMAT_R32_SFLOAT:
        return MTL::PixelFormatR32Float;
    case Gfx::SE_FORMAT_R16G16_UNORM:
        return MTL::PixelFormatRG16Unorm;
    case Gfx::SE_FORMAT_R16G16_SNORM:
        return MTL::PixelFormatRG16Snorm;
    case Gfx::SE_FORMAT_R16G16_UINT:
        return MTL::PixelFormatRG16Uint;
    case Gfx::SE_FORMAT_R16G16_SINT:
        return MTL::PixelFormatRG16Sint;
    case Gfx::SE_FORMAT_R16G16_SFLOAT:
        return MTL::PixelFormatRG16Float;
    case Gfx::SE_FORMAT_R8G8B8A8_UNORM:
        return MTL::PixelFormatRGBA8Unorm;
    case Gfx::SE_FORMAT_R8G8B8A8_SRGB:
        return MTL::PixelFormatRGBA8Unorm_sRGB;
    case Gfx::SE_FORMAT_R8G8B8A8_SNORM:
        return MTL::PixelFormatRGBA8Snorm;
    case Gfx::SE_FORMAT_R8G8B8A8_UINT:
        return MTL::PixelFormatRGBA8Uint;
    case Gfx::SE_FORMAT_R8G8B8A8_SINT:
        return MTL::PixelFormatRGBA8Sint;
    case Gfx::SE_FORMAT_B8G8R8A8_UNORM:
        return MTL::PixelFormatBGRA8Unorm;
    case Gfx::SE_FORMAT_B8G8R8A8_SRGB:
        return MTL::PixelFormatBGRA8Unorm_sRGB;
    case Gfx::SE_FORMAT_R32G32_UINT:
        return MTL::PixelFormatRG32Uint;
    case Gfx::SE_FORMAT_R32G32_SINT:
        return MTL::PixelFormatRG32Sint;
    case Gfx::SE_FORMAT_R32G32_SFLOAT:
        return MTL::PixelFormatRG32Float;
    case Gfx::SE_FORMAT_R16G16B16A16_UNORM:
        return MTL::PixelFormatRGBA16Unorm;
    case Gfx::SE_FORMAT_R16G16B16A16_SNORM:
        return MTL::PixelFormatRGBA16Snorm;
    case Gfx::SE_FORMAT_R16G16B16A16_UINT:
        return MTL::PixelFormatRGBA16Uint;
    case Gfx::SE_FORMAT_R16G16B16A16_SINT:
        return MTL::PixelFormatRGBA16Sint;
    case Gfx::SE_FORMAT_R16G16B16A16_SFLOAT:
        return MTL::PixelFormatRGBA16Float;
    case Gfx::SE_FORMAT_R32G32B32A32_UINT:
        return MTL::PixelFormatRGBA32Uint;
    case Gfx::SE_FORMAT_R32G32B32A32_SINT:
        return MTL::PixelFormatRGBA32Sint;
    case Gfx::SE_FORMAT_R32G32B32A32_SFLOAT:
        return MTL::PixelFormatRGBA32Float;
    case Gfx::SE_FORMAT_BC1_RGBA_UNORM_BLOCK:
        return MTL::PixelFormatBC1_RGBA;
    case Gfx::SE_FORMAT_BC1_RGBA_SRGB_BLOCK:
        return MTL::PixelFormatBC1_RGBA_sRGB;
    case Gfx::SE_FORMAT_BC2_UNORM_BLOCK:
        return MTL::PixelFormatBC2_RGBA;
    case Gfx::SE_FORMAT_BC2_SRGB_BLOCK:
        return MTL::PixelFormatBC2_RGBA_sRGB;
    case Gfx::SE_FORMAT_BC3_UNORM_BLOCK:
        return MTL::PixelFormatBC3_RGBA;
    case Gfx::SE_FORMAT_BC3_SRGB_BLOCK:
        return MTL::PixelFormatBC3_RGBA_sRGB;
    case Gfx::SE_FORMAT_BC4_UNORM_BLOCK:
        return MTL::PixelFormatBC4_RUnorm;
    case Gfx::SE_FORMAT_BC4_SNORM_BLOCK:
        return MTL::PixelFormatBC4_RSnorm;
    case Gfx::SE_FORMAT_BC5_UNORM_BLOCK:
        return MTL::PixelFormatBC5_RGUnorm;
    case Gfx::SE_FORMAT_BC5_SNORM_BLOCK:
        return MTL::PixelFormatBC5_RGSnorm;
    case Gfx::SE_FORMAT_BC6H_SFLOAT_BLOCK:
        return MTL::PixelFormatBC6H_RGBFloat;
    case Gfx::SE_FORMAT_BC6H_UFLOAT_BLOCK:
        return MTL::PixelFormatBC6H_RGBUfloat;
    case Gfx::SE_FORMAT_BC7_UNORM_BLOCK:
        return MTL::PixelFormatBC7_RGBAUnorm;
    case Gfx::SE_FORMAT_BC7_SRGB_BLOCK:
        return MTL::PixelFormatBC7_RGBAUnorm_sRGB;
    case Gfx::SE_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
        return MTL::PixelFormatPVRTC_RGB_2BPP;
    case Gfx::SE_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
        return MTL::PixelFormatPVRTC_RGB_2BPP_sRGB;
    case Gfx::SE_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
        return MTL::PixelFormatPVRTC_RGB_4BPP;
    case Gfx::SE_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
        return MTL::PixelFormatPVRTC_RGB_4BPP_sRGB;
    case Gfx::SE_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
        return MTL::PixelFormatPVRTC_RGBA_2BPP;
    case Gfx::SE_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
        return MTL::PixelFormatPVRTC_RGBA_2BPP_sRGB;
    case Gfx::SE_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
        return MTL::PixelFormatPVRTC_RGBA_4BPP;
    case Gfx::SE_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
        return MTL::PixelFormatPVRTC_RGBA_4BPP_sRGB;
    case Gfx::SE_FORMAT_EAC_R11_UNORM_BLOCK:
        return MTL::PixelFormatEAC_R11Unorm;
    case Gfx::SE_FORMAT_EAC_R11_SNORM_BLOCK:
        return MTL::PixelFormatEAC_R11Snorm;
    case Gfx::SE_FORMAT_EAC_R11G11_UNORM_BLOCK:
        return MTL::PixelFormatEAC_RG11Unorm;
    case Gfx::SE_FORMAT_EAC_R11G11_SNORM_BLOCK:
        return MTL::PixelFormatEAC_RG11Snorm;
    case Gfx::SE_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        return MTL::PixelFormatETC2_RGB8;
    case Gfx::SE_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        return MTL::PixelFormatETC2_RGB8_sRGB;
    case Gfx::SE_FORMAT_ASTC_4x4_SRGB_BLOCK:
        return MTL::PixelFormatASTC_4x4_sRGB;
    case Gfx::SE_FORMAT_ASTC_5x4_SRGB_BLOCK:
        return MTL::PixelFormatASTC_5x4_sRGB;
    case Gfx::SE_FORMAT_ASTC_5x5_SRGB_BLOCK:
        return MTL::PixelFormatASTC_5x5_sRGB;
    case Gfx::SE_FORMAT_ASTC_6x5_SRGB_BLOCK:
        return MTL::PixelFormatASTC_6x5_sRGB;
    case Gfx::SE_FORMAT_ASTC_6x6_SRGB_BLOCK:
        return MTL::PixelFormatASTC_6x6_sRGB;
    case Gfx::SE_FORMAT_ASTC_8x5_SRGB_BLOCK:
        return MTL::PixelFormatASTC_8x5_sRGB;
    case Gfx::SE_FORMAT_ASTC_8x6_SRGB_BLOCK:
        return MTL::PixelFormatASTC_8x6_sRGB;
    case Gfx::SE_FORMAT_ASTC_8x8_SRGB_BLOCK:
        return MTL::PixelFormatASTC_8x8_sRGB;
    case Gfx::SE_FORMAT_ASTC_10x5_SRGB_BLOCK:
        return MTL::PixelFormatASTC_10x5_sRGB;
    case Gfx::SE_FORMAT_ASTC_10x6_SRGB_BLOCK:
        return MTL::PixelFormatASTC_10x6_sRGB;
    case Gfx::SE_FORMAT_ASTC_10x8_SRGB_BLOCK:
        return MTL::PixelFormatASTC_10x8_sRGB;
    case Gfx::SE_FORMAT_ASTC_10x10_SRGB_BLOCK:
        return MTL::PixelFormatASTC_10x10_sRGB;
    case Gfx::SE_FORMAT_ASTC_12x10_SRGB_BLOCK:
        return MTL::PixelFormatASTC_12x10_sRGB;
    case Gfx::SE_FORMAT_ASTC_12x12_SRGB_BLOCK:
        return MTL::PixelFormatASTC_12x12_sRGB;
    case Gfx::SE_FORMAT_ASTC_4x4_UNORM_BLOCK:
        return MTL::PixelFormatASTC_4x4_LDR;
    case Gfx::SE_FORMAT_ASTC_5x4_UNORM_BLOCK:
        return MTL::PixelFormatASTC_5x4_LDR;
    case Gfx::SE_FORMAT_ASTC_5x5_UNORM_BLOCK:
        return MTL::PixelFormatASTC_5x5_LDR;
    case Gfx::SE_FORMAT_ASTC_6x5_UNORM_BLOCK:
        return MTL::PixelFormatASTC_6x5_LDR;
    case Gfx::SE_FORMAT_ASTC_6x6_UNORM_BLOCK:
        return MTL::PixelFormatASTC_6x6_LDR;
    case Gfx::SE_FORMAT_ASTC_8x5_UNORM_BLOCK:
        return MTL::PixelFormatASTC_8x5_LDR;
    case Gfx::SE_FORMAT_ASTC_8x6_UNORM_BLOCK:
        return MTL::PixelFormatASTC_8x6_LDR;
    case Gfx::SE_FORMAT_ASTC_8x8_UNORM_BLOCK:
        return MTL::PixelFormatASTC_8x8_LDR;
    case Gfx::SE_FORMAT_ASTC_10x5_UNORM_BLOCK:
        return MTL::PixelFormatASTC_10x5_LDR;
    case Gfx::SE_FORMAT_ASTC_10x6_UNORM_BLOCK:
        return MTL::PixelFormatASTC_10x6_LDR;
    case Gfx::SE_FORMAT_ASTC_10x8_UNORM_BLOCK:
        return MTL::PixelFormatASTC_10x8_LDR;
    case Gfx::SE_FORMAT_ASTC_10x10_UNORM_BLOCK:
        return MTL::PixelFormatASTC_10x10_LDR;
    case Gfx::SE_FORMAT_ASTC_12x10_UNORM_BLOCK:
        return MTL::PixelFormatASTC_12x10_LDR;
    case Gfx::SE_FORMAT_ASTC_12x12_UNORM_BLOCK:
        return MTL::PixelFormatASTC_12x12_LDR;
    case Gfx::SE_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_4x4_HDR;
    case Gfx::SE_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_5x4_HDR;
    case Gfx::SE_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_5x5_HDR;
    case Gfx::SE_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_6x5_HDR;
    case Gfx::SE_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_6x6_HDR;
    case Gfx::SE_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_8x5_HDR;
    case Gfx::SE_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_8x6_HDR;
    case Gfx::SE_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_8x8_HDR;
    case Gfx::SE_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_10x5_HDR;
    case Gfx::SE_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_10x6_HDR;
    case Gfx::SE_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_10x8_HDR;
    case Gfx::SE_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_10x10_HDR;
    case Gfx::SE_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_12x10_HDR;
    case Gfx::SE_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
        return MTL::PixelFormatASTC_12x12_HDR;
    case Gfx::SE_FORMAT_G8B8G8R8_422_UNORM:
        return MTL::PixelFormatGBGR422;
    case Gfx::SE_FORMAT_B8G8R8G8_422_UNORM:
        return MTL::PixelFormatBGRG422;
    case Gfx::SE_FORMAT_D16_UNORM:
        return MTL::PixelFormatDepth16Unorm;
    case Gfx::SE_FORMAT_D32_SFLOAT:
        return MTL::PixelFormatDepth32Float;
    case Gfx::SE_FORMAT_S8_UINT:
        return MTL::PixelFormatStencil8;
    case Gfx::SE_FORMAT_D24_UNORM_S8_UINT:
        return MTL::PixelFormatDepth24Unorm_Stencil8;
    case Gfx::SE_FORMAT_D32_SFLOAT_S8_UINT:
        return MTL::PixelFormatDepth32Float_Stencil8;
    default:
        throw std::logic_error("Not implemented");
    }
}
Gfx::SeFormat Seele::Metal::cast(MTL::PixelFormat format) {
    switch (format) {
    case MTL::PixelFormatInvalid:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatA8Unorm:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatR8Unorm:
        return Gfx::SE_FORMAT_R8_UNORM;
    case MTL::PixelFormatR8Unorm_sRGB:
        return Gfx::SE_FORMAT_R8_SRGB;
    case MTL::PixelFormatR8Snorm:
        return Gfx::SE_FORMAT_R8_SNORM;
    case MTL::PixelFormatR8Uint:
        return Gfx::SE_FORMAT_R8_UINT;
    case MTL::PixelFormatR8Sint:
        return Gfx::SE_FORMAT_R8_SINT;
    case MTL::PixelFormatR16Unorm:
        return Gfx::SE_FORMAT_R16_UNORM;
    case MTL::PixelFormatR16Snorm:
        return Gfx::SE_FORMAT_R16_SNORM;
    case MTL::PixelFormatR16Uint:
        return Gfx::SE_FORMAT_R16_UINT;
    case MTL::PixelFormatR16Sint:
        return Gfx::SE_FORMAT_R16_SINT;
    case MTL::PixelFormatR16Float:
        return Gfx::SE_FORMAT_R16_SFLOAT;
    case MTL::PixelFormatRG8Unorm:
        return Gfx::SE_FORMAT_R8G8_UNORM;
    case MTL::PixelFormatRG8Unorm_sRGB:
        return Gfx::SE_FORMAT_R8G8_SRGB;
    case MTL::PixelFormatRG8Snorm:
        return Gfx::SE_FORMAT_R8G8_SNORM;
    case MTL::PixelFormatRG8Uint:
        return Gfx::SE_FORMAT_R8G8_UINT;
    case MTL::PixelFormatRG8Sint:
        return Gfx::SE_FORMAT_R8G8_SINT;
    case MTL::PixelFormatB5G6R5Unorm:
        return Gfx::SE_FORMAT_B5G6R5_UNORM_PACK16;
    case MTL::PixelFormatA1BGR5Unorm:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatABGR4Unorm:
        return Gfx::SE_FORMAT_R4G4B4A4_UNORM_PACK16;
    case MTL::PixelFormatBGR5A1Unorm:
        return Gfx::SE_FORMAT_B5G5R5A1_UNORM_PACK16;
    case MTL::PixelFormatR32Uint:
        return Gfx::SE_FORMAT_R32_UINT;
    case MTL::PixelFormatR32Sint:
        return Gfx::SE_FORMAT_R32_SINT;
    case MTL::PixelFormatR32Float:
        return Gfx::SE_FORMAT_R32_SFLOAT;
    case MTL::PixelFormatRG16Unorm:
        return Gfx::SE_FORMAT_R16G16_UNORM;
    case MTL::PixelFormatRG16Snorm:
        return Gfx::SE_FORMAT_R16G16_SNORM;
    case MTL::PixelFormatRG16Uint:
        return Gfx::SE_FORMAT_R16G16_UINT;
    case MTL::PixelFormatRG16Sint:
        return Gfx::SE_FORMAT_R16G16_SINT;
    case MTL::PixelFormatRG16Float:
        return Gfx::SE_FORMAT_R16G16_SFLOAT;
    case MTL::PixelFormatRGBA8Unorm:
        return Gfx::SE_FORMAT_R8G8B8A8_UNORM;
    case MTL::PixelFormatRGBA8Unorm_sRGB:
        return Gfx::SE_FORMAT_R8G8B8A8_SRGB;
    case MTL::PixelFormatRGBA8Snorm:
        return Gfx::SE_FORMAT_R8G8B8A8_SNORM;
    case MTL::PixelFormatRGBA8Uint:
        return Gfx::SE_FORMAT_R8G8B8A8_UINT;
    case MTL::PixelFormatRGBA8Sint:
        return Gfx::SE_FORMAT_R8G8B8A8_SINT;
    case MTL::PixelFormatBGRA8Unorm:
        return Gfx::SE_FORMAT_B8G8R8A8_UNORM;
    case MTL::PixelFormatBGRA8Unorm_sRGB:
        return Gfx::SE_FORMAT_B8G8R8A8_SRGB;
    case MTL::PixelFormatRGB10A2Unorm:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatRGB10A2Uint:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatRG11B10Float:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatRGB9E5Float:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatBGR10A2Unorm:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatBGR10_XR:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatBGR10_XR_sRGB:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatRG32Uint:
        return Gfx::SE_FORMAT_R32G32_UINT;
    case MTL::PixelFormatRG32Sint:
        return Gfx::SE_FORMAT_R32G32_SINT;
    case MTL::PixelFormatRG32Float:
        return Gfx::SE_FORMAT_R32G32_SFLOAT;
    case MTL::PixelFormatRGBA16Unorm:
        return Gfx::SE_FORMAT_R16G16B16A16_UNORM;
    case MTL::PixelFormatRGBA16Snorm:
        return Gfx::SE_FORMAT_R16G16B16A16_SNORM;
    case MTL::PixelFormatRGBA16Uint:
        return Gfx::SE_FORMAT_R16G16B16A16_UINT;
    case MTL::PixelFormatRGBA16Sint:
        return Gfx::SE_FORMAT_R16G16B16A16_SINT;
    case MTL::PixelFormatRGBA16Float:
        return Gfx::SE_FORMAT_R16G16B16A16_SFLOAT;
    case MTL::PixelFormatBGRA10_XR:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatBGRA10_XR_sRGB:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatRGBA32Uint:
        return Gfx::SE_FORMAT_R32G32B32A32_UINT;
    case MTL::PixelFormatRGBA32Sint:
        return Gfx::SE_FORMAT_R32G32B32A32_SINT;
    case MTL::PixelFormatRGBA32Float:
        return Gfx::SE_FORMAT_R32G32B32A32_SFLOAT;
    case MTL::PixelFormatBC1_RGBA:
        return Gfx::SE_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case MTL::PixelFormatBC1_RGBA_sRGB:
        return Gfx::SE_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case MTL::PixelFormatBC2_RGBA:
        return Gfx::SE_FORMAT_BC2_UNORM_BLOCK;
    case MTL::PixelFormatBC2_RGBA_sRGB:
        return Gfx::SE_FORMAT_BC2_SRGB_BLOCK;
    case MTL::PixelFormatBC3_RGBA:
        return Gfx::SE_FORMAT_BC3_UNORM_BLOCK;
    case MTL::PixelFormatBC3_RGBA_sRGB:
        return Gfx::SE_FORMAT_BC3_SRGB_BLOCK;
    case MTL::PixelFormatBC4_RUnorm:
        return Gfx::SE_FORMAT_BC4_UNORM_BLOCK;
    case MTL::PixelFormatBC4_RSnorm:
        return Gfx::SE_FORMAT_BC4_SNORM_BLOCK;
    case MTL::PixelFormatBC5_RGUnorm:
        return Gfx::SE_FORMAT_BC4_SNORM_BLOCK;
    case MTL::PixelFormatBC5_RGSnorm:
        return Gfx::SE_FORMAT_BC5_SNORM_BLOCK;
    case MTL::PixelFormatBC6H_RGBFloat:
        return Gfx::SE_FORMAT_BC6H_SFLOAT_BLOCK;
    case MTL::PixelFormatBC6H_RGBUfloat:
        return Gfx::SE_FORMAT_BC6H_UFLOAT_BLOCK;
    case MTL::PixelFormatBC7_RGBAUnorm:
        return Gfx::SE_FORMAT_BC7_UNORM_BLOCK;
    case MTL::PixelFormatBC7_RGBAUnorm_sRGB:
        return Gfx::SE_FORMAT_BC7_SRGB_BLOCK;
    case MTL::PixelFormatPVRTC_RGB_2BPP:
        return Gfx::SE_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
    case MTL::PixelFormatPVRTC_RGB_2BPP_sRGB:
        return Gfx::SE_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
    case MTL::PixelFormatPVRTC_RGB_4BPP:
        return Gfx::SE_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
    case MTL::PixelFormatPVRTC_RGB_4BPP_sRGB:
        return Gfx::SE_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
    case MTL::PixelFormatPVRTC_RGBA_2BPP:
        return Gfx::SE_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
    case MTL::PixelFormatPVRTC_RGBA_2BPP_sRGB:
        return Gfx::SE_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;
    case MTL::PixelFormatPVRTC_RGBA_4BPP:
        return Gfx::SE_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
    case MTL::PixelFormatPVRTC_RGBA_4BPP_sRGB:
        return Gfx::SE_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;
    case MTL::PixelFormatEAC_R11Unorm:
        return Gfx::SE_FORMAT_EAC_R11_UNORM_BLOCK;
    case MTL::PixelFormatEAC_R11Snorm:
        return Gfx::SE_FORMAT_EAC_R11_SNORM_BLOCK;
    case MTL::PixelFormatEAC_RG11Unorm:
        return Gfx::SE_FORMAT_EAC_R11G11_UNORM_BLOCK;
    case MTL::PixelFormatEAC_RG11Snorm:
        return Gfx::SE_FORMAT_EAC_R11G11_SNORM_BLOCK;
    case MTL::PixelFormatEAC_RGBA8:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatEAC_RGBA8_sRGB:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatETC2_RGB8:
        return Gfx::SE_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case MTL::PixelFormatETC2_RGB8_sRGB:
        return Gfx::SE_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
    case MTL::PixelFormatETC2_RGB8A1:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatETC2_RGB8A1_sRGB:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatASTC_4x4_sRGB:
        return Gfx::SE_FORMAT_ASTC_4x4_SRGB_BLOCK;
    case MTL::PixelFormatASTC_5x4_sRGB:
        return Gfx::SE_FORMAT_ASTC_5x4_SRGB_BLOCK;
    case MTL::PixelFormatASTC_5x5_sRGB:
        return Gfx::SE_FORMAT_ASTC_5x5_SRGB_BLOCK;
    case MTL::PixelFormatASTC_6x5_sRGB:
        return Gfx::SE_FORMAT_ASTC_6x5_SRGB_BLOCK;
    case MTL::PixelFormatASTC_6x6_sRGB:
        return Gfx::SE_FORMAT_ASTC_6x6_SRGB_BLOCK;
    case MTL::PixelFormatASTC_8x5_sRGB:
        return Gfx::SE_FORMAT_ASTC_8x5_SRGB_BLOCK;
    case MTL::PixelFormatASTC_8x6_sRGB:
        return Gfx::SE_FORMAT_ASTC_8x6_SRGB_BLOCK;
    case MTL::PixelFormatASTC_8x8_sRGB:
        return Gfx::SE_FORMAT_ASTC_8x8_SRGB_BLOCK;
    case MTL::PixelFormatASTC_10x5_sRGB:
        return Gfx::SE_FORMAT_ASTC_10x5_SRGB_BLOCK;
    case MTL::PixelFormatASTC_10x6_sRGB:
        return Gfx::SE_FORMAT_ASTC_10x6_SRGB_BLOCK;
    case MTL::PixelFormatASTC_10x8_sRGB:
        return Gfx::SE_FORMAT_ASTC_10x8_SRGB_BLOCK;
    case MTL::PixelFormatASTC_10x10_sRGB:
        return Gfx::SE_FORMAT_ASTC_10x10_SRGB_BLOCK;
    case MTL::PixelFormatASTC_12x10_sRGB:
        return Gfx::SE_FORMAT_ASTC_12x10_SRGB_BLOCK;
    case MTL::PixelFormatASTC_12x12_sRGB:
        return Gfx::SE_FORMAT_ASTC_12x12_SRGB_BLOCK;
    case MTL::PixelFormatASTC_4x4_LDR:
        return Gfx::SE_FORMAT_ASTC_4x4_UNORM_BLOCK;
    case MTL::PixelFormatASTC_5x4_LDR:
        return Gfx::SE_FORMAT_ASTC_5x4_UNORM_BLOCK;
    case MTL::PixelFormatASTC_5x5_LDR:
        return Gfx::SE_FORMAT_ASTC_5x5_UNORM_BLOCK;
    case MTL::PixelFormatASTC_6x5_LDR:
        return Gfx::SE_FORMAT_ASTC_6x5_UNORM_BLOCK;
    case MTL::PixelFormatASTC_6x6_LDR:
        return Gfx::SE_FORMAT_ASTC_6x6_UNORM_BLOCK;
    case MTL::PixelFormatASTC_8x5_LDR:
        return Gfx::SE_FORMAT_ASTC_8x5_UNORM_BLOCK;
    case MTL::PixelFormatASTC_8x6_LDR:
        return Gfx::SE_FORMAT_ASTC_8x6_UNORM_BLOCK;
    case MTL::PixelFormatASTC_8x8_LDR:
        return Gfx::SE_FORMAT_ASTC_8x8_UNORM_BLOCK;
    case MTL::PixelFormatASTC_10x5_LDR:
        return Gfx::SE_FORMAT_ASTC_10x5_UNORM_BLOCK;
    case MTL::PixelFormatASTC_10x6_LDR:
        return Gfx::SE_FORMAT_ASTC_10x6_UNORM_BLOCK;
    case MTL::PixelFormatASTC_10x8_LDR:
        return Gfx::SE_FORMAT_ASTC_10x8_UNORM_BLOCK;
    case MTL::PixelFormatASTC_10x10_LDR:
        return Gfx::SE_FORMAT_ASTC_10x10_UNORM_BLOCK;
    case MTL::PixelFormatASTC_12x10_LDR:
        return Gfx::SE_FORMAT_ASTC_12x10_UNORM_BLOCK;
    case MTL::PixelFormatASTC_12x12_LDR:
        return Gfx::SE_FORMAT_ASTC_12x12_UNORM_BLOCK;
    case MTL::PixelFormatASTC_4x4_HDR:
        return Gfx::SE_FORMAT_ASTC_4x4_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_5x4_HDR:
        return Gfx::SE_FORMAT_ASTC_5x4_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_5x5_HDR:
        return Gfx::SE_FORMAT_ASTC_5x5_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_6x5_HDR:
        return Gfx::SE_FORMAT_ASTC_6x5_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_6x6_HDR:
        return Gfx::SE_FORMAT_ASTC_6x6_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_8x5_HDR:
        return Gfx::SE_FORMAT_ASTC_8x5_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_8x6_HDR:
        return Gfx::SE_FORMAT_ASTC_8x6_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_8x8_HDR:
        return Gfx::SE_FORMAT_ASTC_8x8_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_10x5_HDR:
        return Gfx::SE_FORMAT_ASTC_10x5_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_10x6_HDR:
        return Gfx::SE_FORMAT_ASTC_10x6_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_10x8_HDR:
        return Gfx::SE_FORMAT_ASTC_10x8_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_10x10_HDR:
        return Gfx::SE_FORMAT_ASTC_10x10_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_12x10_HDR:
        return Gfx::SE_FORMAT_ASTC_12x10_SFLOAT_BLOCK;
    case MTL::PixelFormatASTC_12x12_HDR:
        return Gfx::SE_FORMAT_ASTC_12x12_SFLOAT_BLOCK;
    case MTL::PixelFormatGBGR422:
        return Gfx::SE_FORMAT_G8B8G8R8_422_UNORM;
    case MTL::PixelFormatBGRG422:
        return Gfx::SE_FORMAT_B8G8R8G8_422_UNORM;
    case MTL::PixelFormatDepth16Unorm:
        return Gfx::SE_FORMAT_D16_UNORM;
    case MTL::PixelFormatDepth32Float:
        return Gfx::SE_FORMAT_D32_SFLOAT;
    case MTL::PixelFormatStencil8:
        return Gfx::SE_FORMAT_S8_UINT;
    case MTL::PixelFormatDepth24Unorm_Stencil8:
        return Gfx::SE_FORMAT_D24_UNORM_S8_UINT;
    case MTL::PixelFormatDepth32Float_Stencil8:
        return Gfx::SE_FORMAT_D32_SFLOAT_S8_UINT;
    case MTL::PixelFormatX32_Stencil8:
        return Gfx::SE_FORMAT_UNDEFINED;
    case MTL::PixelFormatX24_Stencil8:
        return Gfx::SE_FORMAT_UNDEFINED;
    default:
        throw std::logic_error("Not implemented");
    }
}

MTL::LoadAction Seele::Metal::cast(Gfx::SeAttachmentLoadOp loadOp) {
    switch (loadOp) {
    case Seele::Gfx::SE_ATTACHMENT_LOAD_OP_LOAD:
        return MTL::LoadActionLoad;
    case Seele::Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR:
        return MTL::LoadActionClear;
    case Seele::Gfx::SE_ATTACHMENT_LOAD_OP_DONT_CARE:
        return MTL::LoadActionDontCare;
    default:
        throw std::logic_error("Not implemented");
    }
}

Gfx::SeAttachmentLoadOp Seele::Metal::cast(MTL::LoadAction loadOp) {
    switch (loadOp) {
    case MTL::LoadActionDontCare:
        return Gfx::SE_ATTACHMENT_LOAD_OP_DONT_CARE;
    case MTL::LoadActionLoad:
        return Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    case MTL::LoadActionClear:
        return Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR;
    default:
        throw std::logic_error("Not implemented");
    }
}

MTL::StoreAction Seele::Metal::cast(Gfx::SeAttachmentStoreOp storeOp) {
    switch (storeOp) {
    case Seele::Gfx::SE_ATTACHMENT_STORE_OP_STORE:
        return MTL::StoreActionStore;
    case Seele::Gfx::SE_ATTACHMENT_STORE_OP_DONT_CARE:
        return MTL::StoreActionDontCare;
    default:
        throw std::logic_error("Not implemented");
    }
}

Gfx::SeAttachmentStoreOp Seele::Metal::cast(MTL::StoreAction storeOp) {
    switch (storeOp) {
    case MTL::StoreActionDontCare:
        return Gfx::SE_ATTACHMENT_STORE_OP_DONT_CARE;
    case MTL::StoreActionStore:
        return Gfx::SE_ATTACHMENT_STORE_OP_STORE;
    default:
        throw std::logic_error("Not implemented");
    }
}

MTL::SamplerBorderColor Seele::Metal::cast(Gfx::SeBorderColor color) {
    switch (color) {
    case Gfx::SE_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
        return MTL::SamplerBorderColorTransparentBlack;
    case Gfx::SE_BORDER_COLOR_INT_TRANSPARENT_BLACK:
        return MTL::SamplerBorderColorTransparentBlack;
    case Gfx::SE_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
        return MTL::SamplerBorderColorOpaqueBlack;
    case Gfx::SE_BORDER_COLOR_INT_OPAQUE_BLACK:
        return MTL::SamplerBorderColorOpaqueBlack;
    case Gfx::SE_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
        return MTL::SamplerBorderColorOpaqueWhite;
    case Gfx::SE_BORDER_COLOR_INT_OPAQUE_WHITE:
        return MTL::SamplerBorderColorOpaqueWhite;
    default:
        throw std::logic_error("Not implemented");
    }
}

Gfx::SeBorderColor Seele::Metal::cast(MTL::SamplerBorderColor color) {
    switch (color) {
    case MTL::SamplerBorderColorTransparentBlack:
        return Gfx::SE_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    case MTL::SamplerBorderColorOpaqueBlack:
        return Gfx::SE_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    case MTL::SamplerBorderColorOpaqueWhite:
        return Gfx::SE_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    default:
        throw std::logic_error("Not implemented");
    }
}

MTL::CompareFunction Seele::Metal::cast(Gfx::SeCompareOp compare) {
    switch (compare) {
    case Gfx::SE_COMPARE_OP_NEVER:
        return MTL::CompareFunctionNever;
    case Gfx::SE_COMPARE_OP_LESS:
        return MTL::CompareFunctionLess;
    case Gfx::SE_COMPARE_OP_EQUAL:
        return MTL::CompareFunctionEqual;
    case Gfx::SE_COMPARE_OP_LESS_OR_EQUAL:
        return MTL::CompareFunctionLessEqual;
    case Gfx::SE_COMPARE_OP_GREATER:
        return MTL::CompareFunctionGreater;
    case Gfx::SE_COMPARE_OP_NOT_EQUAL:
        return MTL::CompareFunctionNotEqual;
    case Gfx::SE_COMPARE_OP_GREATER_OR_EQUAL:
        return MTL::CompareFunctionGreaterEqual;
    case Gfx::SE_COMPARE_OP_ALWAYS:
        return MTL::CompareFunctionAlways;
    default:
        throw std::logic_error("Not implemented");
    }
}

Gfx::SeCompareOp Seele::Metal::cast(MTL::CompareFunction compare) {
    switch (compare) {
    case MTL::CompareFunctionNever:
        return Gfx::SE_COMPARE_OP_NEVER;
    case MTL::CompareFunctionLess:
        return Gfx::SE_COMPARE_OP_LESS;
    case MTL::CompareFunctionEqual:
        return Gfx::SE_COMPARE_OP_EQUAL;
    case MTL::CompareFunctionLessEqual:
        return Gfx::SE_COMPARE_OP_LESS_OR_EQUAL;
    case MTL::CompareFunctionGreater:
        return Gfx::SE_COMPARE_OP_GREATER;
    case MTL::CompareFunctionNotEqual:
        return Gfx::SE_COMPARE_OP_NOT_EQUAL;
    case MTL::CompareFunctionGreaterEqual:
        return Gfx::SE_COMPARE_OP_GREATER_OR_EQUAL;
    case MTL::CompareFunctionAlways:
        return Gfx::SE_COMPARE_OP_ALWAYS;
    }
}

MTL::SamplerMinMagFilter Seele::Metal::cast(Gfx::SeFilter filter) {
    switch (filter) {
    case Gfx::SE_FILTER_NEAREST:
        return MTL::SamplerMinMagFilterNearest;
    case Gfx::SE_FILTER_LINEAR:
        return MTL::SamplerMinMagFilterLinear;
    case Gfx::SE_FILTER_CUBIC_IMG:
        return MTL::SamplerMinMagFilterLinear;
    default:
        throw std::logic_error("Not implemented");
    }
}

Gfx::SeFilter Seele::Metal::cast(MTL::SamplerMinMagFilter filter) {
    switch (filter) {
    case MTL::SamplerMinMagFilterNearest:
        return Gfx::SE_FILTER_NEAREST;
    case MTL::SamplerMinMagFilterLinear:
        return Gfx::SE_FILTER_LINEAR;
    }
}

MTL::SamplerMipFilter Seele::Metal::cast(Gfx::SeSamplerMipmapMode filter) {
    switch (filter) {
    case Seele::Gfx::SE_SAMPLER_MIPMAP_MODE_NEAREST:
        return MTL::SamplerMipFilterNearest;
    case Seele::Gfx::SE_SAMPLER_MIPMAP_MODE_LINEAR:
        return MTL::SamplerMipFilterLinear;
    default:
        throw std::logic_error("Not implemented");
    }
}

Gfx::SeSamplerMipmapMode Seele::Metal::cast(MTL::SamplerMipFilter filter) {
    switch (filter) {
    case MTL::SamplerMipFilterNotMipmapped:
        return Gfx::SE_SAMPLER_MIPMAP_MODE_LINEAR;
    case MTL::SamplerMipFilterNearest:
        return Gfx::SE_SAMPLER_MIPMAP_MODE_NEAREST;
    case MTL::SamplerMipFilterLinear:
        return Gfx::SE_SAMPLER_MIPMAP_MODE_LINEAR;
    }
}

MTL::SamplerAddressMode Seele::Metal::cast(Gfx::SeSamplerAddressMode mode) {
    switch (mode) {
    case Gfx::SE_SAMPLER_ADDRESS_MODE_REPEAT:
        return MTL::SamplerAddressModeRepeat;
    case Gfx::SE_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
        return MTL::SamplerAddressModeMirrorRepeat;
    case Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
        return MTL::SamplerAddressModeMirrorClampToEdge;
    case Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
        return MTL::SamplerAddressModeClampToBorderColor;
    case Gfx::SE_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
        return MTL::SamplerAddressModeMirrorClampToEdge;
    }
}

Gfx::SeSamplerAddressMode Seele::Metal::cast(MTL::SamplerAddressMode mode) {
    switch (mode) {
    case MTL::SamplerAddressModeClampToEdge:
        return Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case MTL::SamplerAddressModeMirrorClampToEdge:
        return Gfx::SE_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    case MTL::SamplerAddressModeRepeat:
        return Gfx::SE_SAMPLER_ADDRESS_MODE_REPEAT;
    case MTL::SamplerAddressModeMirrorRepeat:
        return Gfx::SE_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case MTL::SamplerAddressModeClampToZero:
        return Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case MTL::SamplerAddressModeClampToBorderColor:
        return Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
}

MTL::PrimitiveTopologyClass Seele::Metal::cast(Gfx::SePrimitiveTopology topology) {
    switch (topology) {
    case Gfx::SE_PRIMITIVE_TOPOLOGY_POINT_LIST:
        return MTL::PrimitiveTopologyClassPoint;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_LIST:
        return MTL::PrimitiveTopologyClassLine;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_STRIP:
        return MTL::PrimitiveTopologyClassLine;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        return MTL::PrimitiveTopologyClassTriangle;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
        return MTL::PrimitiveTopologyClassTriangle;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
        return MTL::PrimitiveTopologyClassTriangle;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
        return MTL::PrimitiveTopologyClassLine;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
        return MTL::PrimitiveTopologyClassLine;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
        return MTL::PrimitiveTopologyClassTriangle;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
        return MTL::PrimitiveTopologyClassTriangle;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_PATCH_LIST:
        return MTL::PrimitiveTopologyClassUnspecified;
    }
}

MTL::IndexType Seele::Metal::cast(Gfx::SeIndexType indexType) {
    switch (indexType) {
    case Gfx::SE_INDEX_TYPE_UINT16:
        return MTL::IndexTypeUInt16;
    case Gfx::SE_INDEX_TYPE_UINT32:
        return MTL::IndexTypeUInt32;
    default:
        throw std::logic_error("Not implemented");
    }
}

Gfx::SeIndexType Seele::Metal::cast(MTL::IndexType indexType) {
    switch (indexType) {
    case MTL::IndexTypeUInt16:
        return Gfx::SE_INDEX_TYPE_UINT16;
    case MTL::IndexTypeUInt32:
        return Gfx::SE_INDEX_TYPE_UINT32;
    }
}
