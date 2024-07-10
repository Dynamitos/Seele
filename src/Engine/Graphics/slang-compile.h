#pragma once
#include "Graphics/Initializer.h"
#include <slang-com-ptr.h>
#include <slang.h>

namespace Seele {
void beginCompilation(const ShaderCompilationInfo& info, SlangCompileTarget target, Gfx::PPipelineLayout layout);
Slang::ComPtr<slang::IBlob> generateShader(const ShaderCreateInfo& createInfo);
}
