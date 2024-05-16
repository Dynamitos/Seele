#include "PipelineCache.h"
#include "Graphics.h"
#include "Enums.h"
#include "RenderPass.h"
#include "Descriptor.h"
#include "Shader.h"
#include <fstream>

using namespace Seele;
using namespace Seele::Vulkan;

PipelineCache::PipelineCache(PGraphics graphics, const std::string& cacheFilePath)
    : graphics(graphics)
    , cacheFile(cacheFilePath)
{
    std::ifstream stream(cacheFilePath, std::ios::binary | std::ios::ate);
    VkPipelineCacheCreateInfo cacheCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .initialDataSize = 0,
    };
    if(stream.good())
    {
        Array<uint8> cacheData;
        uint32 fileSize = static_cast<uint32>(stream.tellg());
        cacheData.resize(fileSize);
        stream.seekg(0);
        stream.read((char*)cacheData.data(), fileSize);
        cacheCreateInfo.initialDataSize = fileSize;
        cacheCreateInfo.pInitialData = cacheData.data();
        std::cout << "Loaded " << fileSize << " bytes from pipeline cache" << std::endl;
    }
    VK_CHECK(vkCreatePipelineCache(graphics->getDevice(), &cacheCreateInfo, nullptr, &cache));
}

PipelineCache::~PipelineCache()
{
    size_t cacheSize;
    VK_CHECK(vkGetPipelineCacheData(graphics->getDevice(), cache, &cacheSize, nullptr));
    Array<uint8> cacheData(cacheSize);
    VK_CHECK(vkGetPipelineCacheData(graphics->getDevice(), cache, &cacheSize, cacheData.data()));
    std::ofstream stream(cacheFile, std::ios::binary);
    stream.write((char*)cacheData.data(), cacheSize);
    stream.flush();
    stream.close();
    vkDestroyPipelineCache(graphics->getDevice(), cache, nullptr);
    std::cout << "Written " << cacheSize << " bytes to cache" << std::endl;
}

PGraphicsPipeline PipelineCache::createPipeline(Gfx::LegacyPipelineCreateInfo gfxInfo)
{
    PPipelineLayout layout = Gfx::PPipelineLayout(gfxInfo.pipelineLayout).cast<PipelineLayout>();
    uint32 hash = layout->getHash();
    Array<VkVertexInputBindingDescription> bindings;
    Array<VkVertexInputAttributeDescription> attributes;
    if (gfxInfo.vertexInput != nullptr)
    {
        const VertexInputStateCreateInfo& vertexInputDesc = gfxInfo.vertexInput->getInfo();
        for (const auto& b : vertexInputDesc.bindings)
        {
            bindings.add() = {
                .binding = b.binding,
                .stride = b.stride,
                .inputRate = VkVertexInputRate(b.inputRate),
            };
        }
        for (const auto& a : vertexInputDesc.attributes)
        {
            attributes.add() = {
                .location = a.location,
                .binding = a.binding,
                .format = cast(a.format),
                .offset = a.offset,
            };
        }
        hash = CRC::Calculate(bindings.data(), sizeof(VkVertexInputBindingDescription) * bindings.size(), CRC::CRC_32(), hash);
        hash = CRC::Calculate(attributes.data(), sizeof(VkVertexInputAttributeDescription) * attributes.size(), CRC::CRC_32(), hash);
    }
    VkPipelineVertexInputStateCreateInfo vertexInput = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = (uint32)bindings.size(),
        .pVertexBindingDescriptions = bindings.data(),
        .vertexAttributeDescriptionCount = (uint32)attributes.size(),
        .pVertexAttributeDescriptions = attributes.data(),
    };
    uint32 stageCount = 0;

    VkPipelineShaderStageCreateInfo stageInfos[2];
    std::memset(stageInfos, 0, sizeof(stageInfos));

    PVertexShader vertexShader = gfxInfo.vertexShader.cast<VertexShader>();
    stageInfos[stageCount++] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertexShader->getModuleHandle(),
        .pName = vertexShader->getEntryPointName(),
        .pSpecializationInfo = nullptr,
    };

    if (gfxInfo.fragmentShader != nullptr)
    {
        PFragmentShader fragment = gfxInfo.fragmentShader.cast<FragmentShader>();

        stageInfos[stageCount++] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragment->getModuleHandle(),
            .pName = fragment->getEntryPointName(),
            .pSpecializationInfo = nullptr,
        };
    }
    hash = CRC::Calculate(stageInfos, sizeof(stageInfos), CRC::CRC_32(), hash);
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = cast(gfxInfo.topology),
        .primitiveRestartEnable = false,
    };
    hash = CRC::Calculate(&assemblyInfo, sizeof(assemblyInfo), CRC::CRC_32(), hash);
    VkPipelineViewportStateCreateInfo viewportInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };
    hash = CRC::Calculate(&viewportInfo, sizeof(viewportInfo), CRC::CRC_32(), hash);
    VkPipelineRasterizationStateCreateInfo rasterizationState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = gfxInfo.rasterizationState.depthClampEnable,
        .rasterizerDiscardEnable = gfxInfo.rasterizationState.rasterizerDiscardEnable,
        .polygonMode = cast(gfxInfo.rasterizationState.polygonMode),
        .cullMode = gfxInfo.rasterizationState.cullMode,
        .frontFace = (VkFrontFace)gfxInfo.rasterizationState.frontFace,
        .depthBiasEnable = gfxInfo.rasterizationState.depthBiasEnable,
        .depthBiasConstantFactor = gfxInfo.rasterizationState.depthBiasConstantFactor,
        .depthBiasClamp = gfxInfo.rasterizationState.depthBiasClamp,
        .depthBiasSlopeFactor = gfxInfo.rasterizationState.depthBiasSlopeFactor,
        .lineWidth = gfxInfo.rasterizationState.lineWidth,
    };
    hash = CRC::Calculate(&rasterizationState, sizeof(rasterizationState), CRC::CRC_32(), hash);

    VkPipelineMultisampleStateCreateInfo multisampleState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = (VkSampleCountFlagBits)gfxInfo.multisampleState.samples,
        .sampleShadingEnable = gfxInfo.multisampleState.sampleShadingEnable,
        .minSampleShading = gfxInfo.multisampleState.minSampleShading,
        .alphaToCoverageEnable = gfxInfo.multisampleState.alphaCoverageEnable,
        .alphaToOneEnable = gfxInfo.multisampleState.alphaToOneEnable,
    };
    hash = CRC::Calculate(&multisampleState, sizeof(multisampleState), CRC::CRC_32(), hash);

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = gfxInfo.depthStencilState.depthTestEnable,
        .depthWriteEnable = gfxInfo.depthStencilState.depthWriteEnable,
        .depthCompareOp = cast(gfxInfo.depthStencilState.depthCompareOp),
        .depthBoundsTestEnable = gfxInfo.depthStencilState.depthBoundsTestEnable,
        .front = {(VkStencilOp)gfxInfo.depthStencilState.front},
        .back = {(VkStencilOp)gfxInfo.depthStencilState.back},
        .minDepthBounds = gfxInfo.depthStencilState.minDepthBounds,
        .maxDepthBounds = gfxInfo.depthStencilState.maxDepthBounds,
    };
    hash = CRC::Calculate(&depthStencilState, sizeof(depthStencilState), CRC::CRC_32(), hash);

    Array<VkPipelineColorBlendAttachmentState> blendAttachments;
    for(uint32 i = 0; i < gfxInfo.colorBlend.attachmentCount; ++i)
    {
        const Gfx::ColorBlendState::BlendAttachment& attachment = gfxInfo.colorBlend.blendAttachments[i];
        blendAttachments.add() = {
            .blendEnable = attachment.blendEnable,
            .srcColorBlendFactor = (VkBlendFactor)attachment.srcColorBlendFactor,
            .dstColorBlendFactor = (VkBlendFactor)attachment.dstColorBlendFactor,
            .colorBlendOp = (VkBlendOp)attachment.colorBlendOp,
            .srcAlphaBlendFactor = (VkBlendFactor)attachment.srcAlphaBlendFactor,
            .dstAlphaBlendFactor = (VkBlendFactor)attachment.dstAlphaBlendFactor,
            .alphaBlendOp = (VkBlendOp)attachment.alphaBlendOp,
            .colorWriteMask = attachment.colorWriteMask,
        };
    }
    hash = CRC::Calculate(blendAttachments.data(), blendAttachments.size() * sizeof(VkPipelineColorBlendAttachmentState), CRC::CRC_32(), hash);

    VkPipelineColorBlendStateCreateInfo blendState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = gfxInfo.colorBlend.logicOpEnable,
        .logicOp = (VkLogicOp)gfxInfo.colorBlend.logicOp,
        .attachmentCount = (uint32)blendAttachments.size(),
        .pAttachments = blendAttachments.data(),
    };
    std::memcpy(blendState.blendConstants, gfxInfo.colorBlend.blendConstants.data(), sizeof(blendState.blendConstants));
    hash = CRC::Calculate(&blendState, sizeof(blendState), CRC::CRC_32(), hash);

    uint32 numDynamicEnabled = 0;
    StaticArray<VkDynamicState, 2> dynamicEnabled;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_SCISSOR;
    hash = CRC::Calculate(dynamicEnabled.data(), dynamicEnabled.size() * sizeof(VkDynamicState), CRC::CRC_32(), hash);

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = (uint32)dynamicEnabled.size(),
        .pDynamicStates = dynamicEnabled.data(),
    };

    if (graphicsPipelines.contains(hash))
    {
        return graphicsPipelines[hash];
    }
    VkPipeline pipelineHandle;

    VkGraphicsPipelineCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .stageCount = stageCount,
        .pStages = stageInfos,
        .pVertexInputState = &vertexInput,
        .pInputAssemblyState = &assemblyInfo,
        .pViewportState = &viewportInfo,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &blendState,
        .pDynamicState = &dynamicState,
        .layout = layout->getHandle(),
        .renderPass = gfxInfo.renderPass.cast<RenderPass>()->getHandle(),
        .subpass = 0,
    };
    auto beginTime = std::chrono::high_resolution_clock::now();
    VK_CHECK(vkCreateGraphicsPipelines(graphics->getDevice(), cache, 1, &createInfo, nullptr, &pipelineHandle));
    auto endTime = std::chrono::high_resolution_clock::now();
    int64 delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - beginTime).count();
    std::cout << "Gfx creation time: " << delta << std::endl;
    
    OGraphicsPipeline pipeline = new GraphicsPipeline(graphics, pipelineHandle, gfxInfo.pipelineLayout);
    PGraphicsPipeline result = pipeline;
    graphicsPipelines[hash] = std::move(pipeline);
    return result;
}


PGraphicsPipeline PipelineCache::createPipeline(Gfx::MeshPipelineCreateInfo gfxInfo)
{
    uint32 hash = CRC::Calculate(&gfxInfo, sizeof(Gfx::MeshPipelineCreateInfo), CRC::CRC_32());
    if (graphicsPipelines.contains(hash))
    {
        return graphicsPipelines[hash];
    }
    PPipelineLayout layout = Gfx::PPipelineLayout(gfxInfo.pipelineLayout).cast<PipelineLayout>();
    //uint32 hash = layout->getHash();
    uint32 stageCount = 0;


    VkPipelineShaderStageCreateInfo stageInfos[3];
    std::memset(stageInfos, 0, sizeof(stageInfos));

    if (gfxInfo.taskShader != nullptr)
    {
        PTaskShader taskShader = gfxInfo.taskShader.cast<TaskShader>();
        stageInfos[stageCount++] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_TASK_BIT_EXT,
            .module = taskShader->getModuleHandle(),
            .pName = taskShader->getEntryPointName(),
            .pSpecializationInfo = nullptr,
        };
    }

    PMeshShader meshShader = gfxInfo.meshShader.cast<MeshShader>();
    stageInfos[stageCount++] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_MESH_BIT_EXT,
        .module = meshShader->getModuleHandle(),
        .pName = meshShader->getEntryPointName(),
        .pSpecializationInfo = nullptr,
    };

    if (gfxInfo.fragmentShader != nullptr)
    {
        PFragmentShader fragment = gfxInfo.fragmentShader.cast<FragmentShader>();

        stageInfos[stageCount++] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragment->getModuleHandle(),
            .pName = fragment->getEntryPointName(),
            .pSpecializationInfo = nullptr,
        };
    }
    //hash = CRC::Calculate(stageInfos, sizeof(stageInfos), CRC::CRC_32(), hash);

    VkPipelineViewportStateCreateInfo viewportInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };
    //hash = CRC::Calculate(&viewportInfo, sizeof(VkPipelineViewportStateCreateInfo), CRC::CRC_32(), hash);
    VkPipelineRasterizationStateCreateInfo rasterizationState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = gfxInfo.rasterizationState.depthClampEnable,
        .rasterizerDiscardEnable = gfxInfo.rasterizationState.rasterizerDiscardEnable,
        .polygonMode = cast(gfxInfo.rasterizationState.polygonMode),
        .cullMode = gfxInfo.rasterizationState.cullMode,
        .frontFace = (VkFrontFace)gfxInfo.rasterizationState.frontFace,
        .depthBiasEnable = gfxInfo.rasterizationState.depthBiasEnable,
        .depthBiasConstantFactor = gfxInfo.rasterizationState.depthBiasConstantFactor,
        .depthBiasClamp = gfxInfo.rasterizationState.depthBiasClamp,
        .depthBiasSlopeFactor = gfxInfo.rasterizationState.depthBiasSlopeFactor,
        .lineWidth = 0,
    };
    //hash = CRC::Calculate(&rasterizationState, sizeof(VkPipelineRasterizationStateCreateInfo), CRC::CRC_32(), hash);

    VkPipelineMultisampleStateCreateInfo multisampleState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = (VkSampleCountFlagBits)gfxInfo.multisampleState.samples,
        .sampleShadingEnable = gfxInfo.multisampleState.sampleShadingEnable,
        .minSampleShading = gfxInfo.multisampleState.minSampleShading,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = gfxInfo.multisampleState.alphaCoverageEnable,
        .alphaToOneEnable = gfxInfo.multisampleState.alphaToOneEnable,
    };
    //hash = CRC::Calculate(&multisampleState, sizeof(VkPipelineMultisampleStateCreateInfo), CRC::CRC_32(), hash);

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = gfxInfo.depthStencilState.depthTestEnable,
        .depthWriteEnable = gfxInfo.depthStencilState.depthWriteEnable,
        .depthCompareOp = cast(gfxInfo.depthStencilState.depthCompareOp),
        .depthBoundsTestEnable = gfxInfo.depthStencilState.depthBoundsTestEnable,
        .stencilTestEnable = gfxInfo.depthStencilState.stencilTestEnable,
        .front = VkStencilOpState{
            .failOp = VK_STENCIL_OP_ZERO,
            .passOp = (VkStencilOp)gfxInfo.depthStencilState.front,
            .depthFailOp = VK_STENCIL_OP_ZERO,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .compareMask = 0,
            .writeMask = 0,
            .reference = 0,
        },
        .back = VkStencilOpState{
            .failOp = VK_STENCIL_OP_ZERO,
            .passOp = (VkStencilOp)gfxInfo.depthStencilState.back,
            .depthFailOp = VK_STENCIL_OP_ZERO,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .compareMask = 0,
            .writeMask = 0,
            .reference = 0,
        },
        .minDepthBounds = gfxInfo.depthStencilState.minDepthBounds,
        .maxDepthBounds = gfxInfo.depthStencilState.maxDepthBounds,
    };
    //hash = CRC::Calculate(&depthStencilState, sizeof(VkPipelineDepthStencilStateCreateInfo), CRC::CRC_32(), hash);

    Array<VkPipelineColorBlendAttachmentState> blendAttachments;
    for (uint32 i = 0; i < gfxInfo.colorBlend.attachmentCount; ++i)
    {
        const Gfx::ColorBlendState::BlendAttachment& attachment = gfxInfo.colorBlend.blendAttachments[i];
        blendAttachments.add() = {
            .blendEnable = attachment.blendEnable,
            .srcColorBlendFactor = (VkBlendFactor)attachment.srcColorBlendFactor,
            .dstColorBlendFactor = (VkBlendFactor)attachment.dstColorBlendFactor,
            .colorBlendOp = (VkBlendOp)attachment.colorBlendOp,
            .srcAlphaBlendFactor = (VkBlendFactor)attachment.srcAlphaBlendFactor,
            .dstAlphaBlendFactor = (VkBlendFactor)attachment.dstAlphaBlendFactor,
            .alphaBlendOp = (VkBlendOp)attachment.alphaBlendOp,
            .colorWriteMask = attachment.colorWriteMask,
        };
    }
    //hash = CRC::Calculate(blendAttachments.data(), blendAttachments.size() * sizeof(VkPipelineColorBlendAttachmentState), CRC::CRC_32(), hash);

    VkPipelineColorBlendStateCreateInfo blendState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = gfxInfo.colorBlend.logicOpEnable,
        .logicOp = (VkLogicOp)gfxInfo.colorBlend.logicOp,
        .attachmentCount = (uint32)blendAttachments.size(),
        .pAttachments = blendAttachments.data(),
    };
    std::memcpy(blendState.blendConstants, gfxInfo.colorBlend.blendConstants.data(), sizeof(blendState.blendConstants));

    uint32 numDynamicEnabled = 0;
    StaticArray<VkDynamicState, 2> dynamicEnabled;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_SCISSOR;
    //hash = CRC::Calculate(dynamicEnabled.data(), sizeof(dynamicEnabled), CRC::CRC_32(), hash);

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .dynamicStateCount = (uint32)dynamicEnabled.size(),
        .pDynamicStates = dynamicEnabled.data(),
    };
    VkPipeline pipelineHandle;

    VkGraphicsPipelineCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .stageCount = stageCount,
        .pStages = stageInfos,
        .pVertexInputState = nullptr,
        .pInputAssemblyState = nullptr,
        .pViewportState = &viewportInfo,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &blendState,
        .pDynamicState = &dynamicState,
        .layout = layout->getHandle(),
        .renderPass = gfxInfo.renderPass.cast<RenderPass>()->getHandle(),
        .subpass = 0,
    };
    auto beginTime = std::chrono::high_resolution_clock::now();
    VK_CHECK(vkCreateGraphicsPipelines(graphics->getDevice(), cache, 1, &createInfo, nullptr, &pipelineHandle));
    auto endTime = std::chrono::high_resolution_clock::now();
    int64 delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - beginTime).count();
    std::cout << "Gfx creation time: " << delta << std::endl;

    OGraphicsPipeline pipeline = new GraphicsPipeline(graphics, pipelineHandle, gfxInfo.pipelineLayout);
    PGraphicsPipeline result = pipeline;
    graphicsPipelines[hash] = std::move(pipeline);
    return result;
}


PComputePipeline PipelineCache::createPipeline(Gfx::ComputePipelineCreateInfo computeInfo) 
{
    PPipelineLayout layout = Gfx::PPipelineLayout(computeInfo.pipelineLayout).cast<PipelineLayout>();
    auto computeStage = computeInfo.computeShader.cast<ComputeShader>();

    uint32 hash = layout->getHash();

    VkComputePipelineCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .stage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = computeStage->getModuleHandle(),
            .pName = computeStage->getEntryPointName(),
        },
        .layout = layout->getHandle(),
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };
    hash = CRC::Calculate(&createInfo, sizeof(createInfo), CRC::CRC_32(), hash);
    
    VkPipeline pipelineHandle;
    auto beginTime = std::chrono::high_resolution_clock::now();
    VK_CHECK(vkCreateComputePipelines(graphics->getDevice(), cache, 1, &createInfo, nullptr, &pipelineHandle));
    auto endTime = std::chrono::high_resolution_clock::now();
    int64 delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - beginTime).count();
    std::cout << "Compute creation time: " << delta << std::endl;

    OComputePipeline pipeline = new ComputePipeline(graphics, pipelineHandle, computeInfo.pipelineLayout);
    PComputePipeline result = pipeline;
    graphicsPipelines[hash] = std::move(pipeline);
    return result;
}