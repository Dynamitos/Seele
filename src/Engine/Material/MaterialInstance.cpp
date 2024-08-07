#include "MaterialInstance.h"
#include "Graphics/Graphics.h"
#include "Material.h"

using namespace Seele;

MaterialInstance::MaterialInstance() {}

MaterialInstance::MaterialInstance(uint64 id, Gfx::PGraphics graphics, Array<OShaderExpression>& expressions, Array<std::string> params,
                                   uint32 numTextures, uint32 numSamplers, uint32 numFloats)
    : graphics(graphics), numTextures(numTextures), numSamplers(numSamplers), numFloats(numFloats), id(id) {
    texturesOffset = Material::addTextures(numTextures);
    samplersOffset = Material::addSamplers(numSamplers);
    floatBufferOffset = Material::addFloats(numFloats);
    ArchiveBuffer buffer(graphics);
    parameters.reserve(params.size());
    for (size_t i = 0; i < params.size(); ++i) {
        const std::string& name = params[i];
        Serialization::save(buffer, *expressions.find([&name](const OShaderExpression& p) { return p->key == name; }));
        buffer.rewind();
        OShaderParameter param;
        Serialization::load(buffer, param);
        parameters.add(std::move(param));
        buffer.rewind();
    }
}

MaterialInstance::~MaterialInstance() {}

void MaterialInstance::updateDescriptor() {
    for (auto& p : parameters) {
        p->updateDescriptorSet(texturesOffset, samplersOffset, floatBufferOffset);
    }
}

void MaterialInstance::setBaseMaterial(PMaterialAsset asset) { baseMaterial = asset; }

void MaterialInstance::save(ArchiveBuffer& buffer) const {
    Serialization::save(buffer, numTextures);
    Serialization::save(buffer, numSamplers);
    Serialization::save(buffer, numFloats);
    Serialization::save(buffer, parameters);
    Serialization::save(buffer, id);
}

void MaterialInstance::load(ArchiveBuffer& buffer) {
    Serialization::load(buffer, numTextures);
    Serialization::load(buffer, numSamplers);
    Serialization::load(buffer, numFloats);
    Serialization::load(buffer, parameters);
    Serialization::load(buffer, id);
    texturesOffset = Material::addTextures(numTextures);
    samplersOffset = Material::addSamplers(numSamplers);
    floatBufferOffset = Material::addFloats(numFloats);
}

uint64 MaterialInstance::getCPUSize() const {
    uint64 result = sizeof(MaterialInstance);
    for (size_t i = 0; i < parameters.size(); ++i) {
        result += parameters[i]->getCPUSize();
    }
    return result;
}

uint64 MaterialInstance::getGPUSize() const {
    uint64 result = 0;
    for (size_t i = 0; i < parameters.size(); ++i) {
        result += parameters[i]->getGPUSize();
    }
    return result;
}
