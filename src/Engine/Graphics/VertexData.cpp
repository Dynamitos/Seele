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
    for (auto& mat : materialData)
    {
        for (auto& inst : mat.instances)
        {
            inst.instanceData.clear();
            inst.instanceMeshData.clear();
        }
        if (mat.material != nullptr)
        {
            mat.material->getDescriptorLayout()->reset();
        }
    }
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
    if (materialData.size() <= mat->getId())
    {
        materialData.resize(mat->getId() + 1);
    }
    MaterialData& matData = materialData[mat->getId()];
    matData.material = mat;
    if (matData.instances.size() <= referencedInstance->getId())
    {
        matData.instances.resize(referencedInstance->getId() + 1);
    }
    BatchedDrawCall& matInstanceData = matData.instances[referencedInstance->getId()];
    matInstanceData.materialInstance = referencedInstance;

    Matrix4 transformMatrix = transform.toMatrix() * mesh->transform;
    matInstanceData.instanceData.add(InstanceData{
        .transformMatrix = transformMatrix,
        .inverseTransformMatrix = glm::inverse(transformMatrix),
        });
    const auto& data = meshData[mesh->id];
    matInstanceData.instanceMeshData.add(data);
    referencedInstance->updateDescriptor();
    for (size_t i = 0; i < 0; ++i)
    {
        auto bounding = meshlets[data.meshletOffset + i].bounding;
        StaticArray<Vector, 8> corners;
        Vector min = bounding.min;//bounding.center - bounding.radius * Vector(1, 1, 1);
        Vector max = bounding.max;//bounding.center + bounding.radius * Vector(1, 1, 1);
        corners[0] = transformMatrix * Vector4(min.x, min.y, min.z, 1);
        corners[1] = transformMatrix * Vector4(min.x, min.y, max.z, 1);
        corners[2] = transformMatrix * Vector4(min.x, max.y, min.z, 1);
        corners[3] = transformMatrix * Vector4(min.x, max.y, max.z, 1);
        corners[4] = transformMatrix * Vector4(max.x, min.y, min.z, 1);
        corners[5] = transformMatrix * Vector4(max.x, min.y, max.z, 1);
        corners[6] = transformMatrix * Vector4(max.x, max.y, min.z, 1);
        corners[7] = transformMatrix * Vector4(max.x, max.y, max.z, 1);
        addDebugVertex(DebugVertex{ .position = corners[0], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[1], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[0], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[2], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[1], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[3], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[2], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[3], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[0], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[4], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[1], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[5], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[2], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[6], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[3], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[7], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[4], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[5], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[4], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[6], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[6], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[7], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[5], .color = meshlets[data.meshletOffset + i].color });
        addDebugVertex(DebugVertex{ .position = corners[7], .color = meshlets[data.meshletOffset + i].color });
    }

}

void VertexData::createDescriptors()
{
    std::unique_lock l(materialDataLock);

    instanceData.clear();
    instanceMeshData.clear();

    //uint32 numMeshlets = 0;
    //Array<uint32> cullingOffsets;
    for (auto& mat : materialData)
    {
        for (auto& instance : mat.instances)
        {
            instance.offsets.instanceOffset = instanceData.size();
            //instance.offsets.cullingCounterOffset = cullingOffsets.size();
            //instance.numMeshlets = 0;
            for (size_t i = 0; i < instance.instanceData.size(); ++i)
            {
                instanceData.add(instance.instanceData[i]);
                instanceMeshData.add(instance.instanceMeshData[i]);
                //instance.numMeshlets += instance.instanceMeshData[i].numMeshlets;
                //cullingOffsets.add(numMeshlets);
                //numMeshlets += instance.numMeshlets;
            }
        }
    }
    //cullingOffsetBuffer->rotateBuffer(cullingOffsets.size() * sizeof(uint32));
    //cullingOffsetBuffer->updateContents(ShaderBufferCreateInfo{
    //    .sourceData = {
    //        .size = cullingOffsets.size() * sizeof(uint32),
    //        .data = (uint8*)cullingOffsets.data(),
    //    },
    //    .numElements = cullingOffsets.size()
    //    });
    //cullingOffsetBuffer->pipelineBarrier(
    //    Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
    //    Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
    //    Gfx::SE_ACCESS_MEMORY_READ_BIT,
    //    Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT
    //);
    //cullingBuffer->rotateBuffer(numMeshlets * sizeof(uint32));
    //cullingBuffer->updateContents(ShaderBufferCreateInfo{
    //    .sourceData = {
    //        .size = numMeshlets * sizeof(uint32),
    //        },
    //    .numElements = numMeshlets
    //    });
    //cullingBuffer->pipelineBarrier(
    //    Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
    //    Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
    //    Gfx::SE_ACCESS_MEMORY_WRITE_BIT,
    //    Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT
    //);
    instanceBuffer->rotateBuffer(instanceData.size() * sizeof(InstanceData));
    instanceBuffer->updateContents(ShaderBufferCreateInfo{
        .sourceData = {
            .size = instanceData.size() * sizeof(InstanceData),
            .data = (uint8*)instanceData.data(),
        },
        .numElements = instanceData.size()
        });
    instanceBuffer->pipelineBarrier(
        Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
        Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
        Gfx::SE_ACCESS_MEMORY_READ_BIT,
        Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT
    );

    instanceMeshDataBuffer->rotateBuffer(sizeof(MeshData) * instanceMeshData.size());
    instanceMeshDataBuffer->updateContents(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(MeshData) * instanceMeshData.size(),
            .data = (uint8*)instanceMeshData.data(),
        },
        .numElements = instanceMeshData.size()
        });
    instanceMeshDataBuffer->pipelineBarrier(
        Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
        Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
        Gfx::SE_ACCESS_MEMORY_READ_BIT,
        Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT
    );
    instanceDataLayout->reset();
    descriptorSet = instanceDataLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(0, instanceBuffer);
    descriptorSet->updateBuffer(1, instanceMeshDataBuffer);
    descriptorSet->updateBuffer(2, meshletBuffer);
    descriptorSet->updateBuffer(3, primitiveIndicesBuffer);
    descriptorSet->updateBuffer(4, vertexIndicesBuffer);
    //descriptorSet->updateBuffer(5, cullingOffsetBuffer);
    //descriptorSet->updateBuffer(6, cullingBuffer);

    descriptorSet->writeChanges();
}

void VertexData::loadMesh(MeshId id, Array<uint32> loadedIndices, Array<Meshlet> loadedMeshlets)
{
    assert(loadedMeshlets.size() < 2048);
    std::unique_lock l(vertexDataLock);
    meshlets.reserve(meshlets.size() + loadedMeshlets.size());
    vertexIndices.reserve(vertexIndices.size() + loadedMeshlets.size() * Gfx::numVerticesPerMeshlet);
    primitiveIndices.reserve(primitiveIndices.size() + loadedMeshlets.size() * Gfx::numPrimitivesPerMeshlet * 3);
    uint32 meshletOffset = meshlets.size();
    AABB meshAABB;
    for (uint32 i = 0; i < loadedMeshlets.size(); ++i)
    {
        Meshlet& m = loadedMeshlets[i];
        meshAABB = meshAABB.combine(m.boundingBox);
        uint32 vertexOffset = vertexIndices.size();
        vertexIndices.resize(vertexOffset + m.numVertices);
        std::memcpy(vertexIndices.data() + vertexOffset, m.uniqueVertices, m.numVertices * sizeof(uint32));
        uint32 primitiveOffset = primitiveIndices.size();
        primitiveIndices.resize(primitiveOffset + (m.numPrimitives * 3));
        std::memcpy(primitiveIndices.data() + primitiveOffset, m.primitiveLayout, m.numPrimitives * 3 * sizeof(uint8));
        meshlets.add(MeshletDescription{
            .bounding = m.boundingBox,//.toSphere(),
            .vertexCount = m.numVertices,
            .primitiveCount = m.numPrimitives,
            .vertexOffset = vertexOffset,
            .primitiveOffset = primitiveOffset,
            .color = Vector((float)rand() / RAND_MAX,(float)rand() / RAND_MAX,(float)rand() / RAND_MAX),
            .indicesOffset = (uint32)meshOffsets[id],
            });
    }
    meshData[id] = MeshData{
        .bounding = meshAABB,//.toSphere(),
        .numMeshlets = (uint32)loadedMeshlets.size(),
        .meshletOffset = meshletOffset,
        .firstIndex = (uint32)indices.size(),
        .numIndices = (uint32)loadedIndices.size(),
    };

    if (!graphics->supportMeshShading())
    {
        indices.resize(indices.size() + loadedIndices.size());
        std::memcpy(indices.data() + meshData[id].firstIndex, loadedIndices.data(), loadedIndices.size() * sizeof(uint32));
        indexBuffer = graphics->createIndexBuffer(IndexBufferCreateInfo{
            .sourceData = {
                .size = sizeof(uint32) * indices.size(),
                .data = (uint8*)indices.data(),
            },
            .indexType = Gfx::SE_INDEX_TYPE_UINT32,
            .name = "IndexBuffer",
            });

    }
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
            .size = sizeof(uint8) * primitiveIndices.size(),
            .data = (uint8*)primitiveIndices.data(),
        },
        .numElements = primitiveIndices.size(),
        .dynamic = false,
        .name = "PrimitiveIndicesBuffer",
        });
}

MeshId VertexData::allocateVertexData(uint64 numVertices)
{
    std::unique_lock l(vertexDataLock);
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

VertexData* VertexData::findByTypeName(std::string name)
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

void VertexData::init(Gfx::PGraphics _graphics)
{
    graphics = _graphics;
    verticesAllocated = NUM_DEFAULT_ELEMENTS;
    instanceDataLayout = graphics->createDescriptorLayout("pScene");

    // instanceData
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 0, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, });
    // meshData
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 1, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, });
    // meshletData
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 2, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER });
    // primitiveIndices
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 3, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER });
    // vertexIndices
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 4, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER });
    // cullingList
    //instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 5, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER });
    // cullingOffset
    //instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 6, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER });

    //cullingOffsetBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{ 
    //    .dynamic = true,
    //    .name = "MeshletOffset",
    //    });
    //cullingBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{ 
    //    .dynamic = true,
    //    .name = "MeshletCulling",
    //    });
    instanceBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{ 
        .dynamic = true,
        .name = "InstanceBuffer",
        });
    instanceMeshDataBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{ 
        .dynamic = true,
        .name = "MeshDataBuffer",
        });
    instanceDataLayout->create();
    resizeBuffers();
    graphics->getShaderCompiler()->registerVertexData(this);
}

void VertexData::destroy()
{
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

VertexData::VertexData()
    : idCounter(0)
    , head(0)
    , verticesAllocated(0)
    , dirty(false)
{
}
