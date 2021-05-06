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

Gfx::PRenderTargetAttachment RenderGraph::requestRenderTarget(const std::string& outputName) 
{
    if(registeredAttachments.find(outputName) == registeredAttachments.end())
    {
        std::cout << "Attachment " << outputName << " not found" << std::endl; 
        return nullptr;
    }
    return registeredAttachments[outputName];
}

void RenderGraph::registerRenderPassOutput(const std::string& outputName, Gfx::PRenderTargetAttachment attachment) 
{
    registeredAttachments[outputName] = attachment;
}