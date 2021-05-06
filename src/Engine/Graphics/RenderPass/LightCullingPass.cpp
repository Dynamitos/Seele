#include "LightCullingPass.h"
#include "Graphics/Graphics.h"
#include "Scene/Actor/CameraActor.h"
#include "Scene/Components/CameraComponent.h"

using namespace Seele;

LightCullingPass::LightCullingPass(PRenderGraph renderGraph, Gfx::PViewport viewport, Gfx::PGraphics graphics, PCameraActor camera) 
    : RenderPass(renderGraph)
    , viewport(viewport)
    , graphics(graphics)
    , source(camera->getCameraComponent())
{
}

LightCullingPass::~LightCullingPass() 
{
    
}

void LightCullingPass::beginFrame() 
{
    uint32_t viewportWidth = viewport->getSizeX();
	uint32_t viewportHeight = viewport->getSizeY();

	glm::uvec3 numThreads = glm::ceil(glm::vec3(viewportWidth / (float)BLOCK_SIZE, viewportHeight / (float)BLOCK_SIZE, 1));
	glm::uvec3 numThreadGroups = glm::ceil(glm::vec3(numThreads.x / (float)BLOCK_SIZE, numThreads.y / (float)BLOCK_SIZE, 1));
	
	dispatchParams.numThreads = numThreads;
	dispatchParams.numThreadGroups = numThreadGroups;
	ScreenToView screenToView;
	screenToView.inverseProjection = glm::inverse(source->getProjectionMatrix());
	screenToView.screenDimensions = glm::vec2(viewportWidth, viewportHeight);

	frustumShader = renderDevice->createComputeShader(loadPlaintext("./_Game/shaders/ComputeFrustums.slang"), "computeFrustums");
	frustumDescriptorLayout = renderDevice->createDescriptorLayout();
	frustumDescriptorLayout->addDescriptorBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	frustumDescriptorLayout->addDescriptorBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	frustumDescriptorLayout->addDescriptorBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	frustumLayout = renderDevice->createPipelineLayout();
	frustumLayout->addPushConstants(init::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(DispatchParams), 0));
	frustumLayout->addDescriptorLayout(0, frustumDescriptorLayout);
	frustumLayout->create();
	frustumShader->setPipelineLayout(frustumLayout);
	RHIResourceCreateInfo frustumInfo;
	frustumBuffer = renderDevice->createStructuredBuffer(sizeof(Frustum), sizeof(Frustum) * numThreads.x * numThreads.y * numThreads.z, BufferUsageFlags::BUF_UnorderedAccess, frustumInfo);
	dispatchParamsBuffer = renderDevice->createUniformBuffer(&dispatchParams, sizeof(DispatchParams), UniformBuffer_MultiFrame);
	screenToViewParams = renderDevice->createUniformBuffer(&screenToView, sizeof(ScreenToView), UniformBuffer_MultiFrame);
	frustumDescriptorSet = frustumDescriptorLayout->allocateDescriptorSet();
	frustumDescriptorSet->updateBuffer(0, dispatchParamsBuffer);
	frustumDescriptorSet->updateBuffer(1, screenToViewParams);
	frustumDescriptorSet->updateBuffer(2, frustumBuffer);
	frustumDescriptorSet->writeChanges();
	renderDevice->setComputeShader(frustumShader);
	renderDevice->bindComputeDescriptors(frustumLayout, frustumDescriptorSet);
	renderDevice->dispatchComputeShader(numThreadGroups.x, numThreadGroups.y, numThreadGroups.z);
	renderDevice->pipelineBarrier(frustumBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void LightCullingPass::render() 
{
    
}

void LightCullingPass::endFrame() 
{
    
}

void LightCullingPass::publishOutputs() 
{
    
}

void LightCullingPass::createRenderPass() 
{
    
}

void LightCullingPass::modifyRenderPassMacros(Map<const char*, const char*>& defines) 
{
    
}