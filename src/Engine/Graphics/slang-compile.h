#pragma once
#include "Graphics/Initializer.h"
#include <slang-com-ptr.h>
#include <slang.h>

namespace Seele {
void beginCompilation(const ShaderCompilationInfo& info, SlangCompileTarget target, Gfx::PPipelineLayout layout);
Pair<Slang::ComPtr<slang::IBlob>, std::string> generateShader(const ShaderCreateInfo& createInfo);
}
