#include "VulkanGraphics.h"
#include "VulkanAllocator.h"
#include "Containers/Array.h"
#include "VulkanInitializer.h"
#include <GLFW/glfw3.h>

using namespace Seele::Vulkan;

Graphics::Graphics()
	: callback(VK_NULL_HANDLE)
	, handle(VK_NULL_HANDLE)
	, instance(VK_NULL_HANDLE)
	, physicalDevice(VK_NULL_HANDLE)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

Graphics::~Graphics()
{
	glfwTerminate();
}

void Graphics::init(GraphicsInitializer initInfo)
{
	initInstance(initInfo);
	setupDebugCallback();
	pickPhysicalDevice();
	allocator = new VulkanAllocator(this);
}

void Graphics::beginFrame(void* windowHandle)
{
	GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);
	glfwPollEvents();
}

void Graphics::endFrame(void* windowHandle)
{
	GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);
}

void* Graphics::createWindow(const WindowCreateInfo& createInfo)
{
	GLFWwindow* window = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, createInfo.bFullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
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

Array<const char*> Graphics::getRequiredExtensions()
{
	Array<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++) {
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
	Array<const char*> extensions = getRequiredExtensions();
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
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevice, &props);
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

void Graphics::createDevice()
{

}
