#pragma once
#include "MinimalEngine.h"
namespace Seele
{
	struct GraphicsInitializer
	{
		const char* windowLayoutFile;
	};
	struct WindowCreateInfo
	{
		int32 width;
		int32 height;
		const char* title;
		bool bFullscreen;
	};
	
	class RenderCommandBase
	{

	};
	DECLARE_REF(RenderCommandBase);

	class DescriptorBinding
	{
	public:
		DescriptorBinding()
			: binding(0)
			, descriptorType(VK_DESCRIPTOR_TYPE_MAX_ENUM)
			, descriptorCount(-1)
			, shaderStages(VK_SHADER_STAGE_ALL)
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
		//TODO make generic enum
		VkDescriptorType descriptorType;
		uint32_t descriptorCount;
		VkShaderStageFlags shaderStages;
	};
	DECLARE_REF(DescriptorBinding);

	class DescriptorAllocator
	{

	};
	DECLARE_REF(DescriptorAllocator);

	class DescriptorSet
	{

	};
	DECLARE_REF(DescriptorSet);

	class DescriptorLayout
	{

	};
	DECLARE_REF(DescriptorLayout);
	class PipelineLayout
	{

	};
	DECLARE_REF(PipelineLayout);
	class VertexDeclaration
	{

	};
	DECLARE_REF(VertexDeclaration);
	class GraphicsPipeline
	{

	};
	DECLARE_REF(GraphicsPipeline);
	class UniformBuffer
	{

	};
	DECLARE_REF(UniformBuffer);
	class Viewport
	{

	};
	DECLARE_REF(Viewport);
	class VertexBuffer
	{

	};
	DECLARE_REF(VertexBuffer);
	class IndexBuffer
	{

	};
	DECLARE_REF(IndexBuffer);
	class StructuredBuffer
	{

	};
	DECLARE_REF(StructuredBuffer);
	class Texture
	{

	};
	DECLARE_REF(Texture);
	class Texture2D : public Texture
	{

	};
	DECLARE_REF(Texture2D);
	class RenderPass
	{

	};
	DECLARE_REF(RenderPass);
}