#pragma once
#include "GraphicsEnums.h"
#include "Containers/Array.h"
#include "Math/MemCRC.h"

#ifdef _DEBUG
#define ENABLE_VALIDATION
#endif

namespace Seele
{
	struct SePushConstantRange {
		SeShaderStageFlags    stageFlags;
		uint32_t              offset;
		uint32_t              size;
	};

	struct GraphicsInitializer
	{
		const char* windowLayoutFile;
		const char* applicationName;
		const char* engineName;
		void* windowHandle;
		/**
		 * layers defines the enabled Vulkan layers used in the instance,
		 * if ENABLE_VALIDATION is defined, standard validation is already enabled
		 * not yet implemented
		 */
		Array<const char*> layers;
		Array<const char*> instanceExtensions;
		Array<const char*> deviceExtensions;
		GraphicsInitializer()
			: applicationName("SeeleEngine")
			, engineName("SeeleEngine")
			, layers{ "VK_LAYER_LUNARG_standard_validation" }
			, instanceExtensions{}
			, deviceExtensions{ "VK_KHR_swapchain" }
			, windowHandle(nullptr)
		{}
		GraphicsInitializer(const GraphicsInitializer& other)
			: applicationName(other.applicationName)
			, engineName(other.engineName)
			, layers(other.layers)
			, instanceExtensions(other.instanceExtensions)
			, deviceExtensions(other.deviceExtensions)
		{
		}
	};
	DECLARE_REF(Graphics);
	struct WindowCreateInfo
	{
		int32 width;
		int32 height;
		const char* title;
		bool bFullscreen;
		PGraphics graphics;
	};
	
	class RenderCommandBase
	{

	};
	DEFINE_REF(RenderCommandBase);
	class SamplerState
	{
	public:
		virtual ~SamplerState()
		{

		}
	};
	DEFINE_REF(SamplerState);
	class DescriptorBinding
	{
	public:
		DescriptorBinding()
			: binding(0)
			, descriptorType(SE_DESCRIPTOR_TYPE_MAX_ENUM)
			, descriptorCount(-1)
			, shaderStages(SE_SHADER_STAGE_ALL)
		{}
		DescriptorBinding(const DescriptorBinding& other)
			: binding(other.binding)
			, descriptorType(other.descriptorType)
			, descriptorCount(other.descriptorCount)
			, shaderStages(other.shaderStages)
		{}
		void operator=(const DescriptorBinding& other)
		{
			binding = other.binding;
			descriptorType = other.descriptorType;
			descriptorCount = other.descriptorCount;
			shaderStages = other.shaderStages;
		}
		uint32_t binding;
		SeDescriptorType descriptorType;
		uint32_t descriptorCount;
		SeShaderStageFlags shaderStages;
	};
	DEFINE_REF(DescriptorBinding);

	DECLARE_REF(DescriptorSet);
	class DescriptorAllocator
	{
	public:
		DescriptorAllocator() {}
		~DescriptorAllocator() {}
		virtual void allocateDescriptorSet(PDescriptorSet& descriptorSet) = 0;
	};
	DEFINE_REF(DescriptorAllocator);
	DECLARE_REF(UniformBuffer);
	DECLARE_REF(StructuredBuffer);
	DECLARE_REF(Texture);
	class DescriptorSet
	{
	public:
		virtual ~DescriptorSet() {}
		virtual void writeChanges() = 0;
		virtual void updateBuffer(uint32 binding, PUniformBuffer uniformBuffer) = 0;
		virtual void updateBuffer(uint32 binding, PStructuredBuffer structuredBuffer) = 0;
		virtual void updateSampler(uint32 binding, PSamplerState samplerState) = 0;
		virtual void updateTexture(uint32 binding, PTexture texture, PSamplerState samplerState = nullptr) = 0;
		virtual bool operator<(PDescriptorSet other) = 0;
	};
	DEFINE_REF(DescriptorSet);

	class DescriptorLayout
	{
	public:
		DescriptorLayout() {}
		virtual ~DescriptorLayout(){}
		void operator=(const DescriptorLayout& other)
		{
			descriptorBindings.resize(other.descriptorBindings.size());
			std::memcpy(descriptorBindings.data(), other.descriptorBindings.data(), sizeof(DescriptorLayout) * descriptorBindings.size());
		}
		virtual void create() = 0;
		virtual void addDescriptorBinding(uint32 binding, SeDescriptorType type, uint32 arrayCount = 1);
		virtual PDescriptorSet allocatedDescriptorSet();
		const Array<DescriptorBinding>& getBindings() const { return descriptorBindings; }
	protected:
		Array<DescriptorBinding> descriptorBindings;
		PDescriptorAllocator allocator;
		friend class PipelineLayout;
		friend class DescriptorAllocator;
	};
	DEFINE_REF(DescriptorLayout);
	class PipelineLayout
	{
	public:
		PipelineLayout() {}
		virtual ~PipelineLayout() {}
		virtual void create() = 0;
		void addDescriptorLayout(uint32 setIndex, PDescriptorLayout layout);
		void addPushConstants(const SePushConstantRange& pushConstants);
		virtual uint32  getHash() const = 0;
	protected:
		Array<PDescriptorLayout> descriptorSetLayouts;
		Array<SePushConstantRange> pushConstants;
	};
	DEFINE_REF(PipelineLayout);
	class VertexDeclaration
	{

	};
	DEFINE_REF(VertexDeclaration);
	class GraphicsPipeline
	{
	public:
		virtual ~GraphicsPipeline()
		{

		}
	};
	DEFINE_REF(GraphicsPipeline);
	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer()
		{

		}
	};
	DEFINE_REF(UniformBuffer);
	class Viewport
	{

	};
	DEFINE_REF(Viewport);
	class VertexBuffer
	{

	};
	DEFINE_REF(VertexBuffer);
	class IndexBuffer
	{

	};
	DEFINE_REF(IndexBuffer);
	class StructuredBuffer
	{
	public:
		virtual ~StructuredBuffer()
		{

		}
	};
	DEFINE_REF(StructuredBuffer);
	class Texture
	{

	};
	DEFINE_REF(Texture);
	class Texture2D : public Texture
	{

	};
	DEFINE_REF(Texture2D);
	class RenderPass
	{

	};
	DEFINE_REF(RenderPass);
}