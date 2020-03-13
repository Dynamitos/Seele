#include "VulkanGraphics.h"
#include "VulkanAllocator.h"
#include "Containers/Array.h"
#include "VulkanInitializer.h"
#include <GLFW/glfw3.h>

void Seele::VulkanGraphics::init(GraphicsInitializer initInfo)
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
	setupDebugCallback();
	allocator = new VulkanAllocator(this);
}

void Seele::VulkanGraphics::beginFrame(void* windowHandle)
{
	GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);
	glfwPollEvents();
}

void Seele::VulkanGraphics::endFrame(void* windowHandle)
{
	GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);
}

void* Seele::VulkanGraphics::createWindow(const WindowCreateInfo& createInfo)
{
	GLFWwindow* window = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, createInfo.bFullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	return window;
}

Seele::VulkanGraphics::VulkanGraphics()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

Seele::VulkanGraphics::~VulkanGraphics()
{
	glfwTerminate();
}

VkDevice Seele::VulkanGraphics::getDevice() const
{
	return handle;
}

VkPhysicalDevice Seele::VulkanGraphics::getPhysicalDevice() const
{
	return physicalDevice;
}

Seele::Array<const char*> Seele::VulkanGraphics::getRequiredExtensions()
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
void Seele::VulkanGraphics::setupDebugCallback()
{
	VkDebugReportCallbackCreateInfoEXT createInfo =
		init::DebugReportCallbackCreateInfo(
			VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT);

	VK_CHECK(CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback));
}