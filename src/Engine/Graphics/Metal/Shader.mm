#include "Shader.h"
#include "Foundation/NSError.hpp"
#include "Foundation/NSString.hpp"
#include "Graphics.h"
#include "Graphics/Enums.h"
#include "Graphics/slang-compile.h"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLLibrary.hpp"
#include "Descriptor.h"
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
    Map<std::string, uint32> test;
  Slang::ComPtr<slang::IBlob> kernelBlob = generateShader(createInfo, SLANG_DXIL, test);
  hash = CRC::Calculate(kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(), CRC::CRC_32());
  IRCompiler* pCompiler = IRCompilerCreate();
    IRCompilerSetMinimumGPUFamily(pCompiler, IRGPUFamilyMetal3);
    IRCompilerIgnoreRootSignature(pCompiler, true);
  IRCompilerSetEntryPointName(pCompiler, "main");

  IRObject* pDXIL = IRObjectCreateFromDXIL((const uint8*)kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(),
                                           IRBytecodeOwnershipNone);
    IRVersionedRootSignatureDescriptor descriptor;
    descriptor.version = IRRootSignatureVersion_1_1;
    Map<std::string, Gfx::PDescriptorLayout> layouts =createInfo.rootSignature->getLayouts();
    descriptor.desc_1_1.NumParameters = layouts.size();
    descriptor.desc_1_1.NumStaticSamplers = 0;
    descriptor.desc_1_1.pStaticSamplers = nullptr;
    Array<IRRootParameter1> parameters;
    // each layout has an array for its bindings
    Array<Array<IRDescriptorRange1>> ranges;
    for(const auto& [name, layout] : layouts)
    {
        uint32 textureReg = 0; // SRV
        uint32 samplerReg = 0; // Sampler
        uint32 uavReg = 0; // UAV
        uint32 constantReg = 0; // CBV
        auto& bindingRanges = ranges.add();
        for(auto binding : layout->getBindings())
        {
            IRDescriptorRangeType type;
            uint32 reg = 0;
            switch(binding.descriptorType)
            {
                case Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                case Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    type = IRDescriptorRangeTypeSRV;
                    reg = textureReg++;
                    break;
                case Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                case Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                    type = IRDescriptorRangeTypeCBV;
                    reg = constantReg++;
                    break;
                case Gfx::SE_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                case Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    type = IRDescriptorRangeTypeUAV;
                    reg = uavReg++;
                    break;
                default: throw std::logic_error("Not implemented");
            };
            bindingRanges.add() = IRDescriptorRange1{
                .BaseShaderRegister = reg,
                .NumDescriptors = binding.descriptorCount,
                .RangeType = type,
                .RegisterSpace = createInfo.rootSignature->findParameter(name),
            };
        }
        parameters.add() = IRRootParameter1{
            .ParameterType = IRRootParameterTypeDescriptorTable,
            .DescriptorTable = IRRootDescriptorTable1{ .NumDescriptorRanges = (uint32)(bindingRanges.size()), bindingRanges.data() },
            .ShaderVisibility = IRShaderVisibilityAll,
        };
    }
    IRError* signatureError = nullptr;
    descriptor.desc_1_1.NumParameters = parameters.size();
    descriptor.desc_1_1.pParameters = parameters.data();
    IRRootSignature* rootSignature = IRRootSignatureCreateFromDescriptor(&descriptor, &signatureError);
    assert(rootSignature);
    IRCompilerSetGlobalRootSignature(pCompiler, rootSignature);
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
    
    IRShaderReflection* reflection = IRShaderReflectionCreate();
    IRObjectGetReflection(pOutIR, irStage, reflection);
    std::cout << " NumRes: " << IRShaderReflectionGetResourceCount(reflection) << std::endl;
    std::cout << IRShaderReflectionAllocStringAndSerialize(reflection) << std::endl;
    IRShaderReflectionDestroy(reflection);
        
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
