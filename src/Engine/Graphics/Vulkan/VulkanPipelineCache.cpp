#include "VulkanPipelineCache.h"
#include "VulkanGraphics.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanInitializer.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptorSets.h"
#include "VulkanShader.h"

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
}

PGraphicsPipeline PipelineCache::createPipeline(const GraphicsPipelineCreateInfo& gfxInfo)
{
    VkGraphicsPipelineCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = 0;
    createInfo.flags = 0;
    createInfo.stageCount = 0;
    VkPipelineTessellationStateCreateInfo tessInfo;
    VkPipelineShaderStageCreateInfo stageInfos[5];
    std::memset(stageInfos, 0, sizeof(stageInfos));
    if(gfxInfo.vertexShader != nullptr)
    {
        PVertexShader shader = gfxInfo.vertexShader.cast<VertexShader>();
        VkPipelineShaderStageCreateInfo& vertInfo = stageInfos[createInfo.stageCount++];
        vertInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertInfo.module = shader->getModuleHandle();
        vertInfo.pName = shader->getEntryPointName();
    }
    if(gfxInfo.controlShader != nullptr)
    {
        assert(gfxInfo.evalShader != nullptr);
        PControlShader control = gfxInfo.controlShader.cast<ControlShader>();
        PEvaluationShader eval = gfxInfo.evalShader.cast<EvaluationShader>();
        
        VkPipelineShaderStageCreateInfo& controlInfo = stageInfos[createInfo.stageCount++];
        controlInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        controlInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        controlInfo.module = control->getModuleHandle();
        controlInfo.pName = control->getEntryPointName();

        VkPipelineShaderStageCreateInfo& evalInfo = stageInfos[createInfo.stageCount++];
        evalInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        evalInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        evalInfo.module = eval->getModuleHandle();
        evalInfo.pName = eval->getEntryPointName();

        tessInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessInfo.pNext = 0;
        tessInfo.flags = 0;
        tessInfo.patchControlPoints = control->getNumPatches();
    }
    if(gfxInfo.geometryShader != nullptr)
    {
        PGeometryShader geometry = gfxInfo.geometryShader.cast<GeometryShader>();
        
        VkPipelineShaderStageCreateInfo& geometryInfo = stageInfos[createInfo.stageCount++];
        geometryInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        geometryInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        geometryInfo.module = geometry->getModuleHandle();
        geometryInfo.pName = geometry->getEntryPointName();
    }
    if(gfxInfo.fragmentShader != nullptr)
    {
        PFragmentShader fragment = gfxInfo.fragmentShader.cast<FragmentShader>();

        VkPipelineShaderStageCreateInfo& fragmentInfo = stageInfos[createInfo.stageCount++];
        fragmentInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentInfo.module = fragment->getModuleHandle();
        fragmentInfo.pName = fragment->getEntryPointName();
    }
    VkPipelineVertexInputStateCreateInfo vertexInput =
        init::PipelineVertexInputStateCreateInfo();
    Gfx::PVertexDeclaration vertexDecl = gfxInfo.vertexDeclaration;
    auto vertexStreams = vertexDecl->getVertexStreams();
    Array<VkVertexInputBindingDescription> bindingDesc(vertexStreams.size());
    Array<VkVertexInputAttributeDescription> attribDesc;
    uint32 bindingNum = 0;
    for(auto vertexBinding : vertexStreams)
    {
        uint32 stride = vertexBinding.getVertexBuffer()->getVertexSize();
        for(auto vertexAttrib : vertexBinding.getVertexDescriptions())
        {
            auto attrib = attribDesc.add();
            attrib.binding = bindingNum;
            attrib.format = cast(vertexAttrib.vertexFormat);
            attrib.location = vertexAttrib.location;
            attrib.offset = vertexAttrib.offset;
        }
        bindingDesc[bindingNum].binding = bindingNum;
        bindingDesc[bindingNum].stride = stride;
        bindingDesc[bindingNum].inputRate = vertexBinding.isInstanced() ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
        bindingNum++;
    }
    vertexInput.pVertexBindingDescriptions = bindingDesc.data();
    vertexInput.vertexBindingDescriptionCount = bindingDesc.size();
    vertexInput.pVertexAttributeDescriptions = attribDesc.data();
    vertexInput.vertexAttributeDescriptionCount = attribDesc.size();

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
        init::PipelineMultisampleStateCreateInfo((VkSampleCountFlagBits)gfxInfo.multisampleState.samples, 0);
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
    
    const auto colorAttachments = gfxInfo.renderPass->getLayout()->colorAttachments;
    Array<VkPipelineColorBlendAttachmentState> blendAttachments(colorAttachments.size());
    for(uint32 i = 0; i < colorAttachments.size(); ++i)
    {
        const Gfx::ColorBlendState::BlendAttachment& attachment = gfxInfo.colorBlend.blendAttachments[i];
        VkPipelineColorBlendAttachmentState& blendAttachment = blendAttachments[i];
        blendAttachment.alphaBlendOp = (VkBlendOp)attachment.alphaBlendOp;
        blendAttachment.blendEnable = attachment.blendEnable;
        blendAttachment.colorBlendOp = (VkBlendOp)attachment.colorBlendOp;
        blendAttachment.colorWriteMask = attachment.colorWriteMask;
        blendAttachment.dstAlphaBlendFactor = (VkBlendFactor)attachment.dstAlphaBlendFactor;
        blendAttachment.srcAlphaBlendFactor = (VkBlendFactor)attachment.srcAlphaBlendFactor;
        blendAttachment.dstColorBlendFactor = (VkBlendFactor)attachment.dstColorBlendFactor;
        blendAttachment.srcColorBlendFactor = (VkBlendFactor)attachment.srcColorBlendFactor;
    }
    VkPipelineColorBlendStateCreateInfo blendState = 
        init::PipelineColorBlendStateCreateInfo(
            blendAttachments.size(),
            blendAttachments.data()
        );
    blendState.logicOpEnable = gfxInfo.colorBlend.logicOpEnable;
    blendState.logicOp = (VkLogicOp)gfxInfo.colorBlend.logicOp;
    std::memcpy(blendState.blendConstants, gfxInfo.colorBlend.blendConstants, sizeof(float)*4);

    uint32 numDynamicEnabled = 0;
    StaticArray<VkDynamicState, VK_DYNAMIC_STATE_RANGE_SIZE> dynamicEnabled;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicEnabled[numDynamicEnabled++] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDynamicStateCreateInfo dynamicState =
        init::PipelineDynamicStateCreateInfo(
            dynamicEnabled.data(),
            numDynamicEnabled,
            0
        );

    createInfo.pStages = stageInfos;
    createInfo.pVertexInputState = &vertexInput;
    createInfo.pInputAssemblyState = &assemblyInfo;
    createInfo.pTessellationState = &tessInfo;
    createInfo.pViewportState = &viewportInfo;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pDepthStencilState = &depthStencilState;
    createInfo.pColorBlendState = &blendState;
    createInfo.pDynamicState = &dynamicState;
    createInfo.renderPass = gfxInfo.renderPass.cast<RenderPass>()->getHandle();
    createInfo.layout = gfxInfo.pipelineLayout.cast<PipelineLayout>()->getHandle();
    createInfo.subpass = 0;
    
    VkPipeline pipelineHandle;
    auto beginTime = std::chrono::high_resolution_clock::now();
    VK_CHECK(vkCreateGraphicsPipelines(graphics->getDevice(), cache, 1, &createInfo, nullptr, &pipelineHandle));
    auto endTime = std::chrono::high_resolution_clock::now();
    int64 delta = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count();
    std::cout << "Gfx creation time: " << delta << std::endl;

    PGraphicsPipeline result = new GraphicsPipeline(graphics, pipelineHandle, gfxInfo.pipelineLayout.cast<PipelineLayout>(), gfxInfo);
    return result;
}