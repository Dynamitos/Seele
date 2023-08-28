#include "RenderGraphResources.h"

using namespace Seele;

RenderGraphResources::RenderGraphResources() 
{
    
}

RenderGraphResources::~RenderGraphResources() 
{
    
}

Gfx::PRenderTargetAttachment RenderGraphResources::requestRenderTarget(const std::string& outputName) 
{
    if(registeredAttachments.find(outputName) == registeredAttachments.end())
    {
        std::cout << "Attachment " << outputName << " not found" << std::endl; 
        return nullptr;
    }
    return registeredAttachments[outputName];
}

Gfx::PTexture RenderGraphResources::requestTexture(const std::string& outputName) 
{
    if(registeredTextures.find(outputName) == registeredTextures.end())
    {
        std::cout << "Attachment " << outputName << " not found" << std::endl; 
        return nullptr;
    }
    return registeredTextures[outputName];
}

Gfx::PShaderBuffer RenderGraphResources::requestBuffer(const std::string& outputName) 
{
    if(registeredBuffers.find(outputName) == registeredBuffers.end())
    {
        std::cout << "Attachment " << outputName << " not found" << std::endl; 
        return nullptr;
    }
    return registeredBuffers[outputName];
}

Gfx::PUniformBuffer RenderGraphResources::requestUniform(const std::string& outputName) 
{
    if(registeredUniforms.find(outputName) == registeredUniforms.end())
    {
        std::cout << "Attachment " << outputName << " not found" << std::endl; 
        return nullptr;
    }
    return registeredUniforms[outputName];
}

void RenderGraphResources::registerRenderPassOutput(const std::string& outputName, Gfx::PRenderTargetAttachment attachment) 
{
    registeredAttachments[outputName] = attachment;
}

void RenderGraphResources::registerTextureOutput(const std::string& outputName, Gfx::PTexture texture)
{
    registeredTextures[outputName] = texture;
}

void RenderGraphResources::registerBufferOutput(const std::string& outputName, Gfx::PShaderBuffer buffer) 
{
    registeredBuffers[outputName] = buffer;
}
void RenderGraphResources::registerUniformOutput(const std::string& outputName, Gfx::PUniformBuffer buffer) 
{
    registeredUniforms[outputName] = buffer;
}