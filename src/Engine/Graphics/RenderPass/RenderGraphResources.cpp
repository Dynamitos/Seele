#include "RenderGraphResources.h"
#include "Graphics/Query.h"
#include <string>

using namespace Seele;

RenderGraphResources::RenderGraphResources() {}

RenderGraphResources::~RenderGraphResources() {}

Gfx::RenderTargetAttachment RenderGraphResources::requestRenderTarget(const std::string& outputName) {
    return registeredAttachments.at(outputName);
}

Gfx::PTexture RenderGraphResources::requestTexture(const std::string& outputName) { return registeredTextures.at(outputName); }

Gfx::PShaderBuffer RenderGraphResources::requestBuffer(const std::string& outputName) { return registeredBuffers.at(outputName); }

Gfx::PUniformBuffer RenderGraphResources::requestUniform(const std::string& outputName) { return registeredUniforms.at(outputName); }

Gfx::PPipelineStatisticsQuery RenderGraphResources::requestQuery(const std::string& outputName) { return registeredQueries.at(outputName); }

Gfx::PTimestampQuery Seele::RenderGraphResources::requestTimestampQuery(const std::string& outputName) { return registeredTimestamps.at(outputName); }

void RenderGraphResources::registerRenderPassOutput(const std::string& outputName, Gfx::RenderTargetAttachment attachment) {
    registeredAttachments[outputName] = attachment;
}

void RenderGraphResources::registerTextureOutput(const std::string& outputName, Gfx::PTexture texture) {
    registeredTextures[outputName] = texture;
}

void RenderGraphResources::registerBufferOutput(const std::string& outputName, Gfx::PShaderBuffer buffer) {
    registeredBuffers[outputName] = buffer;
}
void RenderGraphResources::registerUniformOutput(const std::string& outputName, Gfx::PUniformBuffer buffer) {
    registeredUniforms[outputName] = buffer;
}

void RenderGraphResources::registerQueryOutput(const std::string& outputName, Gfx::PPipelineStatisticsQuery query) {
    registeredQueries[outputName] = query;
}

void Seele::RenderGraphResources::registerTimestampQueryOutput(const std::string& outputName, Gfx::PTimestampQuery query) {
    registeredTimestamps[outputName] = query;
}
