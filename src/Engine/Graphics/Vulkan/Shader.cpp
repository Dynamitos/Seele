#include "Shader.h"
#include "Graphics.h"
#include "Graphics/slang-compile.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "stdlib.h"
#include <fmt/core.h>


using namespace Seele;
using namespace Seele::Vulkan;

Shader::Shader(PGraphics graphics, VkShaderStageFlags stage) : graphics(graphics), stage(stage) {}

Shader::~Shader() {
    if (module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(graphics->getDevice(), module, nullptr);
    }
}

uint32 Seele::Vulkan::Shader::getShaderHash() const { return hash; }

void Shader::create(const ShaderCreateInfo& createInfo) {
    Slang::ComPtr<slang::IBlob> kernelBlob = generateShader(createInfo);
    VkShaderModuleCreateInfo moduleInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = kernelBlob->getBufferSize(),
        .pCode = (uint32_t*)kernelBlob->getBufferPointer(),
    };
    VK_CHECK(vkCreateShaderModule(graphics->getDevice(), &moduleInfo, nullptr, &module));

    hash = CRC::Calculate(kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(), CRC::CRC_32(), hash);
}