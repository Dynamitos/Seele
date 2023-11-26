#include "VertexData.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"
#include "Graphics/Descriptor.h"
#include "Component/Mesh.h"
#include "Graphics/Shader.h"
#include <set>

using namespace Seele;

constexpr static uint64 NUM_DEFAULT_ELEMENTS = 16 * 1024;

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

void VertexData::updateMesh(PMesh mesh, Component::Transform& transform)
{
    PMaterial mat = mesh->referencedMaterial->getHandle()->getBaseMaterial();
    MaterialData& matData = materialData[mat->getName()];
    matData.material = mat;
    MaterialInstanceData& matInstanceData = matData.instances[mesh->referencedMaterial->getHandle()->getId()];
    for (auto& data : meshData[mesh->id])
    {
        matInstanceData.meshes.add(MeshInstanceData{
            .instance = InstanceData {
                .transformMatrix = transform.toMatrix(),
            },
            .data = data
            });
    }
    matInstanceData.materialInstance = mesh->referencedMaterial->getHandle();
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
                meshes.add(inst.data);
                instanceData.add(inst.instance);
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
            
            matInst.meshDataBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
                .sourceData = {
                    .size = sizeof(MeshData) * meshes.size(),
                    .data = (uint8*)meshes.data(),
                },
                .numElements = meshes.size(),
                .dynamic = false,
            });
            matInst.descriptorSet->updateBuffer(0, matInst.instanceBuffer);
            matInst.descriptorSet->updateBuffer(1, matInst.meshDataBuffer);
            matInst.descriptorSet->updateBuffer(2, meshletBuffer);
            matInst.descriptorSet->updateBuffer(3, primitiveIndicesBuffer);
            matInst.descriptorSet->updateBuffer(4, vertexIndicesBuffer);
            
            matInst.descriptorSet->writeChanges();
        }
    }
}

void VertexData::loadMesh(MeshId id, Array<uint32> loadedIndices, Array<Meshlet> loadedMeshlets)
{
    meshlets.reserve(meshlets.size() + loadedMeshlets.size());
    vertexIndices.reserve(vertexIndices.size() + loadedMeshlets.size() * Gfx::numVerticesPerMeshlet);
    primitiveIndices.reserve(primitiveIndices.size() + loadedMeshlets.size() * Gfx::numPrimitivesPerMeshlet * 3);
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
    meshData[id][0].firstIndex = indices.size();
    meshData[id][0].numIndices = loadedIndices.size();
    indices.resize(indices.size() + loadedIndices.size());
    std::memcpy(indices.data() + meshData[id][0].firstIndex, loadedIndices.data(), loadedIndices.size() * sizeof(uint32));
    indexBuffer = graphics->createIndexBuffer(IndexBufferCreateInfo{
        .sourceData = {
            .size = sizeof(uint32) * indices.size(),
            .data = (uint8*)indices.data(),
        },
        .indexType = Gfx::SE_INDEX_TYPE_UINT32,
    });
    meshletBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(MeshletDescription) * meshlets.size(),
            .data = (uint8*)meshlets.data()
        },
        .numElements = meshlets.size(),
        .dynamic = false,
    });
    vertexIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(uint32) * vertexIndices.size(),
            .data = (uint8*)vertexIndices.data(),
        },
        .numElements = vertexIndices.size(),
        .dynamic = false,
    });
    primitiveIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(uint8) * primitiveIndices.size(),
            .data = (uint8*)primitiveIndices.data(),
        },
        .numElements = primitiveIndices.size(),
        .dynamic = false,
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

    // meshData
    instanceDataLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    // meshletData
    instanceDataLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    // primitiveIndices
    instanceDataLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    // vetexIndices
    instanceDataLayout->addDescriptorBinding(4, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    
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

void Seele::Meshlet::buildFromIndexBuffer(const Array<uint32>& indices, Array<Meshlet>& meshlets)
{
    std::set<uint32> uniqueVertices;
    Meshlet current = {
        .numVertices = 0,
        .numPrimitives = 0,
    };
    auto insertAndGetIndex = [&uniqueVertices, &current](uint32 index) -> int8_t
        {
            auto [it, inserted] = uniqueVertices.insert(index);
            if (inserted)
            {
                if (current.numVertices == Gfx::numVerticesPerMeshlet)
                {
                    return -1;
                }
                current.uniqueVertices[current.numVertices] = index;
                return current.numVertices++;
            }
            else
            {
                for (uint32 i = 0; i < current.numVertices; ++i)
                {
                    if (current.uniqueVertices[i] == index)
                    {
                        return i;
                    }
                }
                // it could be in unique vertices but not in meshlet vertices
                return -1;
            }
        };
    auto completeMeshlet = [&meshlets, &current, &uniqueVertices]() {
        meshlets.add(current);
        current = {
            .numVertices = 0,
            .numPrimitives = 0,
        };
        uniqueVertices.clear();
        };
    for (size_t faceIndex = 0; faceIndex < indices.size() / 3; ++faceIndex)
    {
        auto i1 = insertAndGetIndex(indices[faceIndex * 3 + 0]);
        auto i2 = insertAndGetIndex(indices[faceIndex * 3 + 1]);
        auto i3 = insertAndGetIndex(indices[faceIndex * 3 + 2]);
        if (i1 == -1 || i2 == -1 || i3 == -1)
        {
            completeMeshlet();
            i1 = insertAndGetIndex(indices[faceIndex * 3 + 0]);
            i2 = insertAndGetIndex(indices[faceIndex * 3 + 1]);
            i3 = insertAndGetIndex(indices[faceIndex * 3 + 2]);
        }
        current.primitiveLayout[current.numPrimitives * 3 + 0] = i1;
        current.primitiveLayout[current.numPrimitives * 3 + 1] = i2;
        current.primitiveLayout[current.numPrimitives * 3 + 2] = i3;
        current.numPrimitives++;
        if (current.numPrimitives == Gfx::numPrimitivesPerMeshlet)
        {
            completeMeshlet();
        }
    }
    if (!uniqueVertices.empty())
    {
        completeMeshlet();
    }
}
