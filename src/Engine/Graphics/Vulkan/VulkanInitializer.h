#include "Containers/Array.h"
#include <vulkan/vulkan.h>

namespace Seele
{
	namespace Vulkan
	{
		VkBool32 debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);

		VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
		void DestroyDebugReportCallbackEXT(VkInstance instance, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT pCallback);

		namespace init
		{
			VkApplicationInfo ApplicationInfo(
				const char* appName,
				uint32_t appVersion,
				const char* engineName,
				uint32_t engineVersion,
				uint32_t apiVersion);

			VkInstanceCreateInfo InstanceCreateInfo(
				VkApplicationInfo* appInfo,
				const Array<const char*>& extensions,
				const Array<const char*>& layers);

			VkDebugReportCallbackCreateInfoEXT DebugReportCallbackCreateInfo(
				VkDebugReportFlagsEXT flags);

			VkDeviceQueueCreateInfo DeviceQueueCreateInfo(
				int queueFamilyIndex,
				int queueCount);

			VkDeviceQueueCreateInfo DeviceQueueCreateInfo(
				int queueFamilyIndex,
				int queueCount,
				float* queuePriority);

			VkDeviceCreateInfo DeviceCreateInfo(
				VkDeviceQueueCreateInfo* queueInfos,
				uint32_t queueCount,
				VkPhysicalDeviceFeatures* features,
				const char* const* deviceExtensions,
				uint32_t deviceExtensionCount,
				const char* const* layers,
				uint32_t layerCount);

			VkDeviceCreateInfo DeviceCreateInfo(
				VkDeviceQueueCreateInfo* queueInfos,
				uint32_t queueCount,
				VkPhysicalDeviceFeatures* features);

			VkSwapchainCreateInfoKHR SwapchainCreateInfo(
				VkSurfaceKHR surface,
				uint32_t minImageCount,
				VkFormat imageFormat,
				VkColorSpaceKHR colorSpace,
				VkExtent2D extent,
				uint32_t arrayLayers,
				VkImageUsageFlags usage,
				VkSurfaceTransformFlagBitsKHR Transform,
				VkCompositeAlphaFlagBitsKHR alpha,
				VkPresentModeKHR presentMode,
				VkBool32 clipped);

			VkSwapchainCreateInfoKHR SwapchainCreateInfo(
				VkSurfaceKHR surface,
				uint32_t minImageCount,
				VkFormat imageFormat,
				VkColorSpaceKHR colorSpace,
				uint32_t width,
				uint32_t height,
				uint32_t arrayLayers,
				VkImageUsageFlags usage,
				VkSurfaceTransformFlagBitsKHR Transform,
				VkCompositeAlphaFlagBitsKHR alpha,
				VkPresentModeKHR presentMode,
				VkBool32 clipped);

			VkFramebufferCreateInfo FramebufferCreateInfo(
				VkRenderPass renderPass,
				uint32_t attachmentCount,
				VkImageView* attachments,
				uint32_t width,
				uint32_t height,
				uint32_t layers);

			VkAttachmentDescription AttachmentDescription(
				VkFormat format,
				VkSampleCountFlagBits sample,
				VkAttachmentLoadOp loadOp,
				VkAttachmentStoreOp storeOp,
				VkAttachmentLoadOp stencilLoadOp,
				VkAttachmentStoreOp stencilStoreOp,
				VkImageLayout imageLayout,
				VkImageLayout finalLayout);

			VkSubpassDescription SubpassDescription(
				VkPipelineBindPoint bindPoint,
				uint32_t colorAttachmentCount,
				VkAttachmentReference* colorReference,
				uint32_t depthAttachmentCount = 0,
				VkAttachmentReference* depthReference = nullptr,
				uint32_t inputAttachmentCount = 0,
				VkAttachmentReference* inputReference = nullptr,
				uint32_t resolveAttachmentCount = 0,
				VkAttachmentReference* resolveReference = nullptr,
				uint32_t preserveAttachmentCount = 0,
				VkAttachmentReference* preserveReference = nullptr);

			VkRenderPassCreateInfo RenderPassCreateInfo(
				uint32_t attachmentCount,
				const VkAttachmentDescription* attachments,
				uint32_t subpassCount,
				const VkSubpassDescription* subpasses,
				uint32_t dependencyCount,
				const VkSubpassDependency* subpassDependencies);

			VkMemoryAllocateInfo MemoryAllocateInfo();

			VkCommandBufferAllocateInfo CommandBufferAllocateInfo(
				VkCommandPool cmdPool,
				VkCommandBufferLevel level,
				uint32_t bufferCount);

			VkCommandPoolCreateInfo CommandPoolCreateInfo();

			VkCommandBufferBeginInfo CommandBufferBeginInfo();

			VkCommandBufferInheritanceInfo CommandBufferInheritanceInfo();

			VkRenderPassBeginInfo RenderPassBeginInfo();

			VkRenderPassCreateInfo RenderPassCreateInfo();

			VkImageMemoryBarrier ImageMemoryBarrier(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueue = VK_QUEUE_FAMILY_IGNORED, uint32_t dstQueue = VK_QUEUE_FAMILY_IGNORED);
			VkImageMemoryBarrier ImageMemoryBarrier();

			VkBufferMemoryBarrier BufferMemoryBarrier();

			VkMemoryBarrier MemoryBarrier();

			VkImageCreateInfo ImageCreateInfo();

			VkSamplerCreateInfo SamplerCreateInfo();

			VkImageViewCreateInfo ImageViewCreateInfo();

			VkSemaphoreCreateInfo SemaphoreCreateInfo();

			VkFenceCreateInfo FenceCreateInfo(
				VkFenceCreateFlags flags);

			VkEventCreateInfo EventCreateInfo();

			VkSubmitInfo SubmitInfo();

			VkImageSubresourceRange ImageSubresourceRange(
				VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT,
				uint32_t startMip = 0);

			VkViewport Viewport(
				float width,
				float height,
				float minDepth,
				float maxDepth);

			VkRect2D Rect2D(
				int32_t width,
				int32_t height,
				int32_t offsetX,
				int32_t offsetY);

			VkBufferCreateInfo BufferCreateInfo();

			VkBufferCreateInfo BufferCreateInfo(
				VkBufferUsageFlags usage,
				VkDeviceSize size);

			VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(
				uint32_t poolSizeCount,
				VkDescriptorPoolSize* pPoolSizes,
				uint32_t maxSets);

			VkDescriptorPoolSize DescriptorPoolSize(
				VkDescriptorType type,
				uint32_t descriptorCount);

			VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(
				VkDescriptorType type,
				VkShaderStageFlags stageFlags,
				uint32_t binding,
				uint32_t count);

			VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(
				const VkDescriptorSetLayoutBinding* pBindings,
				uint32_t bindingCount);

			VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo(
				const VkDescriptorSetLayout* pSetLayouts,
				uint32_t setLayoutCount);

			VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(
				VkDescriptorPool descriptorPool,
				const VkDescriptorSetLayout* pSetLayouts,
				uint32_t descriptorSetCount);

			VkDescriptorBufferInfo DescriptorBufferInfo(
				VkBuffer buffer,
				VkDeviceSize offset,
				VkDeviceSize range);
			VkDescriptorImageInfo DescriptorImageInfo(
				VkSampler sampler,
				VkImageView imageView,
				VkImageLayout imageLayout);

			VkWriteDescriptorSet WriteDescriptorSet(
				VkDescriptorSet dstSet,
				VkDescriptorType type,
				uint32_t binding,
				VkDescriptorBufferInfo* bufferInfo);

			VkWriteDescriptorSet WriteDescriptorSet(
				VkDescriptorSet dstSet,
				VkDescriptorType type,
				uint32_t binding,
				VkDescriptorImageInfo* bufferInfo);


			VkVertexInputBindingDescription VertexInputBindingDescription(
				uint32_t binding,
				uint32_t stride,
				VkVertexInputRate inputRate);

			VkVertexInputAttributeDescription VertexInputAttributeDescription(
				uint32_t binding,
				uint32_t location,
				VkFormat format,
				uint32_t offset);

			VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo();

			VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo(
				VkPrimitiveTopology topology,
				VkPipelineInputAssemblyStateCreateFlags flags,
				VkBool32 primitiveRestartEnable);

			VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo(
				VkPolygonMode polygonMode,
				VkCullModeFlags cullMode,
				VkFrontFace frontFace,
				VkPipelineRasterizationStateCreateFlags flags);

			VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState(
				VkColorComponentFlags colorWriteMask,
				VkBool32 blendEnable);

			VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo(
				uint32_t attachmentCount,
				const VkPipelineColorBlendAttachmentState* pAttachments);

			VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo(
				VkBool32 depthTestEnable,
				VkBool32 depthWriteEnable,
				VkCompareOp depthCompareOp);

			VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo(
				uint32_t viewportCount,
				uint32_t scissorCount,
				VkPipelineViewportStateCreateFlags flags);

			VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo(
				VkSampleCountFlagBits rasterizationSamples,
				VkPipelineMultisampleStateCreateFlags flags);

			VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(
				const VkDynamicState* pDynamicStates,
				uint32_t dynamicStateCount,
				VkPipelineDynamicStateCreateFlags flags);

			VkPipelineTessellationStateCreateInfo PipelineTessellationStateCreateInfo(
				uint32_t patchControlPoints);

			VkGraphicsPipelineCreateInfo PipelineCreateInfo(
				VkPipelineLayout layout,
				VkRenderPass renderPass,
				VkPipelineCreateFlags flags);

			VkComputePipelineCreateInfo ComputePipelineCreateInfo(
				VkPipelineLayout layout,
				VkPipelineCreateFlags flags);

			VkPushConstantRange PushConstantRange(
				VkShaderStageFlags stageFlags,
				uint32_t size,
				uint32_t offset);

			VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(
				VkShaderStageFlagBits stage,
				VkShaderModule module,
				const char* entryName);
		}
	}
}