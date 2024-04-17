#include "Shader.h"
#include "Foundation/NSError.hpp"
#include "Foundation/NSString.hpp"
#include "Graphics.h"
#include "Graphics/Enums.h"
#include "Graphics/slang-compile.h"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLLibrary.hpp"
#include <fstream>
#include <iostream>
#include <metal_irconverter/metal_irconverter.h>
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
  Slang::ComPtr<slang::IBlob> kernelBlob = generateShader(createInfo, SLANG_DXIL);

  hash = CRC::Calculate(kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(), CRC::CRC_32());
  IRCompiler* pCompiler = IRCompilerCreate();
  IRCompilerSetEntryPointName(pCompiler, "main");

  IRObject* pDXIL = IRObjectCreateFromDXIL((const uint8*)kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(),
                                           IRBytecodeOwnershipNone);

  // Compile DXIL to Metal IR:
  IRError* pError = nullptr;
  IRObject* pOutIR = IRCompilerAllocCompileAndLink(pCompiler, NULL, pDXIL, &pError);

  if (!pOutIR) {
    // Inspect pError to determine cause.
    IRErrorDestroy(pError);
  }
  IRShaderStage irStage;
  switch (stage) {
  case Gfx::SE_SHADER_STAGE_VERTEX_BIT:
    irStage = IRShaderStageVertex;
    break;
  case Gfx::SE_SHADER_STAGE_FRAGMENT_BIT:
    irStage = IRShaderStageFragment;
    break;
  case Gfx::SE_SHADER_STAGE_COMPUTE_BIT:
    irStage = IRShaderStageCompute;
    break;
  case Gfx::SE_SHADER_STAGE_TASK_BIT_EXT:
    irStage = IRShaderStageAmplification;
    break;
  case Gfx::SE_SHADER_STAGE_MESH_BIT_EXT:
    irStage = IRShaderStageMesh;
    break;
  }
  // Retrieve Metallib:
  IRMetalLibBinary* pMetallib = IRMetalLibBinaryCreate();
  IRObjectGetMetalLibBinary(pOutIR, irStage, pMetallib);
  dispatch_data_t data = IRMetalLibGetBytecodeData(pMetallib);

  // Store the metallib to custom format or disk, or use to create a MTLLibrary.
  NS::Error* error;
  library = graphics->getDevice()->newLibrary(data, &error);
  function = library->newFunction(NS::String::string("main", NS::ASCIIStringEncoding));
  if(!function)
  {
    assert(false);
  }
  IRMetalLibBinaryDestroy(pMetallib);
  IRObjectDestroy(pDXIL);
  IRObjectDestroy(pOutIR);
  IRCompilerDestroy(pCompiler);
}

uint32 Shader::getShaderHash() const { return hash; }