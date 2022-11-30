#pragma once
#include "GraphicsEnums.h"

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
    Math::URect dimensions;
    float fieldOfView = 1.222f; // 70 deg
};
// doesnt own the data, only proxy it
struct BulkResourceData
{
    uint64 size = 0;
    uint64 offset = 0;
    uint8 *data = nullptr;
    Gfx::QueueType owner = Gfx::QueueType::GRAPHICS;
};
struct TextureCreateInfo
{
    BulkResourceData resourceData = BulkResourceData();
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
    BulkResourceData resourceData = BulkResourceData();
    // bytes per vertex
    uint32 vertexSize = 0;
    uint32 numVertices = 0;
};
struct IndexBufferCreateInfo
{
    BulkResourceData resourceData = BulkResourceData();
    Gfx::SeIndexType indexType = Gfx::SeIndexType::SE_INDEX_TYPE_UINT16;
};
struct UniformBufferCreateInfo
{
    BulkResourceData resourceData = BulkResourceData();
    uint8 bDynamic = 0;
};
struct StructuredBufferCreateInfo
{
    BulkResourceData resourceData;
    uint32 stride;
    uint8 bDynamic = 0;
};
struct ShaderCreateInfo
{
    std::string mainModule;
    //It's possible to input multiple source files for materials or vertexFactories
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
    uint8 streamIndex;
    uint8 offset;
    SeFormat vertexFormat;
    uint8 attributeIndex;
    uint8 stride;
    uint8 bInstanced = 0;
};
static_assert(std::is_aggregate_v<VertexElement>);
struct RasterizationState
{
    uint8 depthClampEnable : 1;
    uint8 rasterizerDiscardEnable : 1;
    uint8 depthBiasEnable : 1;
    float depthBoasConstantFactor;
    float depthBiasClamp;
    float depthBiasSlopeFactor;
    float lineWidth;
    SePolygonMode polygonMode;
    SeCullModeFlags cullMode;
    SeFrontFace frontFace;
};
struct MultisampleState
{
    uint32 samples;
    float minSampleShading;
    uint8 sampleShadingEnable : 1;
    uint8 alphaCoverageEnable;
    uint8 alphaToOneEnable;
};
struct DepthStencilState
{
    uint8 depthTestEnable : 1;
    uint8 depthWriteEnable : 1;
    uint8 depthBoundsTestEnable : 1;
    uint8 stencilTestEnable : 1;
    SeCompareOp depthCompareOp;
    SeStencilOp front;
    SeStencilOp back;
    float minDepthBounds;
    float maxDepthBounds;
};
struct ColorBlendState
{
    uint8 logicOpEnable : 1;
    SeLogicOp logicOp;
    uint32 attachmentCount;
    struct BlendAttachment
    {
        uint8 blendEnable;
        SeBlendFactor srcColorBlendFactor;
        SeBlendFactor dstColorBlendFactor;
        SeBlendOp colorBlendOp;
        SeBlendFactor srcAlphaBlendFactor;
        SeBlendFactor dstAlphaBlendFactor;
        SeBlendOp alphaBlendOp;
        SeColorComponentFlags colorWriteMask;
    } blendAttachments[16];
    float blendConstants[4];
};
} // namespace Gfx
DECLARE_NAME_REF(Gfx, VertexDeclaration)
DECLARE_NAME_REF(Gfx, VertexShader)
DECLARE_NAME_REF(Gfx, ControlShader)
DECLARE_NAME_REF(Gfx, EvaluationShader)
DECLARE_NAME_REF(Gfx, GeometryShader)
DECLARE_NAME_REF(Gfx, FragmentShader)
DECLARE_NAME_REF(Gfx, ComputeShader)
DECLARE_NAME_REF(Gfx, PipelineLayout)
DECLARE_NAME_REF(Gfx, RenderPass)
struct GraphicsPipelineCreateInfo
{
	Gfx::PVertexDeclaration vertexDeclaration;
	Gfx::PVertexShader vertexShader;
	Gfx::PControlShader controlShader;
	Gfx::PEvaluationShader evalShader;
	Gfx::PGeometryShader geometryShader;
	Gfx::PFragmentShader fragmentShader;
	Gfx::PRenderPass renderPass;
    Gfx::PPipelineLayout pipelineLayout;
	Gfx::SePrimitiveTopology topology;
	Gfx::RasterizationState rasterizationState;
	Gfx::DepthStencilState depthStencilState;
	Gfx::MultisampleState multisampleState;
	Gfx::ColorBlendState colorBlend;
	GraphicsPipelineCreateInfo()
	{
		std::memset((void*)this, 0, sizeof(*this));
        topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        rasterizationState.cullMode = Gfx::SE_CULL_MODE_BACK_BIT;
        rasterizationState.polygonMode = Gfx::SE_POLYGON_MODE_FILL;
        rasterizationState.frontFace = Gfx::SE_FRONT_FACE_COUNTER_CLOCKWISE;
        depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilState.minDepthBounds = 0.0f;
        depthStencilState.maxDepthBounds = 1.0f;
        depthStencilState.stencilTestEnable = false;
        depthStencilState.depthWriteEnable = true;
        depthStencilState.depthTestEnable = true;
        multisampleState.samples = 1;
        colorBlend.attachmentCount = 0;
        colorBlend.logicOpEnable = false;
        colorBlend.blendConstants[0] = 1.0f;
        colorBlend.blendConstants[1] = 1.0f;
        colorBlend.blendConstants[2] = 1.0f;
        colorBlend.blendConstants[3] = 1.0f;
        colorBlend.blendAttachments[0].colorWriteMask = 
            Gfx::SE_COLOR_COMPONENT_R_BIT | 
            Gfx::SE_COLOR_COMPONENT_G_BIT | 
            Gfx::SE_COLOR_COMPONENT_B_BIT | 
            Gfx::SE_COLOR_COMPONENT_A_BIT;
	}
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
} // namespace Seele