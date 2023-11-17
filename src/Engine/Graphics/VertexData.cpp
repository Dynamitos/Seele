#include "VertexData.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"
#include "Graphics/Descriptor.h"
#include "Component/Mesh.h"
#include "Graphics/Shader.h"

using namespace Seele;

constexpr static uint64 NUM_DEFAULT_ELEMENTS = 1024 * 1024;

void VertexData::resetMeshData()
{
    for (auto& [_, mat] : materialData)
    {
        mat.material->getDescriptorLayout()->reset();
    }
    materialData.clear();
    if (dirty)
    {
        updateBuffers();
        dirty = false;
    }
}

void VertexData::updateMesh(const Component::Transform& transform, PMesh mesh)
{
    PMaterial mat = mesh->referencedMaterial->getHandle()->getBaseMaterial();
    MaterialData& matData = materialData[mat->getName()];
    matData.material = mat;
    MaterialInstanceData& matInstanceData = matData.instances[mesh->referencedMaterial->getHandle()->getId()];
    matInstanceData.meshes.add(MeshInstanceData{
        .id = mesh->id,
        .instance = InstanceData {
            .transformMatrix = transform.toMatrix(),
        },
        .indexBuffer = mesh->indexBuffer,
    });
    matInstanceData.materialInstance = mesh->referencedMaterial->getHandle();
    matInstanceData.materialInstance->updateDescriptor();
    matInstanceData.numMeshes += meshData[mesh->id].size();
}

void VertexData::createDescriptors()
{
    instanceDataLayout->reset();
    for (const auto& [_, mat] : materialData)
    {
        for (auto& [_, matInst] : mat.instances)
        {
            Array<InstanceData> instanceData;
            Array<MeshData> meshes;
            for (auto& inst : matInst.meshes)
            {
                inst.meshes = 0;
                for (const auto& mesh : meshData[inst.id])
                {
                    instanceData.add(inst.instance);
                    meshes.add(mesh);
                    inst.meshes++;
                }
            }
            matInst.instanceBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
                .sourceData = {
                    .size = sizeof(InstanceData) * instanceData.size(),
                    .data = (uint8*)instanceData.data(),
                },
                .numElements = instanceData.size(),
                .dynamic = false,
            });
            matInst.descriptorSet = instanceDataLayout->allocateDescriptorSet();
            matInst.descriptorSet->updateBuffer(0, matInst.instanceBuffer);
            if (graphics->supportMeshShading())
            {
                matInst.meshDataBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
                    .sourceData = {
                        .size = sizeof(MeshData) * meshes.size(),
                        .data = (uint8*)meshes.data(),
                    },
                    .numElements = meshes.size(),
                    .dynamic = false,
                });
                matInst.descriptorSet->updateBuffer(1, matInst.meshDataBuffer);
                matInst.descriptorSet->updateBuffer(2, meshletBuffer);
                matInst.descriptorSet->updateBuffer(3, primitiveIndicesBuffer);
                matInst.descriptorSet->updateBuffer(4, vertexIndicesBuffer);
            }
            matInst.descriptorSet->writeChanges();
            matInst.numMeshes = meshes.size();
        }
    }
}

void VertexData::loadMesh(MeshId id, Array<Meshlet> loadedMeshlets)
{
    meshData[id].clear();
    uint32 currentMesh = 0;
    while (currentMesh < loadedMeshlets.size())
    {
        uint32 numMeshlets = std::min<uint32>(512, loadedMeshlets.size() - currentMesh);
        uint32 meshletOffset = meshlets.size();
        for (uint32 i = 0; i < numMeshlets; ++i)
        {
            Meshlet& m = loadedMeshlets[currentMesh + i];
            uint32 vertexOffset = vertexIndices.size();
            vertexIndices.resize(vertexOffset + m.numVertices);
            std::memcpy(vertexIndices.data() + vertexOffset, m.uniqueVertices.data(), m.numVertices * sizeof(uint32));
            uint32 primitiveOffset = primitiveIndices.size();
            primitiveIndices.resize(primitiveOffset + (m.numPrimitives * 3));
            std::memcpy(primitiveIndices.data() + primitiveOffset, m.primitiveLayout.data(), m.numPrimitives * 3 * sizeof(uint8));
            meshlets.add(MeshletDescription{
                .boundingBox = MeshletAABB(),
                .vertexCount = m.numVertices,
                .primitiveCount = m.numPrimitives,
                .vertexOffset = vertexOffset,
                .primitiveOffset = primitiveOffset,
                });
        }
        meshData[id].add(MeshData{
            .numMeshlets = numMeshlets,
            .meshletOffset = meshletOffset,
            .indicesOffset = static_cast<uint32>(meshOffsets[id]),
            });
        currentMesh += numMeshlets;
    }
    meshletBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(MeshletDescription) * meshlets.size(),
            .data = (uint8*)meshlets.data()
        },
        .numElements = meshlets.size(),
        .dynamic = true,
    });
    vertexIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(uint32) * vertexIndices.size(),
            .data = (uint8*)vertexIndices.data(),
        },
        .numElements = vertexIndices.size(),
        .dynamic = true,
    });
    primitiveIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(uint8) * primitiveIndices.size(),
            .data = (uint8*)primitiveIndices.data(),
        },
        .numElements = primitiveIndices.size(),
        .dynamic = true,
    });
}

MeshId VertexData::allocateVertexData(uint64 numVertices)
{
    MeshId res{ idCounter++ };
    meshOffsets[res] = head;
    meshVertexCounts[res] = numVertices;
    head += numVertices;
    if (head > verticesAllocated)
    {
        verticesAllocated = head;
        resizeBuffers();
    }
    return res;
}

uint64 VertexData::getMeshOffset(MeshId id)
{
    return meshOffsets[id];
}

uint64 VertexData::getMeshVertexCount(MeshId id)
{
    return meshVertexCounts[id];
}

List<VertexData*> vertexDataList;

List<VertexData*> VertexData::getList()
{
    return vertexDataList;
}

VertexData* Seele::VertexData::findByTypeName(std::string name)
{
    for (auto vd : vertexDataList)
    {
        if (vd->getTypeName() == name)
        {
            return vd;
        }
    }
    return nullptr;
}

void Seele::VertexData::init(Gfx::PGraphics graphics)
{
    this->graphics = graphics;
    verticesAllocated = NUM_DEFAULT_ELEMENTS;
    instanceDataLayout = graphics->createDescriptorLayout("VertexDataInstanceLayout");
    instanceDataLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    if (graphics->supportMeshShading())
    {
        // meshData
        instanceDataLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        // meshletData
        instanceDataLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        // primitiveIndices
        instanceDataLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        // vetexIndices
        instanceDataLayout->addDescriptorBinding(4, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }
    instanceDataLayout->create();
    resizeBuffers();
    graphics->getShaderCompiler()->registerVertexData(this);
}

VertexData::VertexData()
    : idCounter(0)
    , head(0)
    , verticesAllocated(0)
    , dirty(false)
{
}

void Meshlet::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, uniqueVertices);
    Serialization::save(buffer, primitiveLayout);
    Serialization::save(buffer, numVertices);
    Serialization::save(buffer, numPrimitives);
}

void Meshlet::load(ArchiveBuffer& buffer)
{
    Serialization::load(buffer, uniqueVertices);
    Serialization::load(buffer, primitiveLayout);
    Serialization::load(buffer, numVertices);
    Serialization::load(buffer, numPrimitives);
}
