#include "Material.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include "MaterialInstance.h"
#include "Serialization/Serialization.h"
#include "Window/WindowManager.h"
#include <fstream>

using namespace Seele;

Array<Gfx::PTexture2D> Material::textures;
Array<Gfx::PSampler> Material::samplers;
Gfx::OShaderBuffer Material::floatBuffer;
Array<float> Material::floatData;
Gfx::ODescriptorLayout Material::layout;
Gfx::PDescriptorSet Material::set;
std::atomic_uint64_t Material::materialIdCounter = 0;
Array<PMaterial> Material::materials;

Material::Material() {}

Material::Material(Gfx::PGraphics graphics, uint32 numTextures, uint32 numSamplers, uint32 numFloats, bool twoSided, float opacity,
                   std::string materialName, Array<OShaderExpression> expressions, Array<std::string> parameter, MaterialNode brdf)
    : graphics(graphics), numTextures(numTextures), numSamplers(numSamplers), numFloats(numFloats), twoSided(twoSided), opacity(opacity),
      instanceId(0), materialName(materialName), codeExpressions(std::move(expressions)), parameters(std::move(parameter)),
      brdf(std::move(brdf)), materialId(materialIdCounter++) {
    if (layout == nullptr) {
        init(graphics);
    }
    materials.add(this);
}

Material::~Material() {}

void Material::init(Gfx::PGraphics graphics) {
    layout = graphics->createDescriptorLayout("pMaterial");
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = 2000,
        .bindingFlags = Gfx::SE_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
        .shaderStages = Gfx::SE_SHADER_STAGE_FRAGMENT_BIT | Gfx::SE_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = 2000,
        .bindingFlags = Gfx::SE_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
        .shaderStages = Gfx::SE_SHADER_STAGE_FRAGMENT_BIT | Gfx::SE_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 2,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .bindingFlags = Gfx::SE_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
        .shaderStages = Gfx::SE_SHADER_STAGE_FRAGMENT_BIT | Gfx::SE_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
    });
    layout->create();
    floatBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .dynamic = true,
        .name = "MaterialFloatBuffer",
    });
}

void Material::destroy() {
    floatBuffer = nullptr;
    set = nullptr;
    layout = nullptr;
}

void Material::updateDescriptor() {
    floatBuffer->rotateBuffer(floatData.size() * sizeof(float));
    floatBuffer->updateContents(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = floatData.size() * sizeof(float),
                .data = (uint8*)floatData.data(),
            },
    });
    floatBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                 Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    layout->reset();
    set = layout->allocateDescriptorSet();
    for (uint32 i = 0; i < textures.size(); ++i) {
        if (textures[i] != nullptr) {
            set->updateTexture(0, i, textures[i]);
        }
    }
    for (uint32 i = 0; i < samplers.size(); ++i) {
        if (samplers[i] != nullptr) {
            set->updateSampler(1, i, samplers[i]);
        }
    }
    set->updateBuffer(2, floatBuffer);
    set->writeChanges();
}

void Material::updateTexture(uint32 index, Gfx::PTexture2D texture) { textures[index] = texture; }

void Material::updateSampler(uint32 index, Gfx::PSampler sampler) { samplers[index] = sampler; }

void Material::updateFloatData(uint32 offset, uint32 numFloats, float* data) {
    std::memcpy(floatData.data() + offset, data, numFloats * sizeof(float));
}

uint32 Material::addTextures(uint32 numTextures) {
    uint32 textureOffset = textures.size();
    textures.resize(textures.size() + numTextures);
    return textureOffset;
}

uint32 Material::addSamplers(uint32 numSamplers) {
    uint32 samplerOffset = samplers.size();
    samplers.resize(samplers.size() + numSamplers);
    return samplerOffset;
}

uint32 Material::addFloats(uint32 numFloats) {
    uint32 floatOffset = floatData.size();
    floatData.resize(floatData.size() + numFloats);
    return floatOffset;
}

OMaterialInstance Material::instantiate() {
    return new MaterialInstance(instanceId++, graphics, codeExpressions, parameters, numTextures, numSamplers, numFloats);
}
bool Material::isTwoSided() const { return twoSided; }
bool Material::hasTransparency() const { return opacity != 1.0f; }
float Material::getOpacity() const { return opacity; }

void Material::save(ArchiveBuffer& buffer) const {
    Serialization::save(buffer, numTextures);
    Serialization::save(buffer, numSamplers);
    Serialization::save(buffer, numFloats);
    Serialization::save(buffer, twoSided);
    Serialization::save(buffer, opacity);
    Serialization::save(buffer, instanceId);
    Serialization::save(buffer, materialName);
    Serialization::save(buffer, codeExpressions);
    Serialization::save(buffer, parameters);
    Serialization::save(buffer, brdf);
}

void Material::load(ArchiveBuffer& buffer) {
    graphics = buffer.getGraphics();
    if (layout == nullptr) {
        init(graphics);
    }
    Serialization::load(buffer, numTextures);
    Serialization::load(buffer, numSamplers);
    Serialization::load(buffer, numFloats);
    Serialization::load(buffer, twoSided);
    Serialization::load(buffer, opacity);
    Serialization::load(buffer, instanceId);
    Serialization::load(buffer, materialName);
    Serialization::load(buffer, codeExpressions);
    Serialization::load(buffer, parameters);
    Serialization::load(buffer, brdf);
    materialId = materialIdCounter++;
}

void Material::compile() {
    std::ofstream codeStream("./shaders/generated/" + materialName + ".slang");
    codeStream << "import BRDF;\n";
    codeStream << "import MaterialParameter;\n";
    codeStream << "import Material;\n";
    codeStream << "struct Material{\n";
    codeStream << "\ttypedef " << brdf.profile << " BRDF;\n";
    codeStream << "\tstatic " << brdf.profile << " prepare(MaterialParameter input) {\n";
    codeStream << "\t\t" << brdf.profile << " result;\n";
    Map<std::string, std::string> varState;
    // initialize variable state
    for (const auto& expr : codeExpressions) {
        codeStream << "\t\t" << expr->evaluate(varState);
    }
    for (const auto& [name, exp] : brdf.variables) {
        codeStream << "\t\tresult." << name << " = " << varState[exp] << ";" << std::endl;
    }
    codeStream << "\t\treturn result;\n";
    codeStream << "\t}\n";
    codeStream << "};\n";
    graphics->getShaderCompiler()->registerMaterial(this);
}

uint64 Material::getCPUSize() const { 
    uint64 result = sizeof(Material);
    for (size_t i = 0; i < parameters.size(); ++i) {
        result += parameters[i].size();
    }
    for (const auto& expr : codeExpressions)
    {
        result += expr->getCPUSize();
    }
    return result;
}

uint64 Material::getGPUSize() const
{
    uint64 result = 0;
    for (const auto& expr : codeExpressions) {
        result += expr->getGPUSize();
    }
    return result;
}
