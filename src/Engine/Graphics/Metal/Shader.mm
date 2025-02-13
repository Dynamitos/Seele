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
#include <regex>

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
    auto [kernelBlob, entryPoint] = generateShader(createInfo);
    hash = CRC::Calculate(kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(), CRC::CRC_32());
    std::regex pattern("\\[\\[buffer\\(\\d+\\)\\]\\]");
    
    std::string codeStr = std::string((const char*)kernelBlob->getBufferPointer());
    auto matches_begin = std::sregex_iterator(codeStr.begin(), codeStr.end(), pattern);
    auto matches_end = std::sregex_iterator();

    for (auto it = matches_begin; it != matches_end; ++it) {
        usedDescriptors.add(std::atoi((*it).str().c_str()+9)); // [[buffer( is 9 characters
    }
    
    NS::Error* error;
    MTL::CompileOptions* options = MTL::CompileOptions::alloc()->init();
    library = graphics->getDevice()->newLibrary(NS::String::string((char*)kernelBlob->getBufferPointer(), NS::ASCIIStringEncoding), options,
                                                &error);
    options->release();
    if (error) {
        std::cout << error->localizedDescription()->cString(NS::ASCIIStringEncoding) << std::endl;
    }
    function = library->newFunction(NS::String::string(entryPoint.c_str(), NS::ASCIIStringEncoding));
    if (!function) {
        assert(false);
    }
}

uint32 Shader::getShaderHash() const { return hash; }
