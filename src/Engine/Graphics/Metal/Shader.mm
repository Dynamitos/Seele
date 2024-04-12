#include "Shader.h"
#include "Graphics.h"
#include "Graphics/Enums.h"
#include "Graphics/slang-compile.h"
#include <iostream>
#include </usr/local/include/metal_irconverter/metal_irconverter.h>
#include <slang.h>

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
      generateShader(createInfo, SLANG_DXIL);
  thread_local IRCompiler *pCompiler = nullptr;
  if (pCompiler == nullptr) {
    pCompiler = IRCompilerCreate();
  }
  IRCompilerSetEntryPointName(pCompiler, "main");

  IRObject *pDXIL = IRObjectCreateFromDXIL(
      (const uint8 *)kernelBlob->getBufferPointer(),
      kernelBlob->getBufferSize(), IRBytecodeOwnershipNone);

  // Compile DXIL to Metal IR:
  IRError *pError = nullptr;
  IRObject *pOutIR =
      IRCompilerAllocCompileAndLink(pCompiler, NULL, pDXIL, &pError);

  if (!pOutIR) {
    // Inspect pError to determine cause.
    IRErrorDestroy(pError);
  }
  IRShaderStage irStage;
  switch (stage) {
  case Gfx::SE_SHADER_STAGE_VERTEX_BIT: irStage = IRShaderStageVertex;
  case Gfx::SE_SHADER_STAGE_FRAGMENT_BIT: irStage = IRShaderStageFragment;
  case Gfx::SE_SHADER_STAGE_COMPUTE_BIT: irStage = IRShaderStageCompute;
  case Gfx::SE_SHADER_STAGE_TASK_BIT_NV: irStage = IRShaderStageAmplification;
  case Gfx::SE_SHADER_STAGE_MESH_BIT_NV: irStage = IRShaderStageMesh;
  }
  // Retrieve Metallib:
  IRMetalLibBinary *pMetallib = IRMetalLibBinaryCreate();
  IRObjectGetMetalLibBinary(pOutIR, irStage, pMetallib);
  size_t metallibSize = IRMetalLibGetBytecodeSize(pMetallib);
  uint8_t *metallib = new uint8_t[metallibSize];
  IRMetalLibGetBytecode(pMetallib, metallib);

  IRShaderReflection* reflection = IRShaderReflectionCreate();
  IRObjectGetReflection(pOutIR, irStage, reflection);
  Array<IRResourceLocation> locations(
  IRShaderReflectionGetResourceCount(reflection));
  IRShaderReflectionGetResourceLocations(reflection, locations.data());
  for(auto l : locations)
  {
    std::cout << "Resource " << l.resourceName << " Type " << l.resourceType << " size " << l.sizeBytes << " slot " << l.slot << " space " << l.space << " offset " << l.topLevelOffset << std::endl;
  }
  IRShaderReflectionDestroy(reflection);

  // Store the metallib to custom format or disk, or use to create a MTLLibrary.
  NS::Error *__autoreleasing error = nil;
  dispatch_data_t data = dispatch_data_create(metallib, metallibSize,
                                              dispatch_get_main_queue(), NULL);

  library = graphics->getDevice()->newLibrary(data, &error);
  function =
      library->newFunction(NS::String::string("main", NS::ASCIIStringEncoding));

  delete[] metallib;
  IRMetalLibBinaryDestroy(pMetallib);
  IRObjectDestroy(pDXIL);
  IRObjectDestroy(pOutIR);
  IRCompilerDestroy(pCompiler);
}