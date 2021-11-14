#include "MaterialAsset.h"
#include "Window/WindowManager.h"
#include "Asset/AssetRegistry.h"
#include "Asset/TextureAsset.h"
#include "BRDF.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <iostream>

Gfx::ShaderMap MaterialAsset::shaderMap;
std::mutex MaterialAsset::shaderMapLock;

using namespace Seele;
using json = nlohmann::json;

using namespace Seele;

MaterialAsset::MaterialAsset()
{
}

MaterialAsset::MaterialAsset(const std::string& directory, const std::string& name) 
    : Asset(directory, name)
{
}

MaterialAsset::MaterialAsset(const std::filesystem::path& fullPath) 
    : Asset(fullPath)
{
}

MaterialAsset::~MaterialAsset()
{
}


void MaterialAsset::save()
{
    assert(false && "TODO");
}

void MaterialAsset::load()
{
    setStatus(Status::Loading);
    auto& stream = getReadStream();
    json j;
    stream >> j;
    materialName = j["name"].get<std::string>();
    layout = WindowManager::getGraphics()->createDescriptorLayout(materialName + "Layout");
    //Shader file needs to conform to the slang standard, which prohibits _
    materialName.erase(std::remove(materialName.begin(), materialName.end(), '_'), materialName.end());
    std::ofstream codeStream("./shaders/generated/"+materialName+".slang");
    std::string profile = j["profile"].get<std::string>();

    codeStream << "import VERTEX_INPUT_IMPORT;" << std::endl;
    codeStream << "import Material;" << std::endl;
    codeStream << "import BRDF;" << std::endl;
    codeStream << "import MaterialParameter;" << std::endl << std::endl;

    codeStream << "struct " << materialName << " : IMaterial {" << std::endl;
    uint32 uniformBufferOffset = 0;
    uint32 bindingCounter = 0; // Uniform buffers are always binding 0
    uniformBinding = -1;
    for(auto param : j["params"].items())
    {
        std::string type = param.value()["type"].get<std::string>();
        auto defaultValue = param.value().find("default");
        if(type.compare("float") == 0)
        {
            PFloatParameter p = new FloatParameter(param.key(), uniformBufferOffset, 0);
            codeStream << "\tlayout(offset = " << uniformBufferOffset << ")";
            if(uniformBinding == -1)
            {
                layout->addDescriptorBinding(bindingCounter, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
                uniformBinding = bindingCounter++;
            }
            uniformBufferOffset += 4;
            if(defaultValue != param.value().end())
            {
                p->data = std::stof(defaultValue.value().get<std::string>());
            }
            parameters.add(p);
        }
        else if(type.compare("float3") == 0)
        {
            PVectorParameter p = new VectorParameter(param.key(), uniformBufferOffset, 0);
            codeStream << "\tlayout(offset = " << uniformBufferOffset << ")";
            if(uniformBinding == -1)
            {
                layout->addDescriptorBinding(bindingCounter, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
                uniformBinding = bindingCounter++;
            }
            uniformBufferOffset += 12;
            if(defaultValue != param.value().end())
            {
                p->data = parseVector(defaultValue.value().get<std::string>().c_str());
            }
            parameters.add(p);
        }
        else if(type.compare("Texture2D") == 0)
        {
            PTextureParameter p = new TextureParameter(param.key(), 0, bindingCounter);
            layout->addDescriptorBinding(bindingCounter++, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
            if(defaultValue != param.value().end())
            {
                std::string defaultString = defaultValue.value().get<std::string>();
                p->data = AssetRegistry::findTexture(defaultString);
            }
            else
            {
                p->data = AssetRegistry::findTexture(""); // this will return placeholder texture
            }
            assert(p->data != nullptr);
            parameters.add(p);
        }
        else if(type.compare("SamplerState") == 0)
        {
            PSamplerParameter p = new SamplerParameter(param.key(), 0, bindingCounter);
            layout->addDescriptorBinding(bindingCounter++, Gfx::SE_DESCRIPTOR_TYPE_SAMPLER);
            SamplerCreateInfo createInfo;
            p->data = WindowManager::getGraphics()->createSamplerState(createInfo);
            parameters.add(p);
        }
        else
        {
            std::cout << "Error unsupported parameter type" << std::endl;
        }
        codeStream << "\t" << type << " " << param.key() << ";\n";
    }
    uniformDataSize = uniformBufferOffset;
    if(uniformDataSize != 0)
    {
        uniformData = new uint8[uniformDataSize];
        UniformBufferCreateInfo uniformInitializer;
        uniformInitializer.resourceData.data = uniformData;
        uniformInitializer.resourceData.size = uniformDataSize;
        uniformBuffer = WindowManager::getGraphics()->createUniformBuffer(uniformInitializer);
    }
    BRDF* brdf = BRDF::getBRDFByName(profile);
    brdf->generateMaterialCode(codeStream, j["code"]);
    codeStream << "};";
    codeStream.close();
    layout->create();
    setStatus(Status::Ready);
}


void MaterialAsset::beginFrame() 
{
}

void MaterialAsset::endFrame() 
{
}

Gfx::PDescriptorSet MaterialAsset::createDescriptorSet()
{
    Gfx::PDescriptorSet descriptorSet = layout->allocateDescriptorSet();
    BulkResourceData uniformUpdate;
    uniformUpdate.size = uniformDataSize;
    uniformUpdate.data = uniformData;
    for(auto param : parameters)
    {
        param->updateDescriptorSet(descriptorSet, uniformData);
    }
    if(uniformUpdate.size != 0)
    {
        uniformBuffer->updateContents(uniformUpdate);
        descriptorSet->updateBuffer(uniformBinding, uniformBuffer);
    }
    descriptorSet->writeChanges();
    return descriptorSet;
}


const Gfx::ShaderCollection* MaterialAsset::getShaders(Gfx::RenderPassType renderPass, VertexInputType* vertexInput) const
{
    Gfx::ShaderPermutation permutation;
    permutation.passType = renderPass;
    std::string vertexInputName = vertexInput->getName();
    std::memcpy(permutation.materialName, materialName.c_str(), sizeof(permutation.materialName));
    std::memcpy(permutation.vertexInputName, vertexInputName.c_str(), sizeof(permutation.vertexInputName));
    return shaderMap.findShaders(Gfx::PermutationId(permutation));
}

Gfx::ShaderCollection& MaterialAsset::createShaders(Gfx::PGraphics graphics, Gfx::RenderPassType renderPass, VertexInputType* vertexInput) 
{
    std::unique_lock lock(shaderMapLock);
    return shaderMap.createShaders(graphics, renderPass, this, vertexInput, false);
}
