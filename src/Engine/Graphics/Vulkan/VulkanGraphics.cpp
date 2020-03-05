#include "VulkanGraphics.h"
#include <GLFW/glfw3.h>

void Seele::VulkanGraphics::init(GraphicsInitializer initializer)
{
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
