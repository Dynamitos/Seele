#include "VertexData.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"
#include "Graphics/Descriptor.h"
#include "Component/Mesh.h"
#include "Graphics/Shader.h"
#include "Graphics/Mesh.h"
#include "Containers/Set.h"

using namespace Seele;

constexpr static uint64 NUM_DEFAULT_ELEMENTS = 1024 * 1024;

void VertexData::resetMeshData()
{
    std::unique_lock l(materialDataLock);
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
    std::unique_lock l(materialDataLock);
    PMaterialInstance referencedInstance = mesh->referencedMaterial->getHandle();
    PMaterial mat = referencedInstance->getBaseMaterial();
    MaterialData& matData = materialData[mat->getName()];
    matData.material = mat;
    MaterialInstanceData& matInstanceData = matData.instances[referencedInstance->getId()];
    for (const auto& data : meshData[mesh->id])
    {
        matInstanceData.meshes.add(MeshInstanceData{
            .instance = InstanceData {
                .transformMatrix = transform.toMatrix(),
            },
            .data = data,
            });
        //for (size_t i = 0; i < data.numMeshlets; ++i)
        //{
        //    auto bounding = meshlets[data.meshletOffset + i].bounding;
        //    StaticArray<Vector, 8> corners;
        //    corners[0] = transform.toMatrix() * Vector4(bounding.min.x, bounding.min.y, bounding.min.z, 1);
        //    corners[1] = transform.toMatrix() * Vector4(bounding.min.x, bounding.min.y, bounding.max.z, 1);
        //    corners[2] = transform.toMatrix() * Vector4(bounding.min.x, bounding.max.y, bounding.min.z, 1);
        //    corners[3] = transform.toMatrix() * Vector4(bounding.min.x, bounding.max.y, bounding.max.z, 1);
        //    corners[4] = transform.toMatrix() * Vector4(bounding.max.x, bounding.min.y, bounding.min.z, 1);
        //    corners[5] = transform.toMatrix() * Vector4(bounding.max.x, bounding.min.y, bounding.max.z, 1);
        //    corners[6] = transform.toMatrix() * Vector4(bounding.max.x, bounding.max.y, bounding.min.z, 1);
        //    corners[7] = transform.toMatrix() * Vector4(bounding.max.x, bounding.max.y, bounding.max.z, 1);
        //    addDebugVertex(DebugVertex{ .position = corners[0], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[1], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[0], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[2], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[1], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[3], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[2], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[3], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[0], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[4], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[1], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[5], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[2], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[6], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[3], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[7], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[4], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[5], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[4], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[6], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[6], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[7], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[5], .color = meshlets[data.meshletOffset + i].color });
        //    addDebugVertex(DebugVertex{ .position = corners[7], .color = meshlets[data.meshletOffset + i].color });
        //}
    }
    matInstanceData.materialInstance = referencedInstance;
    referencedInstance->updateDescriptor();
}

void VertexData::createDescriptors()
{
    std::unique_lock l(materialDataLock);
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
                .name = "InstanceBuffer"
                });
            matInst.instanceBuffer->pipelineBarrier(
                Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                Gfx::SE_ACCESS_MEMORY_READ_BIT,
                Gfx::SE_PIPELINE_STAGE_VERTEX_SHADER_BIT
            );
            matInst.descriptorSet = instanceDataLayout->allocateDescriptorSet();
            
            matInst.meshDataBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
                .sourceData = {
                    .size = sizeof(MeshData) * meshes.size(),
                    .data = (uint8*)meshes.data(),
                },
                .numElements = meshes.size(),
                .dynamic = false,
                .name = "MeshDataBuffer"
            });
            matInst.meshDataBuffer->pipelineBarrier(
                Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                Gfx::SE_ACCESS_MEMORY_READ_BIT,
                Gfx::SE_PIPELINE_STAGE_VERTEX_SHADER_BIT
            );
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
        AABB meshAABB;
        for (uint32 i = 0; i < numMeshlets; ++i)
        {
            Meshlet& m = loadedMeshlets[currentMesh + i];
            meshAABB = meshAABB.combine(m.boundingBox);
            uint32 vertexOffset = vertexIndices.size();
            vertexIndices.resize(vertexOffset + m.numVertices);
            std::memcpy(vertexIndices.data() + vertexOffset, m.uniqueVertices, m.numVertices * sizeof(uint32));
            uint32 primitiveOffset = primitiveIndices.size();
            primitiveIndices.resize(primitiveOffset + (m.numPrimitives * 3));
            for(size_t x = 0; x < m.numPrimitives*3; ++x)
            {
                primitiveIndices[primitiveOffset + x] = m.primitiveLayout[x];
            }
            //std::memcpy(primitiveIndices.data() + primitiveOffset, m.primitiveLayout, m.numPrimitives * 3 * sizeof(uint8));
            meshlets.add(MeshletDescription{
                .bounding = m.boundingBox.toSphere(),
                .vertexCount = m.numVertices,
                .primitiveCount = m.numPrimitives,
                .vertexOffset = vertexOffset,
                .primitiveOffset = primitiveOffset,
                .color = Vector((float)rand() / RAND_MAX,(float)rand() / RAND_MAX,(float)rand() / RAND_MAX),
                });
        }
        meshData[id].add(MeshData{
            .bounding = meshAABB.toSphere(),
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
        .name = "IndexBuffer",
    });
    meshletBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(MeshletDescription) * meshlets.size(),
            .data = (uint8*)meshlets.data()
        },
        .numElements = meshlets.size(),
        .dynamic = false,
        .name = "MeshletBuffer"
    });
    vertexIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(uint32) * vertexIndices.size(),
            .data = (uint8*)vertexIndices.data(),
        },
        .numElements = vertexIndices.size(),
        .dynamic = false,
        .name = "VertexIndicesBuffer"
    });
    primitiveIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(uint32) * primitiveIndices.size(),
            .data = (uint8*)primitiveIndices.data(),
        },
        .numElements = primitiveIndices.size(),
        .dynamic = false,
        .name = "PrimitiveIndicesBuffer",
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
    instanceDataLayout = graphics->createDescriptorLayout("pScene");
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding =0, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,});

    // meshData
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 1, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,});
    // meshletData
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding =2, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    // primitiveIndices
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding =3, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    // vetexIndices
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding =4, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    
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
