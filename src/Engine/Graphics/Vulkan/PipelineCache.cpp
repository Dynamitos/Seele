#include "PipelineCache.h"
#include "Graphics.h"
#include "Enums.h"
#include "Initializer.h"
#include "RenderPass.h"
#include "DescriptorSets.h"
#include "Shader.h"
#include <fstream>

using namespace Seele;
using namespace Seele::Vulkan;

PipelineCache::PipelineCache(PGraphics graphics, const std::string& cacheFilePath)
    : graphics(graphics)
    , cacheFile(cacheFilePath)
{
    std::ifstream stream(cacheFilePath, std::ios::binary | std::ios::ate);
    VkPipelineCacheCreateInfo cacheCreateInfo;
    cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    cacheCreateInfo.pNext = nullptr;
    cacheCreateInfo.flags = 0;
    cacheCreateInfo.initialDataSize = 0;
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
    VkDeviceSize cacheSize;
    vkGetPipelineCacheData(graphics->getDevice(), cache, &cacheSize, nullptr);
    Array<uint8> cacheData;
    vkGetPipelineCacheData(graphics->getDevice(), cache, &cacheSize, cacheData.data());
    std::ofstream stream(cacheFile, std::ios::binary);
    stream.write((char*)cacheData.data(), cacheSize);
    stream.flush();
    stream.close();
    vkDestroyPipelineCache(graphics->getDevice(), cache, nullptr);
    std::cout << "Written " << cacheSize << " bytes to cache" << std::endl;
}

PGraphicsPipeline PipelineCache::createPipeline(const Gfx::LegacyPipelineCreateInfo& gfxInfo)
{
    uint32 stageCount = 0;
    
    VkPipelineShaderStageCreateInfo stageInfos[2];
    std::memset(stageInfos, 0, sizeof(stageInfos));
    
    PVertexShader vertexShader = gfxInfo.vertexShader.cast<VertexShader>();
    stageInfos[stageCount++] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertexShader->getModuleHandle(),
        .pName = vertexShader->getEntryPointName(),
    };
    
    if(gfxInfo.fragmentShader != nullptr)
    {
        PFragmentShader fragment = gfxInfo.fragmentShader.cast<FragmentShader>();

        stageInfos[stageCount++] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragment->getModuleHandle(),
            .pName = fragment->getEntryPointName(),
        };
    }
    VkPipelineVertexInputStateCreateInfo vertexInput =
        init::PipelineVertexInputStateCreateInfo();

    Array<VkVertexInputBindingDescription> bindings;
    Array<VkVertexInputAttributeDescription> attributes;
    if (gfxInfo.vertexDeclaration != nullptr)
    {
        PVertexDeclaration decl = gfxInfo.vertexDeclaration.cast<VertexDeclaration>();
        for (const auto& elem : decl->elementList)
        {
            attributes.add(VkVertexInputAttributeDescription{
                .location = elem.attributeIndex,
                .binding = elem.binding,
                .format = cast(elem.vertexFormat),
                .offset = elem.offset,
            });
            auto res = bindings.find([elem](const VkVertexInputBindingDescription& b) {return b.binding == elem.binding; });
            if (res == bindings.end())
            {
                bindings.add({});
                res = bindings.end();
            }
            *res = VkVertexInputBindingDescription{
                .binding = elem.binding,
                .stride = elem.stride,
                .inputRate = elem.bInstanced ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX,
            };
        }
        vertexInput.pVertexAttributeDescriptions = attributes.data();
        vertexInput.vertexAttributeDescriptionCount = attributes.size();
        vertexInput.pVertexBindingDescriptions = bindings.data();
        vertexInput.vertexBindingDescriptionCount = bindings.size();
    }

    VkPipelineInputAssemblyStateCreateInfo assemblyInfo =
        init::PipelineInputAssemblyStateCreateInfo(
            cast(gfxInfo.topology),
            0,
            false
        );
    VkPipelineViewportStateCreateInfo viewportInfo =
        init::PipelineViewportStateCreateInfo(
            1,
            1,
            0
        );
    VkPipelineRasterizationStateCreateInfo rasterizationState =
        init::PipelineRasterizationStateCreateInfo(
            cast(gfxInfo.rasterizationState.polygonMode),
            gfxInfo.rasterizationState.cullMode,
            (VkFrontFace)gfxInfo.rasterizationState.frontFace,
            0
        );
    rasterizationState.depthBiasEnable = gfxInfo.rasterizationState.depthBiasEnable;    
    rasterizationState.depthBiasClamp = gfxInfo.rasterizationState.depthBiasClamp;
    rasterizationState.depthBiasConstantFactor = gfxInfo.rasterizationState.depthBoasConstantFactor;
    rasterizationState.depthBiasSlopeFactor = gfxInfo.rasterizationState.depthBiasSlopeFactor;
    rasterizationState.depthClampEnable = gfxInfo.rasterizationState.depthClampEnable;
    rasterizationState.lineWidth = gfxInfo.rasterizationState.lineWidth;
    rasterizationState.rasterizerDiscardEnable = gfxInfo.rasterizationState.rasterizerDiscardEnable;

    VkPipelineMultisampleStateCreateInfo multisampleState =
        init::PipelineMultisampleStateCreateInfo(
            (VkSampleCountFlagBits)gfxInfo.multisampleState.samples, 
            0);
    multisampleState.alphaToCoverageEnable = gfxInfo.multisampleState.alphaCoverageEnable;
    multisampleState.alphaToOneEnable = gfxInfo.multisampleState.alphaToOneEnable;
    multisampleState.minSampleShading = gfxInfo.multisampleState.minSampleShading;
    multisampleState.sampleShadingEnable = gfxInfo.multisampleState.sampleShadingEnable;

    VkPipelineDepthStencilStateCreateInfo depthStencilState =
        init::PipelineDepthStencilStateCreateInfo(
            gfxInfo.depthStencilState.depthTestEnable,
            gfxInfo.depthStencilState.depthWriteEnable,
            cast(gfxInfo.depthStencilState.depthCompareOp)
        );

    const auto& colorAttachments = gfxInfo.renderPass->getLayout()->colorAttachments;
    Array<VkPipelineColorBlendAttachmentState> blendAttachments(colorAttachments.size());
    for(uint32 i = 0; i < colorAttachments.size(); ++i)
    {
        const Gfx::ColorBlendState::BlendAttachment& attachment = gfxInfo.colorBlend.blendAttachments[i];
        blendAttachments[i] = {
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
    VkPipelineColorBlendStateCreateInfo blendState = 
        init::PipelineColorBlendStateCreateInfo(
            (uint32)blendAttachments.size(),
            blendAttachments.data()
        );
    blendState.logicOpEnable = gfxInfo.colorBlend.logicOpEnable;
    blendState.logicOp = (VkLogicOp)gfxInfo.colorBlend.logicOp;
    std::memcpy(blendState.blendConstants, gfxInfo.colorBlend.blendConstants, sizeof(gfxInfo.colorBlend.blendConstants));

    uint32 numDynamicEnabled = 0;
    StaticArray<VkDynamicState, 2> dynamicEnabled;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDynamicStateCreateInfo dynamicState =
        init::PipelineDynamicStateCreateInfo(
            dynamicEnabled.data(),
            numDynamicEnabled,
            0
        );

    PPipelineLayout layout = gfxInfo.pipelineLayout.cast<PipelineLayout>();

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
    

    PGraphicsPipeline result = new GraphicsPipeline(graphics, pipelineHandle, layout);
    return result;
}


PGraphicsPipeline PipelineCache::createPipeline(const Gfx::MeshPipelineCreateInfo& gfxInfo)
{
    uint32 stageCount = 0;

    VkPipelineShaderStageCreateInfo stageInfos[3];
    std::memset(stageInfos, 0, sizeof(stageInfos));

    if (gfxInfo.taskShader != nullptr)
    {
        PTaskShader taskShader = gfxInfo.taskShader.cast<TaskShader>();
        stageInfos[stageCount++] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_TASK_BIT_EXT,
            .module = taskShader->getModuleHandle(),
            .pName = taskShader->getEntryPointName(),
        };
    }

    PMeshShader meshShader = gfxInfo.meshShader.cast<MeshShader>();
    stageInfos[stageCount++] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_MESH_BIT_EXT,
        .module = meshShader->getModuleHandle(),
        .pName = meshShader->getEntryPointName(),
    };

    if (gfxInfo.fragmentShader != nullptr)
    {
        PFragmentShader fragment = gfxInfo.fragmentShader.cast<FragmentShader>();

        stageInfos[stageCount++] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragment->getModuleHandle(),
            .pName = fragment->getEntryPointName(),
        };
    }
    VkPipelineViewportStateCreateInfo viewportInfo =
        init::PipelineViewportStateCreateInfo(
            1,
            1,
            0
        );
    VkPipelineRasterizationStateCreateInfo rasterizationState =
        init::PipelineRasterizationStateCreateInfo(
            cast(gfxInfo.rasterizationState.polygonMode),
            gfxInfo.rasterizationState.cullMode,
            (VkFrontFace)gfxInfo.rasterizationState.frontFace,
            0
        );
    rasterizationState.depthBiasEnable = gfxInfo.rasterizationState.depthBiasEnable;
    rasterizationState.depthBiasClamp = gfxInfo.rasterizationState.depthBiasClamp;
    rasterizationState.depthBiasConstantFactor = gfxInfo.rasterizationState.depthBoasConstantFactor;
    rasterizationState.depthBiasSlopeFactor = gfxInfo.rasterizationState.depthBiasSlopeFactor;
    rasterizationState.depthClampEnable = gfxInfo.rasterizationState.depthClampEnable;
    rasterizationState.lineWidth = gfxInfo.rasterizationState.lineWidth;
    rasterizationState.rasterizerDiscardEnable = gfxInfo.rasterizationState.rasterizerDiscardEnable;

    VkPipelineMultisampleStateCreateInfo multisampleState =
        init::PipelineMultisampleStateCreateInfo(
            (VkSampleCountFlagBits)gfxInfo.multisampleState.samples,
            0);
    multisampleState.alphaToCoverageEnable = gfxInfo.multisampleState.alphaCoverageEnable;
    multisampleState.alphaToOneEnable = gfxInfo.multisampleState.alphaToOneEnable;
    multisampleState.minSampleShading = gfxInfo.multisampleState.minSampleShading;
    multisampleState.sampleShadingEnable = gfxInfo.multisampleState.sampleShadingEnable;

    VkPipelineDepthStencilStateCreateInfo depthStencilState =
        init::PipelineDepthStencilStateCreateInfo(
            gfxInfo.depthStencilState.depthTestEnable,
            gfxInfo.depthStencilState.depthWriteEnable,
            cast(gfxInfo.depthStencilState.depthCompareOp)
        );

    const auto& colorAttachments = gfxInfo.renderPass->getLayout()->colorAttachments;
    Array<VkPipelineColorBlendAttachmentState> blendAttachments(colorAttachments.size());
    for (uint32 i = 0; i < colorAttachments.size(); ++i)
    {
        const Gfx::ColorBlendState::BlendAttachment& attachment = gfxInfo.colorBlend.blendAttachments[i];
        blendAttachments[i] = {
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
    VkPipelineColorBlendStateCreateInfo blendState =
        init::PipelineColorBlendStateCreateInfo(
            (uint32)blendAttachments.size(),
            blendAttachments.data()
        );
    blendState.logicOpEnable = gfxInfo.colorBlend.logicOpEnable;
    blendState.logicOp = (VkLogicOp)gfxInfo.colorBlend.logicOp;
    std::memcpy(blendState.blendConstants, gfxInfo.colorBlend.blendConstants, sizeof(gfxInfo.colorBlend.blendConstants));

    uint32 numDynamicEnabled = 0;
    StaticArray<VkDynamicState, 2> dynamicEnabled;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDynamicStateCreateInfo dynamicState =
        init::PipelineDynamicStateCreateInfo(
            dynamicEnabled.data(),
            numDynamicEnabled,
            0
        );

    PPipelineLayout layout = gfxInfo.pipelineLayout.cast<PipelineLayout>();

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


    PGraphicsPipeline result = new GraphicsPipeline(graphics, pipelineHandle, layout);
    return result;
}


PComputePipeline PipelineCache::createPipeline(const Gfx::ComputePipelineCreateInfo& computeInfo) 
{
    VkComputePipelineCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.pNext = 0;
    createInfo.flags = 0;
    createInfo.basePipelineIndex = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    auto layout = computeInfo.pipelineLayout.cast<PipelineLayout>();
    createInfo.layout = layout->getHandle();
    auto computeStage = computeInfo.computeShader.cast<ComputeShader>();
    createInfo.stage = init::PipelineShaderStageCreateInfo(
        VK_SHADER_STAGE_COMPUTE_BIT,
        computeStage->getModuleHandle(),
        computeStage->getEntryPointName());
    VkPipeline pipelineHandle;
    auto beginTime = std::chrono::high_resolution_clock::now();
    VK_CHECK(vkCreateComputePipelines(graphics->getDevice(), cache, 1, &createInfo, nullptr, &pipelineHandle));
    auto endTime = std::chrono::high_resolution_clock::now();
    int64 delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - beginTime).count();
    std::cout << "Compute creation time: " << delta << std::endl;
    PComputePipeline result = new ComputePipeline(graphics, pipelineHandle, layout);
    return result;
}