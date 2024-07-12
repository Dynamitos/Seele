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
    std::ifstream stream(cacheFilePath, std::ios::binary | std::ios::ate);
    VkPipelineCacheCreateInfo cacheCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .initialDataSize = 0,
    };
    if (stream.good()) {
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
    PPipelineLayout layout = Gfx::PPipelineLayout(gfxInfo.pipelineLayout).cast<PipelineLayout>();
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
    uint32 hash = CRC::Calculate(&gfxInfo, sizeof(Gfx::MeshPipelineCreateInfo), CRC::CRC_32());
    if (graphicsPipelines.contains(hash)) {
        return graphicsPipelines[hash];
    }
    PPipelineLayout layout = Gfx::PPipelineLayout(gfxInfo.pipelineLayout).cast<PipelineLayout>();
    // uint32 hash = layout->getHash();
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
    // hash = CRC::Calculate(stageInfos, sizeof(stageInfos), CRC::CRC_32(), hash);

    VkPipelineViewportStateCreateInfo viewportInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };
    // hash = CRC::Calculate(&viewportInfo, sizeof(VkPipelineViewportStateCreateInfo), CRC::CRC_32(), hash);
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
    // hash = CRC::Calculate(&rasterizationState, sizeof(VkPipelineRasterizationStateCreateInfo), CRC::CRC_32(), hash);

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
    // hash = CRC::Calculate(&multisampleState, sizeof(VkPipelineMultisampleStateCreateInfo), CRC::CRC_32(), hash);

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
    // hash = CRC::Calculate(&depthStencilState, sizeof(VkPipelineDepthStencilStateCreateInfo), CRC::CRC_32(), hash);

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
    // hash = CRC::Calculate(blendAttachments.data(), blendAttachments.size() * sizeof(VkPipelineColorBlendAttachmentState), CRC::CRC_32(),
    // hash);

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
    // hash = CRC::Calculate(dynamicEnabled.data(), sizeof(dynamicEnabled), CRC::CRC_32(), hash);

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

PComputePipeline PipelineCache::createPipeline(Gfx::ComputePipelineCreateInfo computeInfo) {
    PPipelineLayout layout = Gfx::PPipelineLayout(computeInfo.pipelineLayout).cast<PipelineLayout>();
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
            .pName = "main",
        });
        shaderGroups.add(VkRayTracingShaderGroupCreateInfoKHR{
            .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
            .pNext = nullptr,
            .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
            .generalShader = static_cast<uint32>(shaderStages.size() - 1),
            .closestHitShader = VK_SHADER_UNUSED_KHR,
            .anyHitShader = VK_SHADER_UNUSED_KHR,
            .intersectionShader = VK_SHADER_UNUSED_KHR,
        });
    }
    {
        for (auto hitgroup : createInfo.hitGroups) {
            auto hit = hitgroup.closestHitShader.cast<ClosestHitShader>();
            shaderStages.add(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
                .module = hit->getModuleHandle(),
                .pName = "main",
            });
            uint32 hitIndex = static_cast<uint32>(shaderStages.size() - 1);
            uint32 anyHitIndex = VK_SHADER_UNUSED_KHR;
            uint32 intersectionIndex = VK_SHADER_UNUSED_KHR;
            if (hitgroup.anyHitShader != nullptr) {
                auto anyHit = hitgroup.anyHitShader.cast<AnyHitShader>();
                shaderStages.add(VkPipelineShaderStageCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
                    .module = anyHit->getModuleHandle(),
                    .pName = "main",
                });
            }
            if (hitgroup.intersectionShader != nullptr) {
                auto intersect = hitgroup.intersectionShader.cast<IntersectionShader>();
                shaderStages.add(VkPipelineShaderStageCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
                    .module = intersect->getModuleHandle(),
                    .pName = intersect->getEntryPointName(),
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
            });
            shaderGroups.add(VkRayTracingShaderGroupCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
                .pNext = nullptr,
                .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
                .generalShader = static_cast<uint32>(shaderStages.size() - 1),
                .closestHitShader = VK_SHADER_UNUSED_KHR,
                .anyHitShader = VK_SHADER_UNUSED_KHR,
                .intersectionShader = VK_SHADER_UNUSED_KHR,
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
            });
            shaderGroups.add(VkRayTracingShaderGroupCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
                .pNext = nullptr,
                .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
                .generalShader = static_cast<uint32>(shaderStages.size() - 1),
                .closestHitShader = VK_SHADER_UNUSED_KHR,
                .anyHitShader = VK_SHADER_UNUSED_KHR,
                .intersectionShader = VK_SHADER_UNUSED_KHR,
            });
        }
    }
    uint32 hash = CRC::Calculate(shaderStages.data(), sizeof(VkPipelineShaderStageCreateInfo) * shaderStages.size(), CRC::CRC_32());
    hash = CRC::Calculate(shaderGroups.data(), sizeof(VkRayTracingShaderGroupCreateInfoKHR) * shaderGroups.size(), CRC::CRC_32(), hash);
    VkRayTracingPipelineCreateInfoKHR pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .stageCount = static_cast<uint32>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .groupCount = static_cast<uint32>(shaderGroups.size()),
        .pGroups = shaderGroups.data(),
        .maxPipelineRayRecursionDepth = 24,
        .layout = createInfo.pipelineLayout.cast<PipelineLayout>()->getHandle(),
    };
    VkPipeline pipelineHandle;
    VK_CHECK(vkCreateRayTracingPipelinesKHR(graphics->getDevice(), VK_NULL_HANDLE, cache, 1, &pipelineInfo, nullptr, &pipelineHandle));

    const uint32_t handleSize = graphics->getRayTracingProperties().shaderGroupHandleSize;
    const uint32_t handleAlignment = graphics->getRayTracingProperties().shaderGroupHandleAlignment;
    const uint32_t handleSizeAligned = align(handleSize, handleAlignment);
    const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
    const uint32_t sbtSize = handleSizeAligned * groupCount;

    Array<uint8> sbt(sbtSize);

    vkGetRayTracingShaderGroupHandlesKHR(graphics->getDevice(), pipelineHandle, 0, shaderGroups.size(), sbtSize, sbt.data());

    uint32 rayGenSize = align<uint32>(handleSize + createInfo.rayGenGroup.parameters.size(), handleAlignment);
    Array<uint8> rayGenSbt(handleSizeAligned);
    std::memcpy(rayGenSbt.data(), sbt.data(), handleSize);
    std::memcpy(rayGenSbt.data() + handleSize, createInfo.rayGenGroup.parameters.data(), createInfo.rayGenGroup.parameters.size());

    uint64 sbtOffset = handleSizeAligned;

    uint32 maxParamSize = 0;
    for (auto& hitgroup : createInfo.hitGroups) {
        maxParamSize = std::max<uint32>(maxParamSize, hitgroup.parameters.size());
    }
    uint64 hitStride = align(handleSize + maxParamSize, handleAlignment);
    Array<uint8> hitSbt(hitStride * createInfo.hitGroups.size());
    for (uint64 i = 0; i < createInfo.hitGroups.size(); ++i) {
        std::memcpy(hitSbt.data() + i * hitStride, sbt.data() + sbtOffset, handleSize);
        std::memcpy(hitSbt.data() + i * hitStride + handleSize, createInfo.hitGroups[i].parameters.data(),
                    createInfo.hitGroups[i].parameters.size());
        sbtOffset += handleSizeAligned;
    }
    
    maxParamSize = 0;
    for (auto& missGroup : createInfo.missGroups) {
        maxParamSize = std::max<uint32>(maxParamSize, missGroup.parameters.size());
    }
    uint64 missStride = align(handleSize + maxParamSize, handleAlignment);
    Array<uint8> missSbt(missStride * createInfo.missGroups.size());
    for (uint64 i = 0; i < createInfo.missGroups.size(); ++i) {
        std::memcpy(missSbt.data() + i * missStride, sbt.data() + sbtOffset, handleSize);
        std::memcpy(missSbt.data() + i * missStride + handleSize, createInfo.missGroups[i].parameters.data(),
                    createInfo.missGroups[i].parameters.size());
        sbtOffset += handleSizeAligned;
    }

    maxParamSize = 0;
    for (auto& callableGroup : createInfo.callableGroups) {
        maxParamSize = std::max<uint32>(maxParamSize, callableGroup.parameters.size());
    }
    uint64 callableStride = align(handleSize + maxParamSize, handleAlignment);
    Array<uint8> callableSbt(callableStride * createInfo.callableGroups.size());
    for (uint64 i = 0; i < createInfo.callableGroups.size(); ++i) {
        std::memcpy(callableSbt.data() + i * callableStride, sbt.data() + sbtOffset, handleSize);
        std::memcpy(callableSbt.data() + i * callableStride + handleSize, createInfo.callableGroups[i].parameters.data(),
                    createInfo.callableGroups[i].parameters.size());
        sbtOffset += handleSizeAligned;
    }

    OBufferAllocation rayGenBuffer =
        new BufferAllocation(graphics, "RayGenSBT",
                             VkBufferCreateInfo{
                                 .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .size = rayGenSbt.size(),
                                 .usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             },
                             VmaAllocationCreateInfo{
                                 .usage = VMA_MEMORY_USAGE_AUTO,
                             },
                             Gfx::QueueType::GRAPHICS);
    rayGenBuffer->updateContents(0, rayGenSbt.size(), rayGenSbt.data());
    rayGenBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                  Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

    OBufferAllocation hitBuffer =
        new BufferAllocation(graphics, "HitSBT",
                             VkBufferCreateInfo{
                                 .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .size = hitSbt.size(),
                                 .usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             },
                             VmaAllocationCreateInfo{
                                 .usage = VMA_MEMORY_USAGE_AUTO,
                             },
                             Gfx::QueueType::GRAPHICS);
    hitBuffer->updateContents(0, hitSbt.size(), hitSbt.data());
    hitBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                  Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

    OBufferAllocation missBuffer =
        new BufferAllocation(graphics, "MissSBT",
                             VkBufferCreateInfo{
                                 .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .size = missSbt.size(),
                                 .usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             },
                             VmaAllocationCreateInfo{
                                 .usage = VMA_MEMORY_USAGE_AUTO,
                             },
                             Gfx::QueueType::GRAPHICS);
    missBuffer->updateContents(0, missSbt.size(), missSbt.data());
    missBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                  Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);


    OBufferAllocation callableBuffer =
        new BufferAllocation(graphics, "CallableSBT",
                             VkBufferCreateInfo{
                                 .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .size = callableSbt.size(),
                                 .usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             },
                             VmaAllocationCreateInfo{
                                 .usage = VMA_MEMORY_USAGE_AUTO,
                             },
                             Gfx::QueueType::GRAPHICS);
    callableBuffer->updateContents(0, callableSbt.size(), callableSbt.data());
    callableBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                Gfx::SE_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);


    ORayTracingPipeline pipeline =
        new RayTracingPipeline(graphics, pipelineHandle, std::move(rayGenBuffer), rayGenSize, std::move(hitBuffer), hitStride,
                               std::move(missBuffer), missStride, std::move(callableBuffer), callableStride, createInfo.pipelineLayout);
    PRayTracingPipeline handle = pipeline;
    rayTracingPipelines[hash] = std::move(pipeline);
    return handle;
}
