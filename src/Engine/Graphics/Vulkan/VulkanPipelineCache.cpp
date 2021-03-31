#include "VulkanPipelineCache.h"
#include "VulkanGraphics.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanInitializer.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptorSets.h"
#include "VulkanShader.h"
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

struct PipelineCreateHashStruct
{
    uint32 vertexHash;
    uint32 controlHash;
    uint32 evalHash;
    uint32 geometryHash;
    uint32 fragmentHash;
    uint32 pipelineLayoutHash;
    VkPipelineTessellationStateCreateInfo tess;
    VkVertexInputAttributeDescription attribs[16];
    VkVertexInputBindingDescription bindings[16];
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineViewportStateCreateInfo viewport;
    VkPipelineRasterizationStateCreateInfo rasterization;
    VkPipelineMultisampleStateCreateInfo multisample;
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    VkPipelineColorBlendAttachmentState blendAttachments[16];
    VkPipelineColorBlendStateCreateInfo blendState;
};

PGraphicsPipeline PipelineCache::createPipeline(const GraphicsPipelineCreateInfo& gfxInfo)
{
    VkGraphicsPipelineCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = 0;
    createInfo.flags = 0;
    createInfo.stageCount = 0;

    VkPipelineTessellationStateCreateInfo tessInfo;
    std::memset(&tessInfo, 0, sizeof(VkPipelineTessellationStateCreateInfo));
    VkPipelineShaderStageCreateInfo stageInfos[5];
    std::memset(stageInfos, 0, sizeof(stageInfos));
    PipelineCreateHashStruct hashStruct;
    std::memset(&hashStruct, 0, sizeof(PipelineCreateHashStruct));
    
    PVertexShader vertexShader = gfxInfo.vertexShader.cast<VertexShader>();
    VkPipelineShaderStageCreateInfo& vertInfo = stageInfos[createInfo.stageCount++];
    vertInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertInfo.module = vertexShader->getModuleHandle();
    vertInfo.pName = vertexShader->getEntryPointName();
    hashStruct.vertexHash = vertexShader->getShaderHash();

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

        hashStruct.controlHash = control->getShaderHash();
        hashStruct.evalHash = eval->getShaderHash();
        hashStruct.tess = tessInfo;
    }
    if(gfxInfo.geometryShader != nullptr)
    {
        PGeometryShader geometry = gfxInfo.geometryShader.cast<GeometryShader>();
        
        VkPipelineShaderStageCreateInfo& geometryInfo = stageInfos[createInfo.stageCount++];
        geometryInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        geometryInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        geometryInfo.module = geometry->getModuleHandle();
        geometryInfo.pName = geometry->getEntryPointName();

        hashStruct.geometryHash = geometry->getShaderHash();
    }
    if(gfxInfo.fragmentShader != nullptr)
    {
        PFragmentShader fragment = gfxInfo.fragmentShader.cast<FragmentShader>();

        VkPipelineShaderStageCreateInfo& fragmentInfo = stageInfos[createInfo.stageCount++];
        fragmentInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentInfo.module = fragment->getModuleHandle();
        fragmentInfo.pName = fragment->getEntryPointName();

        hashStruct.fragmentHash = fragment->getShaderHash();
    }
    VkPipelineVertexInputStateCreateInfo vertexInput =
        init::PipelineVertexInputStateCreateInfo();
    PVertexDeclaration vertexDecl = gfxInfo.vertexDeclaration;
    auto vertexStreams = vertexDecl->elementList;
    uint32 bindingNum = 0;
    uint32 bindingsMask = 0;
    uint32 attributesNum = 0;
    Array<VkVertexInputBindingDescription> bindings;
    Array<VkVertexInputAttributeDescription> attributes;
    Map<uint32, uint32> bindingToStream;
    Map<uint32, uint32> streamToBinding;
    assert(DEFAULT_ALLOC_SIZE == 16);
    std::memset(bindings.data(), 0, sizeof(VkVertexInputBindingDescription) * 16);
    std::memset(attributes.data(), 0, sizeof(VkVertexInputAttributeDescription) * 16);
    for(auto& element : vertexStreams)
    {
        //if((1 << element.attributeIndex) & vertexAttributeMask) // TODO: attribute mask
        {
            if(element.streamIndex >= bindings.size())
            {
                bindings.resize(element.streamIndex + 1); // This should not cause any actual allocations
            }
            VkVertexInputBindingDescription& currBinding = bindings[element.streamIndex];
            if((bindingsMask & (1 << element.streamIndex)) != 0)
            {
                assert(currBinding.binding == element.streamIndex);
                assert(currBinding.inputRate == element.bInstanced ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX);
                assert(currBinding.stride == element.stride);
            }
            else
            {
                assert(currBinding.binding == 0 && currBinding.inputRate == 0 && currBinding.stride == 0);
                currBinding.binding = element.streamIndex;
                currBinding.inputRate = element.bInstanced ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
                currBinding.stride = element.stride;

                bindingsMask |= 1 << element.streamIndex;
            }
        }
    }

    for(uint32 i = 0; i < bindings.size(); ++i)
    {
        if(!((1 << i) & bindingsMask))
        {
            continue;
        }

        bindingToStream[bindingNum] = i;
        streamToBinding[i] = bindingNum;
        VkVertexInputBindingDescription& currBinding = bindings[bindingNum];
        currBinding = bindings[i];
        currBinding.binding = bindingNum;
        bindingNum++;
    }

    for(auto& element : vertexStreams)
    {
        //TODO: vertex attribute mask
        if(attributesNum >= attributes.size())
        {
            attributes.resize(attributesNum + 1); // This should not cause any actual allocations
        }

        VkVertexInputAttributeDescription& currAttribute = attributes[attributesNum++];
        currAttribute.location = element.attributeIndex;
        currAttribute.binding = streamToBinding[element.streamIndex];
        currAttribute.format = cast(element.vertexFormat);
        currAttribute.offset = element.offset;
    }

    std::memcpy(hashStruct.bindings, bindings.data(), bindings.size() * sizeof(VkVertexInputBindingDescription));
    std::memcpy(hashStruct.attribs, attributes.data(), attributes.size() * sizeof(VkVertexInputAttributeDescription));

    vertexInput.pVertexBindingDescriptions = bindings.data();
    vertexInput.vertexBindingDescriptionCount = (uint32)bindings.size();
    vertexInput.pVertexAttributeDescriptions = attributes.data();
    vertexInput.vertexAttributeDescriptionCount = (uint32)attributes.size();

    VkPipelineInputAssemblyStateCreateInfo assemblyInfo =
        init::PipelineInputAssemblyStateCreateInfo(
            cast(gfxInfo.topology),
            0,
            false
        );
    hashStruct.inputAssembly = assemblyInfo;

    VkPipelineViewportStateCreateInfo viewportInfo =
        init::PipelineViewportStateCreateInfo(
            1,
            1,
            0
        );
    hashStruct.viewport = viewportInfo;

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

    hashStruct.rasterization = rasterizationState;

    VkPipelineMultisampleStateCreateInfo multisampleState =
        init::PipelineMultisampleStateCreateInfo(
            (VkSampleCountFlagBits)gfxInfo.multisampleState.samples, 
            0);
    multisampleState.alphaToCoverageEnable = gfxInfo.multisampleState.alphaCoverageEnable;
    multisampleState.alphaToOneEnable = gfxInfo.multisampleState.alphaToOneEnable;
    multisampleState.minSampleShading = gfxInfo.multisampleState.minSampleShading;
    multisampleState.sampleShadingEnable = gfxInfo.multisampleState.sampleShadingEnable;

    hashStruct.multisample = multisampleState;

    VkPipelineDepthStencilStateCreateInfo depthStencilState =
        init::PipelineDepthStencilStateCreateInfo(
            gfxInfo.depthStencilState.depthTestEnable,
            gfxInfo.depthStencilState.depthWriteEnable,
            cast(gfxInfo.depthStencilState.depthCompareOp)
        );

    hashStruct.depthStencil = depthStencilState;
    
    const auto& colorAttachments = gfxInfo.renderPass->getLayout()->colorAttachments;
    Array<VkPipelineColorBlendAttachmentState> blendAttachments(colorAttachments.size());
    for(uint32 i = 0; i < colorAttachments.size(); ++i)
    {
        const Gfx::ColorBlendState::BlendAttachment& attachment = gfxInfo.colorBlend.blendAttachments[i];
        VkPipelineColorBlendAttachmentState& blendAttachment = blendAttachments[i];
        blendAttachment.alphaBlendOp = (VkBlendOp)attachment.alphaBlendOp;
        blendAttachment.blendEnable = attachment.blendEnable;
        blendAttachment.colorBlendOp = (VkBlendOp)attachment.colorBlendOp;
        blendAttachment.colorWriteMask = colorAttachments[i]->componentFlags;
        blendAttachment.dstAlphaBlendFactor = (VkBlendFactor)attachment.dstAlphaBlendFactor;
        blendAttachment.srcAlphaBlendFactor = (VkBlendFactor)attachment.srcAlphaBlendFactor;
        blendAttachment.dstColorBlendFactor = (VkBlendFactor)attachment.dstColorBlendFactor;
        blendAttachment.srcColorBlendFactor = (VkBlendFactor)attachment.srcColorBlendFactor;

        hashStruct.blendAttachments[i] = blendAttachment;
    }
    VkPipelineColorBlendStateCreateInfo blendState = 
        init::PipelineColorBlendStateCreateInfo(
            (uint32)blendAttachments.size(),
            blendAttachments.data()
        );
    blendState.logicOpEnable = gfxInfo.colorBlend.logicOpEnable;
    blendState.logicOp = (VkLogicOp)gfxInfo.colorBlend.logicOp;
    std::memcpy(blendState.blendConstants, gfxInfo.colorBlend.blendConstants, sizeof(float)*4);

    hashStruct.blendState = blendState;
    hashStruct.blendState.pAttachments = nullptr;

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
    hashStruct.pipelineLayoutHash = layout->getHash();

    boost::crc_32_type crc;
    crc.process_bytes(&hashStruct, sizeof(PipelineCreateHashStruct));
    uint32 hash = crc.checksum();
    VkPipeline pipelineHandle;

    auto foundPipeline = createdPipelines.find(hash);
    if (foundPipeline != createdPipelines.end())
    {
        pipelineHandle = foundPipeline->value;
    }
    else
    {
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
        createInfo.layout = layout->getHandle();
        createInfo.subpass = 0;

        auto beginTime = std::chrono::high_resolution_clock::now();
        VK_CHECK(vkCreateGraphicsPipelines(graphics->getDevice(), cache, 1, &createInfo, nullptr, &pipelineHandle));
        auto endTime = std::chrono::high_resolution_clock::now();
        int64 delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - beginTime).count();
        createdPipelines[hash] = pipelineHandle;
        std::cout << "Gfx creation time: " << delta << std::endl;
    }

    PGraphicsPipeline result = new GraphicsPipeline(graphics, pipelineHandle, layout, gfxInfo);
    return result;
}