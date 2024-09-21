#include "VertexData.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Material/Material.h"
#include "Material/MaterialInstance.h"
#include <iostream>

using namespace Seele;

constexpr static uint64 NUM_DEFAULT_ELEMENTS = 17962284;
uint64 VertexData::meshletCount = 0;

void VertexData::resetMeshData() {
    std::unique_lock l(materialDataLock);
    instanceData.clear();
    instanceMeshData.clear();
    rayTracingScene.clear();
    transparentData.clear();
    for (auto& mat : materialData) {
        for (auto& inst : mat.instances) {
            inst.instanceData.clear();
            inst.instanceMeshData.clear();
        }
        if (mat.material != nullptr) {
            mat.material->getDescriptorLayout()->reset();
        }
    }
    if (dirty) {
        updateBuffers();
        dirty = false;
    }
}

void VertexData::updateMesh(uint32 meshletOffset, PMesh mesh, const Component::Transform& transform) {
    std::unique_lock l(materialDataLock);
    PMaterialInstance referencedInstance = mesh->referencedMaterial->getHandle();
    PMaterial mat = referencedInstance->getBaseMaterial();
    const auto& data = meshData[mesh->id];

    Matrix4 transformMatrix = transform.toMatrix() * mesh->transform;
    InstanceData inst = InstanceData{
        .transformMatrix = transformMatrix,
        .inverseTransformMatrix = glm::inverse(transformMatrix),
    };

    referencedInstance->updateDescriptor();
    if (mat->hasTransparency()) {
        auto params = referencedInstance->getMaterialOffsets();
        transparentData.add(TransparentDraw{
            .matInst = referencedInstance,
            .vertexData = this,
            .offsets =
                {
                    .instanceOffset = 0,
                    .textureOffset = params.textureOffset,
                    .samplerOffset = params.samplerOffset,
                    .floatOffset = params.floatOffset,
                },
            .worldPosition = Vector(inst.transformMatrix[3]),
            .instanceData = inst,
            .meshData = data,
            .cullingOffset = meshletOffset,
            .rayTracingScene = mesh->blas,
        });
        return;
    }
    if (materialData.size() <= mat->getId()) {
        materialData.resize(mat->getId() + 1);
    }
    MaterialData& matData = materialData[mat->getId()];
    matData.material = mat;
    if (matData.instances.size() <= referencedInstance->getId()) {
        matData.instances.resize(referencedInstance->getId() + 1);
    }
    BatchedDrawCall& matInstanceData = matData.instances[referencedInstance->getId()];
    matInstanceData.materialInstance = referencedInstance;
    matInstanceData.rayTracingData.add(mesh->blas);
    matInstanceData.instanceData.add(inst);
    matInstanceData.instanceMeshData.add(data);
    matInstanceData.cullingOffsets.add(meshletOffset);
    for (size_t i = 0; i < 0; ++i) {
        auto bounding = meshlets[data.meshletOffset + i].bounding;
        StaticArray<Vector, 8> corners;
        Vector min = bounding.min; // bounding.center - bounding.radius * Vector(1, 1, 1);
        Vector max = bounding.max; // bounding.center + bounding.radius * Vector(1, 1, 1);
        corners[0] = transformMatrix * Vector4(min.x, min.y, min.z, 1);
        corners[1] = transformMatrix * Vector4(min.x, min.y, max.z, 1);
        corners[2] = transformMatrix * Vector4(min.x, max.y, min.z, 1);
        corners[3] = transformMatrix * Vector4(min.x, max.y, max.z, 1);
        corners[4] = transformMatrix * Vector4(max.x, min.y, min.z, 1);
        corners[5] = transformMatrix * Vector4(max.x, min.y, max.z, 1);
        corners[6] = transformMatrix * Vector4(max.x, max.y, min.z, 1);
        corners[7] = transformMatrix * Vector4(max.x, max.y, max.z, 1);
        addDebugVertex(DebugVertex{.position = corners[0], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[1], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[0], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[2], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[1], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[3], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[2], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[3], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[0], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[4], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[1], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[5], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[2], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[6], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[3], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[7], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[4], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[5], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[4], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[6], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[6], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[7], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[5], .color = meshlets[data.meshletOffset + i].color});
        addDebugVertex(DebugVertex{.position = corners[7], .color = meshlets[data.meshletOffset + i].color});
    }
}

void VertexData::createDescriptors() {
    std::unique_lock l(materialDataLock);

    Array<uint32> cullingOffsets;
    for (auto& mat : materialData) {
        for (auto& instance : mat.instances) {
            instance.offsets.instanceOffset = instanceData.size();
            MaterialOffsets offsets = instance.materialInstance->getMaterialOffsets();
            instance.offsets.textureOffset = offsets.textureOffset;
            instance.offsets.samplerOffset = offsets.samplerOffset;
            instance.offsets.floatOffset = offsets.floatOffset;
            for (size_t i = 0; i < instance.instanceData.size(); ++i) {
                cullingOffsets.add(instance.cullingOffsets[i]);
                instanceData.add(instance.instanceData[i]);
                instanceMeshData.add(instance.instanceMeshData[i]);
                rayTracingScene.add(instance.rayTracingData[i]);
            }
        }
    }
    for (uint32 i = 0; i < transparentData.size(); ++i) {
        transparentData[i].offsets.instanceOffset = instanceData.size();
        cullingOffsets.add(transparentData[i].cullingOffset);
        instanceData.add(transparentData[i].instanceData);
        instanceMeshData.add(transparentData[i].meshData);
        rayTracingScene.add(transparentData[i].rayTracingScene);
    }
    cullingOffsetBuffer->rotateBuffer(cullingOffsets.size() * sizeof(uint32));
    cullingOffsetBuffer->updateContents(0, cullingOffsets.size() * sizeof(uint32), cullingOffsets.data());
    cullingOffsetBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                         Gfx::SE_ACCESS_MEMORY_READ_BIT, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    instanceBuffer->rotateBuffer(instanceData.size() * sizeof(InstanceData));
    instanceBuffer->updateContents(0, instanceData.size() * sizeof(InstanceData), instanceData.data());
    instanceBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_MEMORY_READ_BIT,
                                    Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    instanceMeshDataBuffer->rotateBuffer(sizeof(MeshData) * instanceMeshData.size());
    instanceMeshDataBuffer->updateContents(0, sizeof(MeshData) * instanceMeshData.size(), instanceMeshData.data());
    instanceMeshDataBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                            Gfx::SE_ACCESS_MEMORY_READ_BIT, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    instanceDataLayout->reset();
    descriptorSet = instanceDataLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(0, instanceBuffer);
    descriptorSet->updateBuffer(1, instanceMeshDataBuffer);
    descriptorSet->updateBuffer(2, meshletBuffer);
    descriptorSet->updateBuffer(3, primitiveIndicesBuffer);
    descriptorSet->updateBuffer(4, vertexIndicesBuffer);
    descriptorSet->updateBuffer(5, cullingOffsetBuffer);
    //Material::updateDescriptor();
}

void VertexData::loadMesh(MeshId id, Array<uint32> loadedIndices, Array<Meshlet> loadedMeshlets) {
    std::unique_lock l(vertexDataLock);
    uint32 meshletOffset = meshlets.size();
    AABB meshAABB;
    for (uint32 i = 0; i < loadedMeshlets.size(); ++i) {
        Meshlet& m = loadedMeshlets[i];
        meshAABB = meshAABB.combine(m.boundingBox);
        uint32 vertexOffset = vertexIndices.size();
        vertexIndices.resize(vertexOffset + m.numVertices);
        std::memcpy(vertexIndices.data() + vertexOffset, m.uniqueVertices, m.numVertices * sizeof(uint32));
        uint32 primitiveOffset = primitiveIndices.size();
        primitiveIndices.resize(primitiveOffset + (m.numPrimitives * 3));
        std::memcpy(primitiveIndices.data() + primitiveOffset, m.primitiveLayout, m.numPrimitives * 3 * sizeof(uint8));
        meshlets.add(MeshletDescription{
            .bounding = m.boundingBox, //.toSphere(),
            .vertexCount = m.numVertices,
            .primitiveCount = m.numPrimitives,
            .vertexOffset = vertexOffset,
            .primitiveOffset = primitiveOffset,
            .color = Vector((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX),
            .indicesOffset = (uint32)meshOffsets[id],
        });
    }
    meshData[id] = MeshData{
        .bounding = meshAABB, //.toSphere(),
        .numMeshlets = (uint32)loadedMeshlets.size(),
        .meshletOffset = meshletOffset,
        .firstIndex = (uint32)indices.size(),
        .numIndices = (uint32)loadedIndices.size(),
    };

    indices.resize(indices.size() + loadedIndices.size());
    std::memcpy(indices.data() + meshData[id].firstIndex, loadedIndices.data(), loadedIndices.size() * sizeof(uint32));
}

void VertexData::commitMeshes() {
    indexBuffer = graphics->createIndexBuffer(IndexBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * indices.size(),
                .data = (uint8*)indices.data(),
            },
        .indexType = Gfx::SE_INDEX_TYPE_UINT32,
        .name = "IndexBuffer",
    });
    meshletBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(MeshletDescription) * meshlets.size(),
                .data = (uint8*)meshlets.data(),
            },
        .numElements = meshlets.size(),
        .dynamic = false,
        .name = "MeshletBuffer",
    });
    vertexIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * vertexIndices.size(),
                .data = (uint8*)vertexIndices.data(),
            },
        .numElements = vertexIndices.size(),
        .dynamic = false,
        .name = "VertexIndicesBuffer",
    });

    primitiveIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint8) * primitiveIndices.size(),
                .data = (uint8*)primitiveIndices.data(),
            },
        .numElements = primitiveIndices.size(),
        .dynamic = false,
        .name = "PrimitiveIndicesBuffer",
    });
    updateBuffers();
    vertexIndices.clear();
    primitiveIndices.clear();
    indices.clear();
    meshlets.clear();
    dirty = false;
    // graphics->buildBottomLevelAccelerationStructures(std::move(dataToBuild));
}

MeshId VertexData::allocateVertexData(uint64 numVertices) {
    std::unique_lock l(vertexDataLock);
    MeshId res{idCounter++};
    meshOffsets.add(head);
    meshVertexCounts.add(numVertices);
    meshData.add({});
    head += numVertices;
    if (head > verticesAllocated) {
        verticesAllocated = 2 * head; // double capacity
        std::cout << "Resizing buffers to " << verticesAllocated << std::endl;
        resizeBuffers();
    }
    return res;
}

void VertexData::serializeMesh(MeshId id, uint64, ArchiveBuffer& buffer) {
    std::unique_lock l(vertexDataLock);
    Array<Meshlet> out;
    MeshData data = meshData[id];
    for (size_t i = 0; i < data.numMeshlets; ++i) {
        MeshletDescription& desc = meshlets[i + data.meshletOffset];
        Meshlet m;
        std::memcpy(m.uniqueVertices, &vertexIndices[desc.vertexOffset], desc.vertexCount * sizeof(uint32));
        std::memcpy(m.primitiveLayout, &primitiveIndices[desc.primitiveOffset], desc.primitiveCount * 3 * sizeof(uint8));
        m.numPrimitives = desc.primitiveCount;
        m.numVertices = desc.vertexCount;
        m.boundingBox = desc.bounding;
        out.add(std::move(m));
    }
    Array<uint32> ind(data.numIndices);
    std::memcpy(ind.data(), &indices[data.firstIndex], data.numIndices * sizeof(uint32));
    Serialization::save(buffer, out);
    Serialization::save(buffer, ind);
}

uint64 VertexData::deserializeMesh(MeshId id, ArchiveBuffer& buffer) {
    Array<Meshlet> in;
    Array<uint32> ind;
    Serialization::load(buffer, in);
    Serialization::load(buffer, ind);
    loadMesh(id, ind, in);
    uint64 result = in.size() * sizeof(MeshletDescription);
    result += ind.size() * sizeof(uint32);
    return result;
}

List<VertexData*> vertexDataList;

List<VertexData*> VertexData::getList() { return vertexDataList; }

VertexData* VertexData::findByTypeName(std::string name) {
    for (auto vd : vertexDataList) {
        if (vd->getTypeName() == name) {
            return vd;
        }
    }
    return nullptr;
}

void VertexData::init(Gfx::PGraphics _graphics) {
    graphics = _graphics;
    verticesAllocated = NUM_DEFAULT_ELEMENTS;
    instanceDataLayout = graphics->createDescriptorLayout("pScene");

    // instanceData
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    // meshData
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    // meshletData
    instanceDataLayout->addDescriptorBinding(
        Gfx::DescriptorBinding{.binding = 2, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    // primitiveIndices
    instanceDataLayout->addDescriptorBinding(
        Gfx::DescriptorBinding{.binding = 3, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    // vertexIndices
    instanceDataLayout->addDescriptorBinding(
        Gfx::DescriptorBinding{.binding = 4, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    // cullingOffset
    instanceDataLayout->addDescriptorBinding(
        Gfx::DescriptorBinding{.binding = 5, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    // cullingInfos
    instanceDataLayout->addDescriptorBinding(
        Gfx::DescriptorBinding{.binding = 6, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});

    instanceDataLayout->create();

    cullingOffsetBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .dynamic = true,
        .name = "MeshletOffset",
    });
    instanceBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .dynamic = true,
        .name = "InstanceBuffer",
    });
    instanceMeshDataBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .dynamic = true,
        .name = "MeshDataBuffer",
    });
    resizeBuffers();
    graphics->getShaderCompiler()->registerVertexData(this);
}

void VertexData::destroy() {
    cullingOffsetBuffer = nullptr;
    instanceBuffer = nullptr;
    instanceMeshDataBuffer = nullptr;
    instanceDataLayout = nullptr;
    meshletBuffer = nullptr;
    vertexIndicesBuffer = nullptr;
    primitiveIndicesBuffer = nullptr;
    indexBuffer = nullptr;
    meshData.clear();
    materialData.clear();
}

uint32 VertexData::addCullingMapping(MeshId id) {
    uint32 result = meshletCount;
    meshletCount += getMeshData(id).numMeshlets;
    return result;
}

VertexData::VertexData() : idCounter(0), head(0), verticesAllocated(0), dirty(false) {}
