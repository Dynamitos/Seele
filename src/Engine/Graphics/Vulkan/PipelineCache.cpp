#include "PipelineCache.h"
#include "Descriptor.h"
#include "Enums.h"
#include "Graphics.h"
#include "RenderPass.h"
#include "Shader.h"
#include <fstream>
#include <vulkan/vulkan_core.h>

using namespace Seele;
using namespace Seele::Vulkan;

PipelineCache::PipelineCache(PGraphics graphics, const std::string& cacheFilePath) : graphics(graphics), cacheFile(cacheFilePath) {
    Array<uint8> cacheData;
    std::ifstream stream(cacheFilePath, std::ios::binary | std::ios::ate);
    if (stream.good()) {
        uint32 fileSize = static_cast<uint32>(stream.tellg());
        cacheData.resize(fileSize);
        stream.seekg(0);
        stream.read((char*)cacheData.data(), fileSize);
        std::cout << "Loaded " << fileSize << " bytes from pipeline cache" << std::endl;
    }
    VkPipelineCacheCreateInfo cacheCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .initialDataSize = cacheData.size(),
        .pInitialData = cacheData.data(),
    };

    VK_CHECK(vkCreatePipelineCache(graphics->getDevice(), &cacheCreateInfo, nullptr, &cache));
}

PipelineCache::~PipelineCache() {
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

PGraphicsPipeline PipelineCache::createPipeline(Gfx::LegacyPipelineCreateInfo gfxInfo) {
    uint32 hash = CRC::Calculate(&gfxInfo, sizeof(Gfx::LegacyPipelineCreateInfo), CRC::CRC_32());
    if (graphicsPipelines.contains(hash)) {
        return graphicsPipelines[hash];
    }
    PPipelineLayout layout = gfxInfo.pipelineLayout.cast<PipelineLayout>();
    Array<VkVertexInputBindingDescription> bindings;
    Array<VkVertexInputAttributeDescription> attributes;
    if (gfxInfo.vertexInput != nullptr) {
        const VertexInputStateCreateInfo& vertexInputDesc = gfxInfo.vertexInput->getInfo();
        for (const auto& b : vertexInputDesc.bindings) {
            bindings.add() = {
                .binding = b.binding,
                .stride = b.stride,
                .inputRate = VkVertexInputRate(b.inputRate),
            };
        }
        for (const auto& a : vertexInputDesc.attributes) {
            attributes.add() = {
                .location = a.location,
                .binding = a.binding,
                .format = cast(a.format),
                .offset = a.offset,
            };
        }
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

    if (gfxInfo.fragmentShader != nullptr) {
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
    VkPipelineInputAssemblyStateCreateInfo assemblyInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = cast(gfxInfo.topology),
        .primitiveRestartEnable = false,
    };
    VkPipelineViewportStateCreateInfo viewportInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };
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
    Array<VkPipelineColorBlendAttachmentState> blendAttachments;
    for (uint32 i = 0; i < gfxInfo.colorBlend.attachmentCount; ++i) {
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

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
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

PGraphicsPipeline PipelineCache::createPipeline(Gfx::MeshPipelineCreateInfo gfxInfo) {
    PPipelineLayout layout = Gfx::PPipelineLayout(gfxInfo.pipelineLayout).cast<PipelineLayout>();
    uint32 hash = layout->getHash();
    uint32 stageCount = 0;

    VkPipelineShaderStageCreateInfo stageInfos[3];
    std::memset(stageInfos, 0, sizeof(stageInfos));

    if (gfxInfo.taskShader != nullptr) {
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

    if (gfxInfo.fragmentShader != nullptr) {
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

    VkPipelineViewportStateCreateInfo viewportInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };
    hash = CRC::Calculate(&viewportInfo, sizeof(VkPipelineViewportStateCreateInfo), CRC::CRC_32(), hash);
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
    hash = CRC::Calculate(&rasterizationState, sizeof(VkPipelineRasterizationStateCreateInfo), CRC::CRC_32(), hash);

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
    hash = CRC::Calculate(&multisampleState, sizeof(VkPipelineMultisampleStateCreateInfo), CRC::CRC_32(), hash);

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = gfxInfo.depthStencilState.depthTestEnable,
        .depthWriteEnable = gfxInfo.depthStencilState.depthWriteEnable,
        .depthCompareOp = cast(gfxInfo.depthStencilState.depthCompareOp),
        .depthBoundsTestEnable = gfxInfo.depthStencilState.depthBoundsTestEnable,
        .stencilTestEnable = gfxInfo.depthStencilState.stencilTestEnable,
        .front =
            VkStencilOpState{
                .failOp = VK_STENCIL_OP_ZERO,
                .passOp = (VkStencilOp)gfxInfo.depthStencilState.front,
                .depthFailOp = VK_STENCIL_OP_ZERO,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .compareMask = 0,
                .writeMask = 0,
                .reference = 0,
            },
        .back =
            VkStencilOpState{
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
    hash = CRC::Calculate(&depthStencilState, sizeof(VkPipelineDepthStencilStateCreateInfo), CRC::CRC_32(), hash);

    Array<VkPipelineColorBlendAttachmentState> blendAttachments;
    for (uint32 i = 0; i < gfxInfo.colorBlend.attachmentCount; ++i) {
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

    uint32 numDynamicEnabled = 0;
    StaticArray<VkDynamicState, 2> dynamicEnabled;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_SCISSOR;
    hash = CRC::Calculate(dynamicEnabled.data(), sizeof(dynamicEnabled), CRC::CRC_32(), hash);

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .dynamicStateCount = (uint32)dynamicEnabled.size(),
        .pDynamicStates = dynamicEnabled.data(),
    };
    if (graphicsPipelines.contains(hash)) {
        return graphicsPipelines[hash];
    }
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

PComputePipeline PipelineCache::createPipeline(Gfx::ComputePipelineCreateInfo computeInfo) {
    PPipelineLayout layout = computeInfo.pipelineLayout.cast<PipelineLayout>();
    auto computeStage = computeInfo.computeShader.cast<ComputeShader>();

    uint32 hash = layout->getHash();

    VkComputePipelineCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .stage =
            {
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

PRayTracingPipeline PipelineCache::createPipeline(Gfx::RayTracingPipelineCreateInfo createInfo) {
    Array<VkPipelineShaderStageCreateInfo> shaderStages;
    Array<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
    {
        auto rayGen = createInfo.rayGenGroup.shader.cast<RayGenShader>();
        shaderStages.add(VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
            .module = rayGen->getModuleHandle(),
            .pName = rayGen->getEntryPointName(),
            .pSpecializationInfo = nullptr,
        });
        shaderGroups.add(VkRayTracingShaderGroupCreateInfoKHR{
            .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
            .pNext = nullptr,
            .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
            .generalShader = static_cast<uint32>(shaderStages.size() - 1),
            .closestHitShader = VK_SHADER_UNUSED_KHR,
            .anyHitShader = VK_SHADER_UNUSED_KHR,
            .intersectionShader = VK_SHADER_UNUSED_KHR,
            .pShaderGroupCaptureReplayHandle = nullptr,
        });
    }
    {
        for (const auto& hitgroup : createInfo.hitGroups) {
            auto hit = hitgroup.closestHitShader.cast<ClosestHitShader>();
            shaderStages.add(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
                .module = hit->getModuleHandle(),
                .pName = hit->getEntryPointName(),
                .pSpecializationInfo = nullptr,
            });
            uint32 hitIndex = static_cast<uint32>(shaderStages.size() - 1);
            uint32 anyHitIndex = VK_SHADER_UNUSED_KHR;
            uint32 intersectionIndex = VK_SHADER_UNUSED_KHR;
            if (hitgroup.anyHitShader != nullptr) {
                auto anyHit = hitgroup.anyHitShader.cast<AnyHitShader>();
                anyHitIndex = (uint32)shaderStages.size();
                shaderStages.add(VkPipelineShaderStageCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
                    .module = anyHit->getModuleHandle(),
                    .pName = anyHit->getEntryPointName(),
                    .pSpecializationInfo = nullptr,
                });
            }
            if (hitgroup.intersectionShader != nullptr) {
                auto intersect = hitgroup.intersectionShader.cast<IntersectionShader>();
                intersectionIndex = (uint32)shaderGroups.size();
                shaderStages.add(VkPipelineShaderStageCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
                    .module = intersect->getModuleHandle(),
                    .pName = intersect->getEntryPointName(),
                    .pSpecializationInfo = nullptr,
                });
            }
            shaderGroups.add(VkRayTracingShaderGroupCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
                .pNext = nullptr,
                .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
                .generalShader = VK_SHADER_UNUSED_KHR,
                .closestHitShader = hitIndex,
                .anyHitShader = anyHitIndex,
                .intersectionShader = intersectionIndex,
                .pShaderGroupCaptureReplayHandle = nullptr,
            });
        }
    }
    {
        for (auto gfxMiss : createInfo.missGroups) {
            auto miss = gfxMiss.shader.cast<MissShader>();
            shaderStages.add(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = VK_SHADER_STAGE_MISS_BIT_KHR,
                .module = miss->getModuleHandle(),
                .pName = miss->getEntryPointName(),
                .pSpecializationInfo = nullptr,
            });
            shaderGroups.add(VkRayTracingShaderGroupCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
                .pNext = nullptr,
                .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
                .generalShader = static_cast<uint32>(shaderStages.size() - 1),
                .closestHitShader = VK_SHADER_UNUSED_KHR,
                .anyHitShader = VK_SHADER_UNUSED_KHR,
                .intersectionShader = VK_SHADER_UNUSED_KHR,
                .pShaderGroupCaptureReplayHandle = nullptr,
            });
        }
    }
    {
        for (auto gfxCallable : createInfo.callableGroups) {
            auto call = gfxCallable.shader.cast<CallableShader>();
            shaderStages.add(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
                .module = call->getModuleHandle(),
                .pName = call->getEntryPointName(),
                .pSpecializationInfo = nullptr,
            });
            shaderGroups.add(VkRayTracingShaderGroupCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
                .pNext = nullptr,
                .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
                .generalShader = static_cast<uint32>(shaderStages.size() - 1),
                .closestHitShader = VK_SHADER_UNUSED_KHR,
                .anyHitShader = VK_SHADER_UNUSED_KHR,
                .intersectionShader = VK_SHADER_UNUSED_KHR,
                .pShaderGroupCaptureReplayHandle = nullptr,
            });
        }
    }
    uint32 hash = CRC::Calculate(shaderStages.data(), sizeof(VkPipelineShaderStageCreateInfo) * shaderStages.size(), CRC::CRC_32(),
                                 createInfo.pipelineLayout->getHash());
    hash = CRC::Calculate(shaderGroups.data(), sizeof(VkRayTracingShaderGroupCreateInfoKHR) * shaderGroups.size(), CRC::CRC_32(), hash);
    if (rayTracingPipelines.contains(hash)) {
        return rayTracingPipelines[hash];
    }
    VkRayTracingPipelineCreateInfoKHR pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .stageCount = static_cast<uint32>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .groupCount = static_cast<uint32>(shaderGroups.size()),
        .pGroups = shaderGroups.data(),
        .maxPipelineRayRecursionDepth = 12,
        .layout = createInfo.pipelineLayout.cast<PipelineLayout>()->getHandle(),
    };
    VkPipeline pipelineHandle;
    auto beginTime = std::chrono::high_resolution_clock::now();
    VK_CHECK(vkCreateRayTracingPipelinesKHR(graphics->getDevice(), VK_NULL_HANDLE, cache, 1, &pipelineInfo, nullptr, &pipelineHandle));
    auto endTime = std::chrono::high_resolution_clock::now();
    int64 delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - beginTime).count();
    std::cout << "RT creation time: " << delta << std::endl;

    const uint32_t handleSize = graphics->getRayTracingProperties().shaderGroupHandleSize;
    const uint32_t handleSizeAligned =
        align(graphics->getRayTracingProperties().shaderGroupHandleSize, graphics->getRayTracingProperties().shaderGroupHandleAlignment);
    const uint32_t handleAlignment = graphics->getRayTracingProperties().shaderGroupHandleAlignment;
    const uint32_t sbtAlignment = graphics->getRayTracingProperties().shaderGroupBaseAlignment;
    const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
    const uint32_t sbtSize = groupCount * handleSizeAligned;
    const VkBufferUsageFlags sbtBufferUsage =
        VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    const VmaMemoryUsage sbtMemoryUsage = VMA_MEMORY_USAGE_AUTO;

    uint64 rayGenStride = align<uint64>(handleSize + createInfo.rayGenGroup.parameters.size(), handleAlignment);
    uint64 hitStride = handleSize;
    for (const auto& h : createInfo.hitGroups) {
        hitStride = std::max(hitStride, align<uint64>(handleSize + h.parameters.size(), handleAlignment));
    }
    uint64 missStride = handleSize;
    for (const auto& m : createInfo.missGroups) {
        missStride = std::max(missStride, align<uint64>(handleSize + m.parameters.size(), handleAlignment));
    }
    OBufferAllocation rayGenBuffer = new BufferAllocation(graphics, "RayGenSBT",
                                                          VkBufferCreateInfo{
                                                              .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                                              .pNext = nullptr,
                                                              .flags = 0,
                                                              .size = rayGenStride,
                                                              .usage = sbtBufferUsage,
                                                          },
                                                          VmaAllocationCreateInfo{
                                                              .usage = sbtMemoryUsage,
                                                          },
                                                          Gfx::QueueType::GRAPHICS, sbtAlignment);

    OBufferAllocation hitBuffer = new BufferAllocation(graphics, "HitSBT",
                                                       VkBufferCreateInfo{
                                                           .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                                           .pNext = nullptr,
                                                           .flags = 0,
                                                           .size = hitStride * createInfo.hitGroups.size(),
                                                           .usage = sbtBufferUsage,
                                                       },
                                                       VmaAllocationCreateInfo{
                                                           .usage = sbtMemoryUsage,
                                                       },
                                                       Gfx::QueueType::GRAPHICS, sbtAlignment);

    OBufferAllocation missBuffer = new BufferAllocation(graphics, "MissSBT",
                                                        VkBufferCreateInfo{
                                                            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                                            .pNext = nullptr,
                                                            .flags = 0,
                                                            .size = missStride * createInfo.missGroups.size(),
                                                            .usage = sbtBufferUsage,
                                                        },
                                                        VmaAllocationCreateInfo{
                                                            .usage = sbtMemoryUsage,
                                                        },
                                                        Gfx::QueueType::GRAPHICS, sbtAlignment);

    Array<uint8> sbt(sbtSize);
    vkGetRayTracingShaderGroupHandlesKHR(graphics->getDevice(), pipelineHandle, 0, (uint32)shaderGroups.size(), sbtSize, sbt.data());

    uint64 sbtOffset = 0;
    Array<uint8> rayGenSbt(rayGenStride);
    std::memcpy(rayGenSbt.data(), sbt.data() + sbtOffset, handleSize);
    std::memcpy(rayGenSbt.data() + handleSize, createInfo.rayGenGroup.parameters.data(), createInfo.rayGenGroup.parameters.size());
    sbtOffset += handleSizeAligned;
    rayGenBuffer->updateContents(0, rayGenSbt.size(), rayGenSbt.data());
    rayGenBuffer->pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                                  VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

    Array<uint8> hitSbt(hitStride * createInfo.hitGroups.size());
    for (uint64 i = 0; i < createInfo.hitGroups.size(); ++i) {
        std::memcpy(hitSbt.data() + i * hitStride, sbt.data() + sbtOffset, handleSize);
        std::memcpy(hitSbt.data() + i * hitStride + handleSize, createInfo.hitGroups[i].parameters.data(),
                    createInfo.hitGroups[i].parameters.size());
        sbtOffset += handleSizeAligned;
    }
    hitBuffer->updateContents(0, hitSbt.size(), hitSbt.data());
    hitBuffer->pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                               VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

    Array<uint8> missSbt(missStride * createInfo.missGroups.size());
    for (uint64 i = 0; i < createInfo.missGroups.size(); ++i) {
        std::memcpy(missSbt.data() + i * missStride, sbt.data() + sbtOffset, handleSize);
        std::memcpy(missSbt.data() + i * missStride + handleSize, createInfo.missGroups[i].parameters.data(),
                    createInfo.missGroups[i].parameters.size());
        sbtOffset += handleSizeAligned;
    }
    missBuffer->updateContents(0, missSbt.size(), missSbt.data());
    missBuffer->pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                                VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

    ORayTracingPipeline pipeline =
        new RayTracingPipeline(graphics, pipelineHandle, std::move(rayGenBuffer), rayGenStride, std::move(hitBuffer), hitStride,
                               std::move(missBuffer), missStride, nullptr, 0, createInfo.pipelineLayout);
    PRayTracingPipeline handle = pipeline;
    rayTracingPipelines[hash] = std::move(pipeline);
    return handle;
}
