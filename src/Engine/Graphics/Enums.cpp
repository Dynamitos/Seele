#include "Enums.h"
#include <stdexcept>

using namespace Seele;
using namespace Seele::Gfx;

FormatCompatibilityInfo Gfx::getFormatInfo(SeFormat format)
{
    switch(format) {
        case SE_FORMAT_R4G4_UNORM_PACK8:
        case SE_FORMAT_R8_UNORM:
        case SE_FORMAT_R8_SNORM:
        case SE_FORMAT_R8_USCALED:
        case SE_FORMAT_R8_SSCALED:
        case SE_FORMAT_R8_UINT:
        case SE_FORMAT_R8_SINT:
        case SE_FORMAT_R8_SRGB:
            return FormatCompatibilityInfo {
                .blockSize = 1,
                .blockExtent = UVector(1, 1, 1),
                .texelsPerBlock = 1,
            };
        case SE_FORMAT_R10X6_UNORM_PACK16:
        case SE_FORMAT_R12X4_UNORM_PACK16:
        //case SE_FORMAT_A4R4G4B4_UNORM_PACK16:
        //case SE_FORMAT_A4B4G4R4_UNORM_PACK16:
        case SE_FORMAT_R4G4B4A4_UNORM_PACK16:
        case SE_FORMAT_B4G4R4A4_UNORM_PACK16:
        case SE_FORMAT_R5G6B5_UNORM_PACK16:
        case SE_FORMAT_B5G6R5_UNORM_PACK16:
        case SE_FORMAT_R5G5B5A1_UNORM_PACK16:
        case SE_FORMAT_B5G5R5A1_UNORM_PACK16:
        case SE_FORMAT_A1R5G5B5_UNORM_PACK16:
        case SE_FORMAT_R8G8_UNORM:
        case SE_FORMAT_R8G8_SNORM:
        case SE_FORMAT_R8G8_USCALED:
        case SE_FORMAT_R8G8_SSCALED:
        case SE_FORMAT_R8G8_UINT:
        case SE_FORMAT_R8G8_SINT:
        case SE_FORMAT_R8G8_SRGB:
        case SE_FORMAT_R16_UNORM:
        case SE_FORMAT_R16_SNORM:
        case SE_FORMAT_R16_USCALED:
        case SE_FORMAT_R16_SSCALED:
        case SE_FORMAT_R16_UINT:
        case SE_FORMAT_R16_SINT:
        case SE_FORMAT_R16_SFLOAT:
            return FormatCompatibilityInfo {
                .blockSize = 2,
                .blockExtent = UVector(1, 1, 1),
                .texelsPerBlock = 1,
            };
        case SE_FORMAT_BC7_UNORM_BLOCK:
        case SE_FORMAT_BC7_SRGB_BLOCK:
            return FormatCompatibilityInfo {
                .blockSize = 16,
                .blockExtent = UVector(4, 4, 1),
                .texelsPerBlock = 16,
            };
        case SE_FORMAT_R10X6G10X6_UNORM_2PACK16:
        case SE_FORMAT_R12X4G12X4_UNORM_2PACK16:
            //case SE_FORMAT_R16G16_S10_5_NV:
        case SE_FORMAT_R8G8B8A8_UNORM:
        case SE_FORMAT_R8G8B8A8_SNORM:
        case SE_FORMAT_R8G8B8A8_USCALED:
        case SE_FORMAT_R8G8B8A8_SSCALED:
        case SE_FORMAT_R8G8B8A8_UINT:
        case SE_FORMAT_R8G8B8A8_SINT:
        case SE_FORMAT_R8G8B8A8_SRGB:
        case SE_FORMAT_B8G8R8A8_UNORM:
        case SE_FORMAT_B8G8R8A8_SNORM:
        case SE_FORMAT_B8G8R8A8_USCALED:
        case SE_FORMAT_B8G8R8A8_SSCALED:
        case SE_FORMAT_B8G8R8A8_UINT:
        case SE_FORMAT_B8G8R8A8_SINT:
        case SE_FORMAT_B8G8R8A8_SRGB:
        case SE_FORMAT_A8B8G8R8_UNORM_PACK32:
        case SE_FORMAT_A8B8G8R8_SNORM_PACK32:
        case SE_FORMAT_A8B8G8R8_USCALED_PACK32:
        case SE_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case SE_FORMAT_A8B8G8R8_UINT_PACK32:
        case SE_FORMAT_A8B8G8R8_SINT_PACK32:
        case SE_FORMAT_A8B8G8R8_SRGB_PACK32:
        case SE_FORMAT_A2R10G10B10_UNORM_PACK32:
        case SE_FORMAT_A2R10G10B10_SNORM_PACK32:
        case SE_FORMAT_A2R10G10B10_USCALED_PACK32:
        case SE_FORMAT_A2R10G10B10_SSCALED_PACK32:
        case SE_FORMAT_A2R10G10B10_UINT_PACK32:
        case SE_FORMAT_A2R10G10B10_SINT_PACK32:
        case SE_FORMAT_A2B10G10R10_UNORM_PACK32:
        case SE_FORMAT_A2B10G10R10_SNORM_PACK32:
        case SE_FORMAT_A2B10G10R10_USCALED_PACK32:
        case SE_FORMAT_A2B10G10R10_SSCALED_PACK32:
        case SE_FORMAT_A2B10G10R10_UINT_PACK32:
        case SE_FORMAT_A2B10G10R10_SINT_PACK32:
        case SE_FORMAT_R16G16_UNORM:
        case SE_FORMAT_R16G16_SNORM:
        case SE_FORMAT_R16G16_USCALED:
        case SE_FORMAT_R16G16_SSCALED:
        case SE_FORMAT_R16G16_UINT:
        case SE_FORMAT_R16G16_SINT:
        case SE_FORMAT_R16G16_SFLOAT:
        case SE_FORMAT_R32_UINT:
        case SE_FORMAT_R32_SINT:
        case SE_FORMAT_R32_SFLOAT:
        case SE_FORMAT_B10G11R11_UFLOAT_PACK32:
        case SE_FORMAT_E5B9G9R9_UFLOAT_PACK32:
            return FormatCompatibilityInfo{
                .blockSize = 4,
                .blockExtent = Vector(1, 1, 1),
                .texelsPerBlock = 1,
            };
        default:
            throw new std::logic_error("not yet implemented");
    }
}
