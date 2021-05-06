#include "Containers/Array.h"
#include "VulkanGraphics.h"
#include "VulkanAllocator.h"
#include "VulkanQueue.h"
#include "VulkanInitializer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipelineCache.h"
#include "VulkanDescriptorSets.h"
#include "VulkanShader.h"
#include "Graphics/GraphicsResources.h"
#include <GLFW/glfw3.h>

using namespace Seele::Vulkan;

Graphics::Graphics()
	: instance(VK_NULL_HANDLE)
	, handle(VK_NULL_HANDLE)
	, physicalDevice(VK_NULL_HANDLE)
	, callback(VK_NULL_HANDLE)
{
}

Graphics::~Graphics()
{
	viewports.clear();
	vkDestroyDevice(handle, nullptr);
	DestroyDebugReportCallbackEXT(instance, nullptr, callback);
	vkDestroyInstance(instance, nullptr);
}

void Graphics::init(GraphicsInitializer initInfo)
{
	initInstance(initInfo);
#if ENABLE_VALIDATION
	setupDebugCallback();
#endif
	pickPhysicalDevice();
	createDevice(initInfo);
	allocator = new Allocator(this);
	stagingManager = new StagingManager(this, allocator);
	pipelineCache = new PipelineCache(this, "pipeline.cache");
}

Gfx::PWindow Graphics::createWindow(const WindowCreateInfo &createInfo)
{
	PWindow result = new Window(this, createInfo);
	return result;
}

Gfx::PViewport Graphics::createViewport(Gfx::PWindow owner, const ViewportCreateInfo &viewportInfo)
{
	PViewport result = new Viewport(this, owner, viewportInfo);
	viewports.add(result);
	return result;
}
Gfx::PRenderPass Graphics::createRenderPass(Gfx::PRenderTargetLayout layout)
{
	PRenderPass result = new RenderPass(this, layout);
	return result;
}
void Graphics::beginRenderPass(Gfx::PRenderPass renderPass)
{
	PRenderPass rp = renderPass.cast<RenderPass>();
	uint32 framebufferHash = rp->getFramebufferHash();
	PFramebuffer framebuffer;
	auto found = allocatedFramebuffers.find(framebufferHash);
	if (found == allocatedFramebuffers.end())
	{
		framebuffer = new Framebuffer(this, rp, rp->getLayout());
		allocatedFramebuffers[framebufferHash] = framebuffer;
	}
	else
	{
		framebuffer = found->value;
	}
	getGraphicsCommands()->getCommands()->beginRenderPass(rp, framebuffer);
}

void Graphics::endRenderPass()
{
	getGraphicsCommands()->getCommands()->endRenderPass();
}

void Graphics::executeCommands(Array<Gfx::PRenderCommand> commands)
{
	getGraphicsCommands()->getCommands()->executeCommands(commands);
}

Gfx::PTexture2D Graphics::createTexture2D(const TextureCreateInfo &createInfo)
{
	PTexture2D result = new Texture2D(this, createInfo);
	return result;
}

Gfx::PUniformBuffer Graphics::createUniformBuffer(const UniformBufferCreateInfo &bulkData)
{
	PUniformBuffer uniformBuffer = new UniformBuffer(this, bulkData);
	return uniformBuffer;
}

Gfx::PStructuredBuffer Graphics::createStructuredBuffer(const BulkResourceData &bulkData)
{
	PStructuredBuffer structuredBuffer = new StructuredBuffer(this, bulkData);
	return structuredBuffer;
}
Gfx::PVertexBuffer Graphics::createVertexBuffer(const VertexBufferCreateInfo &bulkData)
{
	PVertexBuffer vertexBuffer = new VertexBuffer(this, bulkData);
	return vertexBuffer;
}

Gfx::PIndexBuffer Graphics::createIndexBuffer(const IndexBufferCreateInfo &bulkData)
{
	PIndexBuffer indexBuffer = new IndexBuffer(this, bulkData);
	return indexBuffer;
}
Gfx::PRenderCommand Graphics::createRenderCommand()
{
	PSecondaryCmdBuffer cmdBuffer = getGraphicsCommands()->createSecondaryCmdBuffer();
	return cmdBuffer;
}

Gfx::PVertexDeclaration Graphics::createVertexDeclaration(const Array<Gfx::VertexElement>& element) 
{
	PVertexDeclaration declaration = new VertexDeclaration(element);
	return declaration;
}

Gfx::PVertexShader Graphics::createVertexShader(const ShaderCreateInfo& createInfo)
{
	PVertexShader shader = new VertexShader(this);
	shader->create(createInfo);
	return shader;
}
Gfx::PControlShader Graphics::createControlShader(const ShaderCreateInfo& createInfo)
{
	PControlShader shader = new ControlShader(this);
	shader->create(createInfo);
	return shader;
}
Gfx::PEvaluationShader Graphics::createEvaluationShader(const ShaderCreateInfo& createInfo)
{
	PEvaluationShader shader = new EvaluationShader(this);
	shader->create(createInfo);
	return shader;
}
Gfx::PGeometryShader Graphics::createGeometryShader(const ShaderCreateInfo& createInfo)
{
	PGeometryShader shader = new GeometryShader(this);
	shader->create(createInfo);
	return shader;
}
Gfx::PFragmentShader Graphics::createFragmentShader(const ShaderCreateInfo& createInfo)
{
	PFragmentShader shader = new FragmentShader(this);
	shader->create(createInfo);
	return shader;
}
Gfx::PGraphicsPipeline Graphics::createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
{
	PGraphicsPipeline pipeline = pipelineCache->createPipeline(createInfo);
	return pipeline;
}

Gfx::PComputePipeline Graphics::createComputePipeline(const ComputePipelineCreateInfo& createInfo) 
{
	PComputePipeline pipeline = pipelineCache->createPipeline(createInfo);
	return pipeline;
}

Gfx::PSamplerState Graphics::createSamplerState(const SamplerCreateInfo&) 
{
	PSamplerState sampler = new SamplerState(); // TODO: proper sampler creation
	VkSamplerCreateInfo vkInfo = 
		init::SamplerCreateInfo();
	VK_CHECK(vkCreateSampler(handle, &vkInfo, nullptr, &sampler->sampler));
	return sampler;
}
Gfx::PDescriptorLayout Graphics::createDescriptorLayout(const std::string& name)
{
	PDescriptorLayout layout = new DescriptorLayout(this, name);
	return layout;
}
Gfx::PPipelineLayout Graphics::createPipelineLayout()
{
	PPipelineLayout layout = new PipelineLayout(this);
	return layout;
}

VkInstance Graphics::getInstance() const
{
	return instance;
}

VkDevice Graphics::getDevice() const
{
	return handle;
}

VkPhysicalDevice Graphics::getPhysicalDevice() const
{
	return physicalDevice;
}

PCommandBufferManager Graphics::getQueueCommands(Gfx::QueueType queueType)
{
	switch (queueType)
	{
	case Gfx::QueueType::GRAPHICS:
		return getGraphicsCommands();
	case Gfx::QueueType::COMPUTE:
		return getComputeCommands();
	case Gfx::QueueType::TRANSFER:
		return getTransferCommands();
	case Gfx::QueueType::DEDICATED_TRANSFER:
		return getDedicatedTransferCommands();
	default:
		throw new std::logic_error("invalid queue type");
	}
}
std::mutex graphicsCommandLock;
PCommandBufferManager Graphics::getGraphicsCommands()
{
	std::scoped_lock lock(graphicsCommandLock);
	auto id = std::this_thread::get_id();
	if(graphicsCommands.find(id) == graphicsCommands.end())
	{
		graphicsCommands[id] = new CommandBufferManager(this, graphicsQueue);
	}
	return graphicsCommands[id];
}
std::mutex computeCommandLock;
PCommandBufferManager Graphics::getComputeCommands()
{
	std::scoped_lock lock(computeCommandLock);
	auto id = std::this_thread::get_id();
	if(computeCommands.find(id) == computeCommands.end())
	{
		computeCommands[id] = new CommandBufferManager(this, computeQueue);
	}
	return computeCommands[id];
}
std::mutex transferCommandLock;
PCommandBufferManager Graphics::getTransferCommands()
{
	std::scoped_lock lock(transferCommandLock);
	auto id = std::this_thread::get_id();
	if(transferCommands.find(id) == transferCommands.end())
	{
		transferCommands[id] = new CommandBufferManager(this, transferQueue);
	}
	return transferCommands[id];
}
std::mutex dedicatedCommandLock;
PCommandBufferManager Graphics::getDedicatedTransferCommands()
{
	std::scoped_lock lock(dedicatedCommandLock);
	auto id = std::this_thread::get_id();
	if(dedicatedTransferCommands.find(id) == dedicatedTransferCommands.end())
	{
		dedicatedTransferCommands[id] = new CommandBufferManager(this, dedicatedTransferQueue);
	}
	return dedicatedTransferCommands[id];
}
PAllocator Graphics::getAllocator()
{
	return allocator;
}

PStagingManager Graphics::getStagingManager()
{
	return stagingManager;
}

Array<const char *> Graphics::getRequiredExtensions()
{
	Array<const char *> extensions;

	unsigned int glfwExtensionCount = 0;
	const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++)
	{
		extensions.add(glfwExtensions[i]);
	}
#if ENABLE_VALIDATION
	extensions.add(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif // ENABLE_VALIDATION
	return extensions;
}
void Graphics::initInstance(GraphicsInitializer initInfo)
{
	glfwInit();
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = initInfo.applicationName;
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = initInfo.engineName;
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = &appInfo;
	Array<const char *> extensions = getRequiredExtensions();
	for (uint32 i = 0; i < initInfo.instanceExtensions.size(); ++i)
	{
		extensions.add(initInfo.instanceExtensions[i]);
	}
	info.enabledExtensionCount = (uint32)extensions.size();
	info.ppEnabledExtensionNames = extensions.data();
#if ENABLE_VALIDATION
	info.enabledLayerCount = (uint32)initInfo.layers.size();
	info.ppEnabledLayerNames = initInfo.layers.data();
#else
	info.enabledLayerCount = 0;
#endif
	VK_CHECK(vkCreateInstance(&info, nullptr, &instance));
}
void Graphics::setupDebugCallback()
{
	VkDebugReportCallbackCreateInfoEXT createInfo =
		init::DebugReportCallbackCreateInfo(
			VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT);

	VK_CHECK(CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback));
}

void Graphics::pickPhysicalDevice()
{
	uint32 physicalDeviceCount;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	Array<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
	VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
	uint32 deviceRating = 0;
	for (auto dev : physicalDevices)
	{
		uint32 currentRating = 0;
		vkGetPhysicalDeviceProperties(dev, &props);
		vkGetPhysicalDeviceFeatures(dev, &features);
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			currentRating += 100;
		}
		else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			currentRating += 10;
		}
		if (currentRating > deviceRating)
		{
			deviceRating = currentRating;
			bestDevice = dev;
		}
	}
	physicalDevice = bestDevice;
	vkGetPhysicalDeviceProperties(physicalDevice, &props);
	vkGetPhysicalDeviceFeatures(physicalDevice, &features);
}

void Graphics::createDevice(GraphicsInitializer initializer)
{
	uint32_t numQueueFamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, nullptr);
	Array<VkQueueFamilyProperties> queueProperties(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, queueProperties.data());

	Array<VkDeviceQueueCreateInfo> queueInfos;
	int32_t graphicsQueueFamilyIndex = -1;
	int32_t transferQueueFamilyIndex = -1;
	int32_t dedicatedTransferQueueFamilyIndex = -1;
	int32_t computeQueueFamilyIndex = -1;
	int32_t asyncComputeFamilyIndex = -1;
	uint32 numPriorities = 0;
	for (uint32 familyIndex = 0; familyIndex < queueProperties.size(); ++familyIndex)
	{
		uint32 numQueuesForFamily = 0;
		VkQueueFamilyProperties currProps = queueProperties[familyIndex];
		// bool bSparse = currProps.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
		currProps.queueFlags = currProps.queueFlags ^ VK_QUEUE_SPARSE_BINDING_BIT;
		if ((currProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
		{
			if (graphicsQueueFamilyIndex == -1)
			{
				graphicsQueueFamilyIndex = familyIndex;
				numQueuesForFamily++;
			}
		}
		if ((currProps.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
		{
			if (computeQueueFamilyIndex == -1)
			{
				computeQueueFamilyIndex = familyIndex;
			}
			if (Gfx::useAsyncCompute)
			{
				if (asyncComputeFamilyIndex == -1)
				{
					if (familyIndex == (uint32)graphicsQueueFamilyIndex)
					{
						if (currProps.queueCount > 1)
						{
							asyncComputeFamilyIndex = familyIndex;
							numQueuesForFamily++;
						}
					}
					else
					{
						asyncComputeFamilyIndex = familyIndex;
					}
				}
			}
		}
		if ((currProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
		{
			if (transferQueueFamilyIndex == -1)
			{
				transferQueueFamilyIndex = familyIndex;
				numQueuesForFamily++;
			}
			if ((currProps.queueFlags ^ VK_QUEUE_TRANSFER_BIT) == 0)
			{
				dedicatedTransferQueueFamilyIndex = familyIndex;
				numQueuesForFamily++;
			}
		}
		if (numQueuesForFamily > 0)
		{
			VkDeviceQueueCreateInfo info =
				init::DeviceQueueCreateInfo(familyIndex, numQueuesForFamily);
			numPriorities += numQueuesForFamily;
			queueInfos.add(info);
		}
	}
	Array<float> queuePriorities;
	queuePriorities.resize(numPriorities);
	float *currentPriority = queuePriorities.data();
	for (uint32 index = 0; index < queueInfos.size(); ++index)
	{
		VkDeviceQueueCreateInfo &currQueue = queueInfos[index];
		currQueue.pQueuePriorities = currentPriority;

		for (int32_t queueIndex = 0; queueIndex < (int32_t)currQueue.queueCount; ++queueIndex)
		{
			*currentPriority++ = 1.0f;
		}
	}
	VkDeviceCreateInfo deviceInfo = init::DeviceCreateInfo(
		queueInfos.data(),
		(uint32)queueInfos.size(),
		&features);
	deviceInfo.enabledExtensionCount = (uint32)initializer.deviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = initializer.deviceExtensions.data();
	deviceInfo.enabledLayerCount = (uint32_t)initializer.layers.size();
	deviceInfo.ppEnabledLayerNames = initializer.layers.data();

	VK_CHECK(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &handle));

	graphicsQueue = new Queue(this, Gfx::QueueType::GRAPHICS, graphicsQueueFamilyIndex, 0);
	if (Gfx::useAsyncCompute && asyncComputeFamilyIndex != -1)
	{
		if (asyncComputeFamilyIndex == graphicsQueueFamilyIndex)
		{
			// Same family as graphics, but different queue
			computeQueue = new Queue(this, Gfx::QueueType::COMPUTE, asyncComputeFamilyIndex, 1);
		}
		else
		{
			// Different family
			computeQueue = new Queue(this, Gfx::QueueType::COMPUTE, asyncComputeFamilyIndex, 0);
		}
	}
	else
	{
		computeQueue = new Queue(this, Gfx::QueueType::COMPUTE, computeQueueFamilyIndex, 0);
	}
	transferQueue = new Queue(this, Gfx::QueueType::TRANSFER, transferQueueFamilyIndex, 0);
	if (dedicatedTransferQueueFamilyIndex != -1)
	{
		dedicatedTransferQueue = new Queue(this, Gfx::QueueType::DEDICATED_TRANSFER, dedicatedTransferQueueFamilyIndex, 0);
	}
	else
	{
		dedicatedTransferQueue = transferQueue;
	}
	queueMapping.graphicsFamily = graphicsQueue->getFamilyIndex();
	queueMapping.computeFamily = computeQueue->getFamilyIndex();
	queueMapping.transferFamily = transferQueue->getFamilyIndex();
	queueMapping.dedicatedTransferFamily = dedicatedTransferQueue->getFamilyIndex();
}
