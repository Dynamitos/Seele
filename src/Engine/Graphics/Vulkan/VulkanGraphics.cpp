#include "VulkanGraphics.h"
#include "VulkanAllocator.h"
#include "VulkanQueue.h"
#include "Containers/Array.h"
#include "VulkanInitializer.h"
#include "VulkanCommandBuffer.h"
#include <GLFW/glfw3.h>

using namespace Seele::Vulkan;

Graphics::Graphics()
	: callback(VK_NULL_HANDLE), handle(VK_NULL_HANDLE), instance(VK_NULL_HANDLE), physicalDevice(VK_NULL_HANDLE)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

Graphics::~Graphics()
{
	vkDestroyDevice(handle, nullptr);
	DestroyDebugReportCallbackEXT(instance, nullptr, callback);
	vkDestroyInstance(instance, nullptr);
	glfwTerminate();
}

void Graphics::init(GraphicsInitializer initInfo)
{
	initInstance(initInfo);
	setupDebugCallback();
	pickPhysicalDevice();
	createDevice(initInfo);
	allocator = new Allocator(this);
	graphicsCommands = new CommandBufferManager(this, graphicsQueue);
	computeCommands = new CommandBufferManager(this, computeQueue);
	transferCommands = new CommandBufferManager(this, transferQueue);
	dedicatedTransferCommands = new CommandBufferManager(this, dedicatedTransferQueue);
}

void Graphics::beginFrame(void *windowHandle)
{
	GLFWwindow *window = static_cast<GLFWwindow *>(windowHandle);
	glfwPollEvents();
}

void Graphics::endFrame(void *windowHandle)
{
	GLFWwindow *window = static_cast<GLFWwindow *>(windowHandle);
}

void *Graphics::createWindow(const WindowCreateInfo &createInfo)
{
	GLFWwindow *window = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, createInfo.bFullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	return window;
}

VkDevice Graphics::getDevice() const
{
	return handle;
}

VkPhysicalDevice Graphics::getPhysicalDevice() const
{
	return physicalDevice;
}

PCommandBufferManager Graphics::getGraphicsCommands()
{
	return graphicsCommands;
}
PCommandBufferManager Graphics::getComputeCommands()
{
	return computeCommands;
}
PCommandBufferManager Graphics::getTransferCommands()
{
	return transferCommands;
}
PCommandBufferManager Graphics::getDedicatedTransferCommands()
{
	return dedicatedTransferCommands;
}
PAllocator Graphics::getAllocator()
{
	return allocator;
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
#ifdef ENABLE_VALIDATION
	extensions.add(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif // ENABLE_VALIDATION
	return extensions;
}
void Graphics::initInstance(GraphicsInitializer initInfo)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = initInfo.applicationName;
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = initInfo.engineName;
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_1;

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
#ifdef ENABLE_VALIDATION
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
	VkPhysicalDevice bestDevice;
	uint32 deviceRating = 0;
	for (auto physicalDevice : physicalDevices)
	{
		uint32 currentRating = 0;
		vkGetPhysicalDeviceProperties(physicalDevice, &props);
		vkGetPhysicalDeviceFeatures(physicalDevice, &features);
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
			bestDevice = physicalDevice;
		}
	}
	this->physicalDevice = bestDevice;
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
	for (int32_t familyIndex = 0; familyIndex < queueProperties.size(); ++familyIndex)
	{
		uint32 numQueuesForFamily = 0;
		VkQueueFamilyProperties currProps = queueProperties[familyIndex];
		bool bSparse = currProps.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
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
					if (familyIndex == graphicsQueueFamilyIndex)
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
		if(numQueuesForFamily > 0)
		{
			VkDeviceQueueCreateInfo info =
				init::DeviceQueueCreateInfo(familyIndex, numQueuesForFamily);
			queueInfos.add(info);
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

	graphicsQueue = new Queue(this, QueueType::GRAPHICS, graphicsQueueFamilyIndex, 0);
	if (Gfx::useAsyncCompute && asyncComputeFamilyIndex != -1)
	{
		if (asyncComputeFamilyIndex == graphicsQueueFamilyIndex)
		{
			// Same family as graphics, but different queue
			computeQueue = new Queue(this, QueueType::COMPUTE, asyncComputeFamilyIndex, 1);
		}
		else
		{
			// Different family
			computeQueue = new Queue(this, QueueType::COMPUTE, asyncComputeFamilyIndex, 0);
		}
	}
	else
	{
		computeQueue = new Queue(this, QueueType::COMPUTE, computeQueueFamilyIndex, 0);
	}
	transferQueue = new Queue(this, QueueType::TRANSFER, transferQueueFamilyIndex, 0);
	if (dedicatedTransferQueueFamilyIndex != -1)
	{
		dedicatedTransferQueue = new Queue(this, QueueType::DEDICATED_TRANSFER, dedicatedTransferQueueFamilyIndex, 0);
	}
	queueMapping.graphicsFamily = graphicsQueue->getFamilyIndex();
	queueMapping.computeFamily = computeQueue->getFamilyIndex();
	queueMapping.transferFamily = transferQueue->getFamilyIndex();
	queueMapping.dedicatedTransferFamily = dedicatedTransferQueue->getFamilyIndex();
}
