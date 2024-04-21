#include "Shader.h"
#include "Descriptor.h"
#include "Foundation/NSError.hpp"
#include "Foundation/NSString.hpp"
#include "Graphics.h"
#include "Graphics/Enums.h"
#include "Graphics/slang-compile.h"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLLibrary.hpp"
#include <fstream>
#include <iostream>
#include <slang.h>

using namespace Seele;
using namespace Seele::Metal;

Shader::Shader(PGraphics graphics, Gfx::SeShaderStageFlags stage) : stage(stage), graphics(graphics) {}

Shader::~Shader() {
  if (function) {
    function->release();
    library->release();
  }
}

void Shader::create(const ShaderCreateInfo& createInfo) {
  std::cout << "Compiling " << createInfo.name << std::endl;
  Map<std::string, uint32> paramMapping;
  Slang::ComPtr<slang::IBlob> kernelBlob = generateShader(createInfo, SLANG_METAL, paramMapping);
    std::cout << (char*)kernelBlob->getBufferPointer() << std::endl;
  hash = CRC::Calculate(kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(), CRC::CRC_32());
    dispatch_data_t dispatchData = dispatch_data_create(kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(), dispatch_get_main_queue(), NULL);
    NS::Error* error;
    library = graphics->getDevice()->newLibrary(dispatchData, &error);
    if(error)
    {
        std::cout << error->localizedDescription()->cString(NS::ASCIIStringEncoding) << std::endl;
    }
  function = library->newFunction(NS::String::string("main", NS::ASCIIStringEncoding));
  if (!function) {
    assert(false);
  }
}

uint32 Shader::getShaderHash() const { return hash; }
