#include "Shader.h"
#include "Graphics.h"
#include "Graphics/slang-compile.h"
#include "Metal/MTLLibrary.hpp"
#include <slang.h>

using namespace Seele;
using namespace Seele::Metal;

Shader::Shader(PGraphics graphics) : graphics(graphics) {}
Shader::~Shader() {
  if (function) {
    function->release();
    library->release();
  }
}

void Shader::create(const ShaderCreateInfo &createInfo) {
  Slang::ComPtr<slang::IBlob> kernelBlob = generateShader(createInfo, SLANG_DXIL);
  thread_local IRCompiler* pCompiler = nullptr;
  if(pCompiler == nullptr)
  {
    pCompiler = IRCompilerCreate();
  }
  IRCompilerSetEntryPointName(pCompiler, "main");

  IRObject* pDXIL = IRObjectCreateFromDXIL(kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(), IRBytecodeOwnershipNone);

  // Compile DXIL to Metal IR:
  IRError* pError = nullptr;
  IRObject* pOutIR = IRCompilerAllocCompileAndLink(pCompiler, NULL,  pDXIL, &pError);

  if (!pOutIR)
  {
    // Inspect pError to determine cause.
    IRErrorDestroy( pError );
  }

  // Retrieve Metallib:
  MetaLibBinary* pMetallib = IRMetalLibBinaryCreate();
  IRObjectGetMetalLibBinary(pOutIR, stage, pMetallib);
  size_t metallibSize = IRMetalLibGetBytecodeSize(pMetallib);
  uint8_t* metallib = new uint8_t[metallibSize];
  IRMetalLibGetBytecode(pMetallib, metallib);

  // Store the metallib to custom format or disk, or use to create a MTLLibrary.
  NS::Error* __autoreleasing error = nil;
  dispatch_data_t data = 
    dispatch_data_create(metallib, metallibSize, dispatch_get_main_queue(), NULL);

  library = graphics->getDevice()->newLibrary(data, &error);
  function = library->newFunction(NS::String::string("main", NS::ASCIIStringEncoding));
  
  delete [] metallib;
  IRMetalLibBinaryDestroy(pMetallib);
  IRObjectDestroy(pDXIL);
  IRObjectDestroy(pOutIR);
  IRCompilerDestroy(pCompiler);
}