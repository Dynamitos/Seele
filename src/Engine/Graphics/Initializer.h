#pragma once
#include "Containers/Map.h"
#include "Enums.h"
#include "Math/Math.h"
#include "MeshData.h"
#include "MinimalEngine.h"


namespace Seele {
struct GraphicsInitializer {
    const char* applicationName;
    const char* engineName;
    const char* windowLayoutFile;
    /**
     * layers defines the enabled Vulkan layers used in the instance,
     * if ENABLE_VALIDATION is defined, standard validation is already enabled
     * not yet implemented
     */
    Array<const char*> layers;
    Array<const char*> instanceExtensions;
    Array<const char*> deviceExtensions;

    void* windowHandle;
    GraphicsInitializer()
        : applicationName("SeeleEngine"), engineName("SeeleEngine"), windowLayoutFile(nullptr), layers{"VK_LAYER_LUNARG_monitor"},
          instanceExtensions{}, deviceExtensions{"VK_KHR_swapchain"}, windowHandle(nullptr) {}
};
struct WindowCreateInfo {
    int32 width;
    int32 height;
    const char* title;
    Gfx::SeFormat preferredFormat = Gfx::SE_FORMAT_B8G8R8A8_UNORM;
    void* windowHandle;
};
struct ViewportCreateInfo {
    URect dimensions;
    float fieldOfView = 1.222f; // 70 deg
    Gfx::SeSampleCountFlags numSamples;
};
// doesnt own the data, only proxy it
struct DataSource {
    uint64 size = 0;
    uint64 offset = 0;
    uint8* data = nullptr;
    Gfx::QueueType owner = Gfx::QueueType::GRAPHICS;
};
struct TextureCreateInfo {
    DataSource sourceData = DataSource();
    Gfx::SeFormat format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT;
    uint32 width = 1;
    uint32 height = 1;
    uint32 depth = 1;
    uint32 mipLevels = 1;
    uint32 layers = 1;
    uint32 elements = 1;
    uint32 samples = 1;
    Gfx::SeImageUsageFlags usage = Gfx::SE_IMAGE_USAGE_SAMPLED_BIT;
    Gfx::SeMemoryPropertyFlags memoryProps = Gfx::SE_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    std::string name;
};
struct SamplerCreateInfo {
    Gfx::SeSamplerCreateFlags flags;
    Gfx::SeFilter magFilter = Gfx::SE_FILTER_LINEAR;
    Gfx::SeFilter minFilter = Gfx::SE_FILTER_LINEAR;
    Gfx::SeSamplerMipmapMode mipmapMode = Gfx::SE_SAMPLER_MIPMAP_MODE_LINEAR;
    Gfx::SeSamplerAddressMode addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_REPEAT;
    Gfx::SeSamplerAddressMode addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_REPEAT;
    Gfx::SeSamplerAddressMode addressModeW = Gfx::SE_SAMPLER_ADDRESS_MODE_REPEAT;
    float mipLodBias = 0.0f;
    uint32 anisotropyEnable = 0;
    float maxAnisotropy = 0.0f;
    uint32 compareEnable = 0;
    Gfx::SeCompareOp compareOp = Gfx::SE_COMPARE_OP_NEVER;
    float minLod = 0.0f;
    float maxLod = 0.0f;
    Gfx::SeBorderColor borderColor = Gfx::SE_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    uint32 unnormalizedCoordinates = 0;
    std::string name;
};
struct VertexBufferCreateInfo {
    DataSource sourceData = DataSource();
    // bytes per vertex
    uint32 vertexSize = 0;
    uint32 numVertices = 0;
    std::string name = "Unnamed";
};
struct IndexBufferCreateInfo {
    DataSource sourceData = DataSource();
    Gfx::SeIndexType indexType = Gfx::SeIndexType::SE_INDEX_TYPE_UINT16;
    std::string name = "Unnamed";
};
struct UniformBufferCreateInfo {
    DataSource sourceData = DataSource();
    uint8 dynamic = 0;
    std::string name = "Unnamed";
};
struct ShaderBufferCreateInfo {
    DataSource sourceData = DataSource();
    uint64 numElements = 1;
    uint8 dynamic = 0;
    uint8 vertexBuffer = 0;
    std::string name = "Unnamed";
};
DECLARE_NAME_REF(Gfx, PipelineLayout)
struct ShaderCreateInfo {
    std::string name; // Debug info
    std::string mainModule;
    Array<std::string> additionalModules;
    std::string entryPoint;
    Array<Pair<const char*, const char*>> typeParameter;
    Map<const char*, const char*> defines;
    Gfx::PPipelineLayout rootSignature;
};
struct VertexInputBinding {
    uint32 binding;
    uint32 stride;
    Gfx::SeVertexInputRate inputRate;
};
struct VertexInputAttribute {
    uint32 location;
    uint32 binding;
    Gfx::SeFormat format;
    uint32 offset;
};
struct VertexInputStateCreateInfo {
    Array<VertexInputBinding> bindings;
    Array<VertexInputAttribute> attributes;
};
DECLARE_REF(MaterialInstance)
DECLARE_REF(Mesh)
namespace Gfx {
struct SePushConstantRange {
    SeShaderStageFlags stageFlags;
    uint32 offset;
    uint32 size;
};
struct RasterizationState {
    uint32 depthClampEnable = 0;
    uint32 rasterizerDiscardEnable = 0;
    SePolygonMode polygonMode = Gfx::SE_POLYGON_MODE_FILL;
    SeCullModeFlags cullMode = Gfx::SE_CULL_MODE_BACK_BIT;
    SeFrontFace frontFace = Gfx::SE_FRONT_FACE_COUNTER_CLOCKWISE;
    uint32 depthBiasEnable = 0;
    float depthBiasConstantFactor = 0;
    float depthBiasClamp = 0;
    float depthBiasSlopeFactor = 0;
    float lineWidth = 1.0;
};
struct MultisampleState {
    SeSampleCountFlags samples = 1;
    uint32 sampleShadingEnable = 0;
    float minSampleShading = 1;
    uint8 alphaCoverageEnable = 0;
    uint8 alphaToOneEnable = 0;
};
struct DepthStencilState {
    uint32 depthTestEnable = 1;
    uint32 depthWriteEnable = 1;
    SeCompareOp depthCompareOp = Gfx::SE_COMPARE_OP_GREATER;
    uint32 depthBoundsTestEnable = 0;
    uint32 stencilTestEnable = 0;
    SeStencilOp front = Gfx::SE_STENCIL_OP_ZERO;
    SeStencilOp back = Gfx::SE_STENCIL_OP_ZERO;
    float minDepthBounds = 0.0f;
    float maxDepthBounds = 1.0f;
};
struct ColorBlendState {
    struct BlendAttachment {
        uint32 blendEnable = 0;
        SeBlendFactor srcColorBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA;
        SeBlendFactor dstColorBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA;
        SeBlendOp colorBlendOp = Gfx::SE_BLEND_OP_ADD;
        SeBlendFactor srcAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA;
        SeBlendFactor dstAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA;
        SeBlendOp alphaBlendOp = Gfx::SE_BLEND_OP_ADD;
        SeColorComponentFlags colorWriteMask =
            Gfx::SE_COLOR_COMPONENT_R_BIT | Gfx::SE_COLOR_COMPONENT_G_BIT | Gfx::SE_COLOR_COMPONENT_B_BIT | Gfx::SE_COLOR_COMPONENT_A_BIT;
    };
    uint32 logicOpEnable = 0;
    SeLogicOp logicOp = Gfx::SE_LOGIC_OP_OR;
    uint32 attachmentCount = 0;
    StaticArray<BlendAttachment, 16> blendAttachments;
    StaticArray<float, 4> blendConstants = {
        1.0f,
        1.0f,
        1.0f,
        1.0f,
    };
};

DECLARE_REF(VertexInput)
DECLARE_REF(VertexShader)
DECLARE_REF(TaskShader)
DECLARE_REF(MeshShader)
DECLARE_REF(FragmentShader)
DECLARE_REF(ComputeShader)
DECLARE_REF(RenderPass)
DECLARE_REF(PipelineLayout)
struct LegacyPipelineCreateInfo {
    SePrimitiveTopology topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PVertexInput vertexInput = nullptr;
    PVertexShader vertexShader = nullptr;
    PFragmentShader fragmentShader = nullptr;
    PRenderPass renderPass = nullptr;
    PPipelineLayout pipelineLayout = nullptr;
    MultisampleState multisampleState;
    RasterizationState rasterizationState;
    DepthStencilState depthStencilState;
    ColorBlendState colorBlend;
};

struct MeshPipelineCreateInfo {
    PTaskShader taskShader = nullptr;
    PMeshShader meshShader = nullptr;
    PFragmentShader fragmentShader = nullptr;
    PRenderPass renderPass = nullptr;
    PPipelineLayout pipelineLayout = nullptr;
    MultisampleState multisampleState;
    RasterizationState rasterizationState;
    DepthStencilState depthStencilState;
    ColorBlendState colorBlend;
};
struct ComputePipelineCreateInfo {
    Gfx::PComputeShader computeShader = nullptr;
    Gfx::PPipelineLayout pipelineLayout = nullptr;
};
struct RayTracingPipelineCreateInfo {
    PPipelineLayout pipelineLayout = nullptr;
    PRayGenShader rayGenShader = nullptr;
    Array<PClosestHitShader> closestHitShaders;
    Array<PAnyHitShader> anyHitShaders;
    Array<PIntersectionShader> intersectionShaders;
    Array<PMissShader> missShaders;
    Array<PCallableShader> callableShaders;
};
struct BottomLevelASCreateInfo {
    PMesh mesh;
};
struct TopLevelASCreateInfo {};
} // namespace Gfx
} // namespace Seele
