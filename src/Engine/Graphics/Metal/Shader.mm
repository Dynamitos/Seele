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
    auto [kernelBlob, entryPoint] = generateShader(createInfo);
    std::ofstream test("test.metal");
    test.write((char*)kernelBlob->getBufferPointer(), kernelBlob->getBufferSize());
    test.close();
    hash = CRC::Calculate(kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(), CRC::CRC_32());
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
