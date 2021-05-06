#include "Material.h"
#include "Asset/AssetRegistry.h"
#include "Graphics/VertexShaderInput.h"
#include "BRDF.h"
#include "Window/WindowManager.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <iostream>

Gfx::ShaderMap Material::shaderMap;
std::mutex Material::shaderMapLock;

using namespace Seele;
using json = nlohmann::json;

Material::Material()
{
}

Material::Material(const std::string& directory, const std::string& name) 
    : MaterialAsset(directory, name)
{
}

Material::Material(const std::filesystem::path& fullPath)
    : MaterialAsset(fullPath)
{
}

Material::~Material()
{
}

void Material::save() 
{
}

void Material::load() 
{
}


void Material::compile()
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
    codeStream << "import LightEnv;" << std::endl;
    codeStream << "import Material;" << std::endl;
    codeStream << "import BRDF;" << std::endl;
    codeStream << "import MaterialParameter;" << std::endl;

    codeStream << "struct " << materialName << ": IMaterial {" << std::endl;
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
            codeStream << "layout(offset = " << uniformBufferOffset << ")";
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
            codeStream << "layout(offset = " << uniformBufferOffset << ")";
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
                p->data = AssetRegistry::findTexture(defaultValue.value().get<std::string>());
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
        codeStream << type << " " << param.key() << ";\n";
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
    layout->create();
    BRDF* brdf = BRDF::getBRDFByName(profile);
    brdf->generateMaterialCode(codeStream, j["code"]);
    codeStream << "};";
    codeStream.close();
    setStatus(Status::Ready);
}

const Gfx::ShaderCollection* Material::getShaders(Gfx::RenderPassType renderPass, VertexInputType* vertexInput) const
{
    Gfx::ShaderPermutation permutation;
    permutation.passType = renderPass;
    std::string vertexInputName = vertexInput->getName();
    std::memcpy(permutation.materialName, materialName.c_str(), sizeof(permutation.materialName));
    std::memcpy(permutation.vertexInputName, vertexInputName.c_str(), sizeof(permutation.vertexInputName));
    return shaderMap.findShaders(Gfx::PermutationId(permutation));
}

Gfx::ShaderCollection& Material::createShaders(Gfx::PGraphics graphics, Gfx::RenderPassType renderPass, VertexInputType* vertexInput) 
{
    std::lock_guard lock(shaderMapLock);
    return shaderMap.createShaders(graphics, renderPass, this, vertexInput, false);
}
