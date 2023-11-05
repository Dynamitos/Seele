#pragma once
#include "Enums.h"
#include "Containers/Map.h"
#include "Math/Math.h"

namespace Seele
{
struct GraphicsInitializer
{
    const char *applicationName;
    const char *engineName;
    const char *windowLayoutFile;
    /**
	 * layers defines the enabled Vulkan layers used in the instance,
	 * if ENABLE_VALIDATION is defined, standard validation is already enabled
	 * not yet implemented
	 */
    Array<const char *> layers;
    Array<const char *> instanceExtensions;
    Array<const char *> deviceExtensions;
    
    void *windowHandle;
    GraphicsInitializer()
        : applicationName("SeeleEngine")
        , engineName("SeeleEngine")
        , windowLayoutFile(nullptr)
        , layers{"VK_LAYER_KHRONOS_validation"}
        , instanceExtensions{}
        , deviceExtensions{"VK_KHR_swapchain"}
        , windowHandle(nullptr)
    {
    }
};
struct WindowCreateInfo
{
    int32 width;
    int32 height;
    const char *title;
    bool bFullscreen;
    Gfx::SeSampleCountFlags numSamples;
    Gfx::SeFormat pixelFormat;
    void *windowHandle;
};
struct ViewportCreateInfo
{
    URect dimensions;
    float fieldOfView = 1.222f; // 70 deg
};
// doesnt own the data, only proxy it
struct DataSource
{
    uint64 size = 0;
    uint64 offset = 0;
    uint8 *data = nullptr;
    Gfx::QueueType owner = Gfx::QueueType::GRAPHICS;
};
struct TextureCreateInfo
{
    DataSource sourceData = DataSource();
    uint32 width = 1;
    uint32 height = 1;
    uint32 depth = 1;
    bool bArray = false;
    uint32 arrayLayers = 1;
    uint32 mipLevels = 1;
    uint32 samples = 1;
    Gfx::SeFormat format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT;
    Gfx::SeImageUsageFlagBits usage = Gfx::SE_IMAGE_USAGE_SAMPLED_BIT;
};
struct SamplerCreateInfo
{
    Gfx::SeSamplerCreateFlags flags;
    Gfx::SeFilter             magFilter;
    Gfx::SeFilter             minFilter;
    Gfx::SeSamplerMipmapMode  mipmapMode;
    Gfx::SeSamplerAddressMode addressModeU;
    Gfx::SeSamplerAddressMode addressModeV;
    Gfx::SeSamplerAddressMode addressModeW;
    float                     mipLodBias;
    uint32                    anisotropyEnable;
    float                     maxAnisotropy;
    uint32                    compareEnable;
    Gfx::SeCompareOp          compareOp;
    float                     minLod;
    float                     maxLod;
    Gfx::SeBorderColor        borderColor;
    uint32                    unnormalizedCoordinates;
};
struct VertexBufferCreateInfo
{
    DataSource sourceData = DataSource();
    // bytes per vertex
    uint32 vertexSize = 0;
    uint32 numVertices = 0;
};
struct IndexBufferCreateInfo
{
    DataSource sourceData = DataSource();
    Gfx::SeIndexType indexType = Gfx::SeIndexType::SE_INDEX_TYPE_UINT16;
};
struct UniformBufferCreateInfo
{
    DataSource sourceData = DataSource();
    uint8 dynamic = 0;
};
struct ShaderBufferCreateInfo
{
    DataSource sourceData = DataSource();
    uint32 stride;
    uint8 dynamic = 0;
};
struct ShaderCreateInfo
{
    std::string mainModule;
    Array<std::string> additionalModules;
    std::string name; // Debug info
    std::string entryPoint;
    Array<const char*> typeParameter;
    Map<const char*, const char*> defines;
};

namespace Gfx
{
struct SePushConstantRange
{
    SeShaderStageFlags stageFlags;
    uint32_t offset;
    uint32_t size;
};
struct VertexElement
{
    uint8 binding;
    uint8 offset;
    SeFormat vertexFormat;
    uint8 attributeIndex;
    uint8 stride;
    uint8 instanced = 0;
};
static_assert(std::is_aggregate_v<VertexElement>);
struct RasterizationState
{
    uint8 depthClampEnable : 1 = 0;
    uint8 rasterizerDiscardEnable : 1 = 0;
    uint8 depthBiasEnable : 1 = 0;
    float depthBiasConstantFactor = 0;
    float depthBiasClamp = 0;
    float depthBiasSlopeFactor = 0;
    float lineWidth = 0;
    SePolygonMode polygonMode;
    SeCullModeFlags cullMode;
    SeFrontFace frontFace;
};
struct MultisampleState
{
    uint32 samples = 1;
    float minSampleShading = 1;
    uint8 sampleShadingEnable : 1 = 0;
    uint8 alphaCoverageEnable = 0;
    uint8 alphaToOneEnable = 0;
};
struct DepthStencilState
{
    uint8 depthTestEnable : 1 = 0;
    uint8 depthWriteEnable : 1 = 0;
    uint8 depthBoundsTestEnable : 1 = 0;
    uint8 stencilTestEnable : 1 = 0;
    SeCompareOp depthCompareOp;
    SeStencilOp front = Gfx::SE_STENCIL_OP_END_RANGE;
    SeStencilOp back = Gfx::SE_STENCIL_OP_END_RANGE;
    float minDepthBounds;
    float maxDepthBounds;
};
struct ColorBlendState
{
    uint8 logicOpEnable : 1;
    SeLogicOp logicOp = Gfx::SE_LOGIC_OP_OR;
    uint32 attachmentCount;
    struct BlendAttachment
    {
        uint8 blendEnable = 0;
        SeBlendFactor srcColorBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA;
        SeBlendFactor dstColorBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA;
        SeBlendOp colorBlendOp = Gfx::SE_BLEND_OP_ADD;
        SeBlendFactor srcAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA;
        SeBlendFactor dstAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA;
        SeBlendOp alphaBlendOp = Gfx::SE_BLEND_OP_ADD;
        SeColorComponentFlags colorWriteMask = 0;
    } blendAttachments[16];
    float blendConstants[4];
};
DECLARE_REF(VertexDeclaration)
DECLARE_REF(VertexShader)
DECLARE_REF(TaskShader)
DECLARE_REF(MeshShader)
DECLARE_REF(FragmentShader)
DECLARE_REF(ComputeShader)
DECLARE_REF(RenderPass)
DECLARE_REF(PipelineLayout)
struct LegacyPipelineCreateInfo
{
    PVertexDeclaration vertexDeclaration;
    SePrimitiveTopology topology;
    PVertexShader vertexShader;
    PFragmentShader fragmentShader;
    PRenderPass renderPass;
    PPipelineLayout pipelineLayout;
    MultisampleState multisampleState;
    RasterizationState rasterizationState;
    DepthStencilState depthStencilState;
    ColorBlendState colorBlend;
    LegacyPipelineCreateInfo();
    ~LegacyPipelineCreateInfo();
};

struct MeshPipelineCreateInfo
{
    PTaskShader taskShader;
    PMeshShader meshShader;
    PFragmentShader fragmentShader;
    PRenderPass renderPass;
    PPipelineLayout pipelineLayout;
    MultisampleState multisampleState;
    RasterizationState rasterizationState;
    DepthStencilState depthStencilState;
    ColorBlendState colorBlend;
    MeshPipelineCreateInfo();
    ~MeshPipelineCreateInfo();
};
struct ComputePipelineCreateInfo
{
    Gfx::PComputeShader computeShader;
    Gfx::PPipelineLayout pipelineLayout;
    ComputePipelineCreateInfo()
    {
        std::memset((void*)this, 0, sizeof(*this));
    }
};
} // namespace Gfx
} // namespace Seele