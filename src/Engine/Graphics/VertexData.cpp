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
            .data = data,
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
        //AABB meshAABB;
        for (uint32 i = 0; i < numMeshlets; ++i)
        {
            Meshlet& m = loadedMeshlets[currentMesh + i];
            //meshAABB = meshAABB.combine(m.boundingBox);
            uint32 vertexOffset = vertexIndices.size();
            vertexIndices.resize(vertexOffset + m.numVertices);
            std::memcpy(vertexIndices.data() + vertexOffset, m.uniqueVertices, m.numVertices * sizeof(uint32));
            uint32 primitiveOffset = primitiveIndices.size();
            primitiveIndices.resize(primitiveOffset + (m.numPrimitives * 3));
            std::memcpy(primitiveIndices.data() + primitiveOffset, m.primitiveLayout, m.numPrimitives * 3 * sizeof(uint8));
            meshlets.add(MeshletDescription{
                .boundingBox = m.boundingBox,
                .vertexCount = m.numVertices,
                .primitiveCount = m.numPrimitives,
                .vertexOffset = vertexOffset,
                .primitiveOffset = primitiveOffset,
                });
        }
        meshData[id].add(MeshData{
            .numMeshlets = numMeshlets,
            .meshletOffset = meshletOffset,
            .indicesOffset = (uint32)meshOffsets[id],
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
        verticesAllocated = std::max(head, verticesAllocated + NUM_DEFAULT_ELEMENTS);
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

void Seele::VertexData::init(Gfx::PGraphics _graphics)
{
    graphics = _graphics;
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

void VertexData::destroy()
{
    instanceDataLayout = nullptr;
    meshletBuffer = nullptr;
    vertexIndicesBuffer = nullptr;
    primitiveIndicesBuffer = nullptr;
    indexBuffer = nullptr;
    meshData.clear();
    materialData.clear();
}

VertexData::VertexData()
    : idCounter(0)
    , head(0)
    , verticesAllocated(0)
    , dirty(false)
{
}

void Meshlet::build(const Array<uint32>& indices, Array<Meshlet>& meshlets)
{
    Meshlet current = {
        .numVertices = 0,
        .numPrimitives = 0,
    };
    auto findIndex = [&current](uint32 index) -> int {
        for (uint32 i = 0; i < current.numVertices; ++i)
        {
            if (current.uniqueVertices[i] == index)
            {
                return i;
            }
        }
        if (current.numVertices == Gfx::numVerticesPerMeshlet)
        {
            return -1;
        }
        current.uniqueVertices[current.numVertices] = index;
        return current.numVertices++;
        };
    auto completeMeshlet = [&meshlets, &current]() {
        meshlets.add(current);
        current = {
            .numVertices = 0,
            .numPrimitives = 0,
        };
        };
    for (size_t faceIndex = 0; faceIndex < indices.size() / 3; ++faceIndex)
    {
        int f1 = findIndex(indices[faceIndex * 3 + 0]);
        int f2 = findIndex(indices[faceIndex * 3 + 1]);
        int f3 = findIndex(indices[faceIndex * 3 + 2]);

        if (f1 == -1 || f2 == -1 || f1 == -1)
        {
            completeMeshlet();
            f1 = findIndex(indices[faceIndex * 3 + 0]);
            f2 = findIndex(indices[faceIndex * 3 + 1]);
            f3 = findIndex(indices[faceIndex * 3 + 2]);
        }
        current.primitiveLayout[current.numPrimitives * 3 + 0] = uint8(f1);
        current.primitiveLayout[current.numPrimitives * 3 + 1] = uint8(f2);
        current.primitiveLayout[current.numPrimitives * 3 + 2] = uint8(f3);
        current.numPrimitives++;
        if (current.numPrimitives == Gfx::numPrimitivesPerMeshlet)
        {
            completeMeshlet();
        }
    }
    if (current.numVertices > 0)
    {
        completeMeshlet();
    }
}

void Meshlet::calcBoundingBox(const Array<Vector>& positions)
{
    for (uint32 i = 0; i < numVertices; ++i)
    {
        boundingBox.adjust(positions[uniqueVertices[i]]);
    }
}