#include "PipelineCache.h"
#include "Descriptor.h"
#include "Enums.h"
#include "Metal/MTLLibrary.hpp"
#include "RenderPass.h"
#include "Foundation/NSError.hpp"
#include "Foundation/NSString.hpp"
#include "Graphics.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Enums.h"
#include "Graphics/Metal/Pipeline.h"
#include "Metal/MTLComputePipeline.hpp"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "Metal/MTLRenderPipeline.hpp"
#include "Metal/MTLVertexDescriptor.hpp"
#include "Shader.h"
#include "Texture.h"
#include <Foundation/Foundation.h>
#include <iostream>

using namespace Seele;
using namespace Seele::Metal;

PipelineCache::PipelineCache(PGraphics graphics, const std::string& name) : graphics(graphics), cacheFile(name) {}

PipelineCache::~PipelineCache() {}

PGraphicsPipeline PipelineCache::createPipeline(Gfx::LegacyPipelineCreateInfo createInfo) {
    PRenderPass renderPass = createInfo.renderPass.cast<RenderPass>();
    MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();

    MTL::VertexDescriptor* vertexDescriptor = pipelineDescriptor->vertexDescriptor()->init();
    MTL::VertexAttributeDescriptorArray* attributes = vertexDescriptor->attributes()->init();
    if (createInfo.vertexInput != nullptr) {
        const auto& vertexInfo = createInfo.vertexInput->getInfo();
        for (size_t attr = 0; attr < vertexInfo.attributes.size(); ++attr) {
            MTL::VertexAttributeDescriptor* attribute = attributes->object(attr)->init();
            attribute->setBufferIndex(vertexInfo.attributes[attr].binding);
            switch (vertexInfo.attributes[attr].format) {
            case Gfx::SE_FORMAT_R32G32B32_SFLOAT:
                attribute->setFormat(MTL::VertexFormatFloat3);
                break;
            default:
                throw std::logic_error("TODO");
            }
            attribute->setOffset(vertexInfo.attributes[attr].offset);
        }

        MTL::VertexBufferLayoutDescriptorArray* bufferLayout = vertexDescriptor->layouts()->init();
        for (size_t binding = 0; binding < vertexInfo.bindings.size(); ++binding) {
            MTL::VertexBufferLayoutDescriptor* buffer = bufferLayout->object(binding)->init();
            buffer->setStride(vertexInfo.bindings[binding].stride);
            buffer->setStepRate(1);
            switch (vertexInfo.bindings[binding].inputRate) {
            case Gfx::SE_VERTEX_INPUT_RATE_VERTEX:
                buffer->setStepFunction(MTL::VertexStepFunctionPerVertex);
                break;
            case Gfx::SE_VERTEX_INPUT_RATE_INSTANCE:
                buffer->setStepFunction(MTL::VertexStepFunctionPerInstance);
                break;
            }
        }
    }
    pipelineDescriptor->setVertexDescriptor(vertexDescriptor);
    auto vertShader = createInfo.vertexShader.cast<VertexShader>();
    Array<uint32> vertexSets = vertShader->usedDescriptors;
    MTL::Function* vertexFunction = vertShader->getFunction();
    Array<uint32> fragmentSets;
    MTL::Function* fragmentFunction = nullptr;
    pipelineDescriptor->setVertexFunction(vertexFunction);
    if (createInfo.fragmentShader != nullptr) {
        auto fragShader = createInfo.fragmentShader.cast<FragmentShader>();
        fragmentSets = fragShader->usedDescriptors;
        fragmentFunction = fragShader->getFunction();
        pipelineDescriptor->setFragmentFunction(fragmentFunction);
    }
    pipelineDescriptor->setInputPrimitiveTopology(cast(createInfo.topology));
    for(uint c = 0; c < renderPass->getLayout().colorAttachments.size(); ++c) {
        const auto& color = renderPass->getLayout().colorAttachments[c];
        MTL::RenderPipelineColorAttachmentDescriptor* desc = MTL::RenderPipelineColorAttachmentDescriptor::alloc()->init();
        desc->setWriteMask(MTL::ColorWriteMaskAll);
        desc->setPixelFormat(cast(color.getFormat()));
        desc->setAlphaBlendOperation(MTL::BlendOperationAdd);
        desc->setDestinationAlphaBlendFactor(MTL::BlendFactorDestinationAlpha);
        desc->setBlendingEnabled(false);
        pipelineDescriptor->colorAttachments()->setObject(desc, c);
    }

    MTL::DepthStencilDescriptor* depthDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
    if (createInfo.renderPass->getLayout().depthAttachment.getTextureView() != nullptr) {
        depthDescriptor->setDepthWriteEnabled(createInfo.depthStencilState.depthWriteEnable);
        depthDescriptor->setDepthCompareFunction(cast(createInfo.depthStencilState.depthCompareOp));
        pipelineDescriptor->setDepthAttachmentPixelFormat(
            cast(createInfo.renderPass->getLayout().depthAttachment.getTextureView()->getFormat()));
    }
    pipelineDescriptor->setAlphaToCoverageEnabled(createInfo.multisampleState.alphaCoverageEnable);
    pipelineDescriptor->setAlphaToOneEnabled(createInfo.multisampleState.alphaToOneEnable);
    pipelineDescriptor->setRasterSampleCount(createInfo.multisampleState.samples);
    pipelineDescriptor->setRasterizationEnabled(!createInfo.rasterizationState.rasterizerDiscardEnable);
    
    MTL::DepthStencilState* depthState = graphics->getDevice()->newDepthStencilState(depthDescriptor);
    depthDescriptor->release();
    
    uint32 hash = pipelineDescriptor->hash();

    if (graphicsPipelines.contains(hash)) {
        return graphicsPipelines[hash];
    }
    MTL::PrimitiveType type = MTL::PrimitiveTypeTriangle;
    switch (createInfo.topology) {
    case Gfx::SE_PRIMITIVE_TOPOLOGY_POINT_LIST:
        type = MTL::PrimitiveTypePoint;
        break;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_LIST:
        type = MTL::PrimitiveTypeLine;
        break;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_STRIP:
        type = MTL::PrimitiveTypeLineStrip;
        break;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        type = MTL::PrimitiveTypeTriangle;
        break;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
        type = MTL::PrimitiveTypeTriangleStrip;
        break;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
        type = MTL::PrimitiveTypeTriangle;
        break;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
        type = MTL::PrimitiveTypeLine;
        break;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
        type = MTL::PrimitiveTypeLineStrip;
        break;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
        type = MTL::PrimitiveTypeTriangle;
        break;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
        type = MTL::PrimitiveTypeTriangleStrip;
        break;
    case Gfx::SE_PRIMITIVE_TOPOLOGY_PATCH_LIST:
        type = MTL::PrimitiveTypeTriangle;
        break;
    }
    NS::Error* error;
    graphicsPipelines[hash] = new GraphicsPipeline(
        graphics, type, graphics->getDevice()->newRenderPipelineState(pipelineDescriptor, &error), std::move(createInfo.pipelineLayout));
    if (error) {
        std::cout << error->localizedDescription()->cString(NS::ASCIIStringEncoding) << std::endl;
        assert(false);
    }

    pipelineDescriptor->release();
    
    graphicsPipelines[hash]->vertexSets = vertexSets;
    graphicsPipelines[hash]->vertexFunction = vertexFunction;
    graphicsPipelines[hash]->fragmentSets = fragmentSets;
    graphicsPipelines[hash]->fragmentFunction = fragmentFunction;
    graphicsPipelines[hash]->depth = depthState;
    return graphicsPipelines[hash];
}

PGraphicsPipeline PipelineCache::createPipeline(Gfx::MeshPipelineCreateInfo createInfo) {
    PRenderPass renderPass = createInfo.renderPass.cast<RenderPass>();
    MTL::MeshRenderPipelineDescriptor* pipelineDescriptor = MTL::MeshRenderPipelineDescriptor::alloc()->init();

    auto meshShader = createInfo.meshShader.cast<MeshShader>();
    Array<uint32> meshSets = meshShader->usedDescriptors;
    MTL::Function* meshFunction = meshShader->getFunction();
    Array<uint32> taskSets;
    MTL::Function* taskFunction = nullptr;
    Array<uint32> fragmentSets;
    MTL::Function* fragmentFunction = nullptr;
    pipelineDescriptor->setMeshFunction(meshFunction);
    if (createInfo.taskShader != nullptr) {
        auto taskShader = createInfo.taskShader.cast<TaskShader>();
        taskSets = taskShader->usedDescriptors;
        taskFunction = taskShader->getFunction();
        pipelineDescriptor->setObjectFunction(taskFunction);
    }
    if (createInfo.fragmentShader != nullptr) {
        auto fragShader = createInfo.fragmentShader.cast<FragmentShader>();
        fragmentSets = fragShader->usedDescriptors;
        fragmentFunction = fragShader->getFunction();
        pipelineDescriptor->setFragmentFunction(fragmentFunction);
    }
    for(uint c = 0; c < renderPass->getLayout().colorAttachments.size(); ++c) {
        const auto& color = renderPass->getLayout().colorAttachments[c];
        MTL::RenderPipelineColorAttachmentDescriptor* desc = MTL::RenderPipelineColorAttachmentDescriptor::alloc()->init();
        desc->setWriteMask(MTL::ColorWriteMaskAll);
        desc->setPixelFormat(cast(color.getFormat()));
        desc->setAlphaBlendOperation(MTL::BlendOperationAdd);
        desc->setDestinationAlphaBlendFactor(MTL::BlendFactorDestinationAlpha);
        desc->setBlendingEnabled(false);
        pipelineDescriptor->colorAttachments()->setObject(desc, c);
    }
    MTL::DepthStencilDescriptor* depthDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
    if (createInfo.renderPass->getLayout().depthAttachment.getTextureView() != nullptr) {
        depthDescriptor->setDepthWriteEnabled(createInfo.depthStencilState.depthWriteEnable);
        depthDescriptor->setDepthCompareFunction(cast(createInfo.depthStencilState.depthCompareOp));
        pipelineDescriptor->setDepthAttachmentPixelFormat(
            cast(createInfo.renderPass->getLayout().depthAttachment.getTextureView()->getFormat()));
    }
    pipelineDescriptor->setAlphaToCoverageEnabled(createInfo.multisampleState.alphaCoverageEnable);
    pipelineDescriptor->setAlphaToOneEnabled(createInfo.multisampleState.alphaToOneEnable);
    pipelineDescriptor->setRasterSampleCount(createInfo.multisampleState.samples);
    pipelineDescriptor->setRasterizationEnabled(!createInfo.rasterizationState.rasterizerDiscardEnable);
    
    MTL::DepthStencilState* depthState = graphics->getDevice()->newDepthStencilState(depthDescriptor);
    depthDescriptor->release();
    if (createInfo.renderPass->getLayout().depthAttachment.getTextureView() != nullptr) {
        pipelineDescriptor->setDepthAttachmentPixelFormat(
            cast(createInfo.renderPass->getLayout().depthAttachment.getTextureView()->getFormat()));
    }
    pipelineDescriptor->setAlphaToCoverageEnabled(createInfo.multisampleState.alphaCoverageEnable);
    pipelineDescriptor->setAlphaToOneEnabled(createInfo.multisampleState.alphaToOneEnable);
    pipelineDescriptor->setRasterSampleCount(createInfo.multisampleState.samples);
    pipelineDescriptor->setRasterizationEnabled(!createInfo.rasterizationState.rasterizerDiscardEnable);

    uint32 hash = pipelineDescriptor->hash();

    if (graphicsPipelines.contains(hash)) {
        return graphicsPipelines[hash];
    }
    NS::Error* error = nullptr;
    MTL::AutoreleasedRenderPipelineReflection reflection;
    graphicsPipelines[hash] = new GraphicsPipeline(
        graphics, MTL::PrimitiveTypeTriangle,
        graphics->getDevice()->newRenderPipelineState(pipelineDescriptor, MTL::PipelineOptionNone, &reflection, &error),
        std::move(createInfo.pipelineLayout));

    if (error) {
        std::cout << error->debugDescription()->utf8String() << std::endl;
    }

    pipelineDescriptor->release();
    
    graphicsPipelines[hash]->taskSets = taskSets;
    graphicsPipelines[hash]->taskFunction = taskFunction;
    graphicsPipelines[hash]->meshSets = meshSets;
    graphicsPipelines[hash]->meshFunction = meshFunction;
    graphicsPipelines[hash]->fragmentSets = fragmentSets;
    graphicsPipelines[hash]->fragmentFunction = fragmentFunction;
    graphicsPipelines[hash]->depth = depthState;
    return graphicsPipelines[hash];
}

PComputePipeline PipelineCache::createPipeline(Gfx::ComputePipelineCreateInfo createInfo) {
    PComputeShader shader = createInfo.computeShader.cast<ComputeShader>();
    uint32 hash = shader->getShaderHash();
    if (computePipelines.contains(hash)) {
        return computePipelines[hash];
    }

    NS::Error* error;
    computePipelines[hash] = new ComputePipeline(graphics, graphics->getDevice()->newComputePipelineState(shader->getFunction(), &error),
                                                 std::move(createInfo.pipelineLayout));
    assert(!error);
    computePipelines[hash]->computeFunction = shader->getFunction();
    return computePipelines[hash];
}
