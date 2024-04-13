#include "Shader.h"
#include "Foundation/NSError.hpp"
#include "Foundation/NSString.hpp"
#include "Graphics.h"
#include "Graphics/Enums.h"
#include "Graphics/slang-compile.h"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLLibrary.hpp"
#include <iostream>
#include <slang.h>
#include <spirv_cross.hpp>
#include <spirv_cross/spirv_msl.hpp>

using namespace Seele;
using namespace Seele::Metal;

Shader::Shader(PGraphics graphics, Gfx::SeShaderStageFlags stage)
    : stage(stage), graphics(graphics) {}
Shader::~Shader() {
  if (function) {
    function->release();
    library->release();
  }
}

void Shader::create(const ShaderCreateInfo &createInfo) {
  Slang::ComPtr<slang::IBlob> kernelBlob =
      generateShader(createInfo, SLANG_SPIRV);
  spirv_cross::CompilerMSL comp((const uint32 *)kernelBlob->getBufferPointer(),
                                kernelBlob->getBufferSize() / 4);
  auto options = comp.get_msl_options();
  options.argument_buffers = true;
  options.argument_buffers_tier =
      spirv_cross::CompilerMSL::Options::ArgumentBuffersTier::Tier2;
  options.set_msl_version(3);
  comp.set_msl_options(options);
  std::string metalCode = comp.compile();
  NS::Error *error = nullptr;
  MTL::CompileOptions *mtlOptions = MTL::CompileOptions::alloc()->init();

  library = graphics->getDevice()->newLibrary(
      NS::String::string(metalCode.c_str(), NS::ASCIIStringEncoding),
      mtlOptions, &error);
  if (error) {
    std::cout << error->debugDescription() << std::endl;
    assert(false);
  }
  function =
      library->newFunction(NS::String::string("main", NS::ASCIIStringEncoding));
  mtlOptions->release();
}