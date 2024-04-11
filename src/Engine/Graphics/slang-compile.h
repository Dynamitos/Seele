#pragma once
#include "Graphics/Initializer.h"
#include <slang-com-ptr.h>
#include <slang.h>

namespace Seele {
Slang::ComPtr<slang::IBlob> generateShader(const ShaderCreateInfo& createInfo, SlangCompileTarget target);
}