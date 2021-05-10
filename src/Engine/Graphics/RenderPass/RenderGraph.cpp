#include "RenderGraph.h"

using namespace Seele;


RenderGraph::RenderGraph() 
{
}

RenderGraph::~RenderGraph() 
{   
}

void RenderGraph::setup() 
{
    for(auto pass : renderPasses)
    {
        pass->publishOutputs();
    }
    for(auto pass : renderPasses)
    {
        pass->createRenderPass();
    }
}

void RenderGraph::addRenderPass(PRenderPass renderPass) 
{
    renderPasses.add(renderPass);
}

Gfx::PRenderTargetAttachment RenderGraph::requestRenderTarget(const std::string& outputName) 
{
    if(registeredAttachments.find(outputName) == registeredAttachments.end())
    {
        std::cout << "Attachment " << outputName << " not found" << std::endl; 
        return nullptr;
    }
    return registeredAttachments[outputName];
}

Gfx::PTexture RenderGraph::requestTexture(const std::string& outputName) 
{
    if(registeredTextures.find(outputName) == registeredTextures.end())
    {
        std::cout << "Attachment " << outputName << " not found" << std::endl; 
        return nullptr;
    }
    return registeredTextures[outputName];
}


Gfx::PStructuredBuffer RenderGraph::requestBuffer(const std::string& outputName) 
{
    if(registeredBuffers.find(outputName) == registeredBuffers.end())
    {
        std::cout << "Attachment " << outputName << " not found" << std::endl; 
        return nullptr;
    }
    return registeredBuffers[outputName];
}

void RenderGraph::registerRenderPassOutput(const std::string& outputName, Gfx::PRenderTargetAttachment attachment) 
{
    registeredAttachments[outputName] = attachment;
}

void RenderGraph::registerTextureOutput(const std::string& outputName, Gfx::PTexture texture)
{
    registeredTextures[outputName] = texture;
}

void RenderGraph::registerBufferOutput(const std::string& outputName, Gfx::PStructuredBuffer buffer) 
{
    registeredBuffers[outputName] = buffer;
}