#include "VertexData.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"
#include "Graphics/Descriptor.h"
#include "Component/Mesh.h"

using namespace Seele;

constexpr static uint64 NUM_DEFAULT_ELEMENTS = 1024;

void VertexData::resetMeshData()
{
    materialData.clear();
    if (dirty)
    {
        updateBuffers();
    }
}

void VertexData::updateMesh(const Component::Transform& transform, const Component::Mesh& mesh)
{
    PMaterial mat = mesh.instance->getBaseMaterial();
    MaterialData& matData = materialData[mat->getName()];
    MaterialInstanceData& matInstanceData = matData.instances[mesh.instance->getId()];
    matInstanceData.meshes.add(MeshInstanceData{
        .id = mesh.id,
        .instance = InstanceData {
            .transformMatrix = transform.toMatrix(),
        }
    });
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
            std::memcpy(vertexIndices.data() + vertexOffset, m.uniqueVertices, sizeof(m.uniqueVertices));
            uint32 primitiveOffset = primitiveIndices.size();
            primitiveIndices.resize(primitiveOffset + (m.numPrimitives * 3));
            std::memcpy(primitiveIndices.data() + primitiveOffset, m.primitiveLayout, sizeof(m.primitiveLayout));
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
    meshletBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo {
        .resourceData = {
            .size = sizeof(MeshletDescription) * meshlets.size(),
            .data = (uint8*)meshlets.data()
        },
        .stride = sizeof(MeshletDescription),
        .bDynamic = true,
    });
    vertexIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo {
        .resourceData = {
            .size = sizeof(uint32) * vertexIndices.size(),
            .data = (uint8*)vertexIndices.data(),
        },
        .stride = sizeof(uint32),
        .bDynamic = true,
    });
    primitiveIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo {
        .resourceData = {
            .size = sizeof(uint8) * primitiveIndices.size(),
            .data = (uint8*)primitiveIndices.data(),
        },
        .stride = sizeof(uint8),
        .bDynamic = true,
    });

}

void VertexData::createDescriptors()
{
    for (const auto& [_, mat] : materialData)
    {
        for (auto& [_, matInst] : mat.instances)
        {
            Array<InstanceData> instanceData;
            Array<MeshData> meshes;
            for (const auto& inst : matInst.meshes)
            {
                for (const auto& mesh : meshData[inst.id])
                {
                    instanceData.add(inst.instance);
                    meshes.add(mesh);
                }
            }
            Gfx::PShaderBuffer instanceBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
                .resourceData = {
                    .size = sizeof(InstanceData) * instanceData.size(),
                    .data = (uint8*)instanceData.data(),
                },
                .stride = sizeof(InstanceData)
            });
            matInst.descriptorSet = instanceDataLayout->allocateDescriptorSet();
            matInst.descriptorSet->updateBuffer(0, instanceBuffer);
            if (Gfx::useMeshShading)
            {
                Gfx::PShaderBuffer meshDataBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
                    .resourceData = {
                        .size = sizeof(MeshData) * meshes.size(),
                        .data = (uint8*)meshes.data(),
                    },
                    .stride = sizeof(MeshData)
                });
                matInst.descriptorSet->updateBuffer(1, meshDataBuffer);
                matInst.descriptorSet->updateBuffer(2, meshletBuffer);
                matInst.descriptorSet->updateBuffer(3, primitiveIndicesBuffer);
                matInst.descriptorSet->updateBuffer(4, vertexIndicesBuffer);
            }
            matInst.descriptorSet->writeChanges();
            matInst.numMeshes = meshes.size();
        }
    }
}

MeshId VertexData::allocateVertexData(uint64 numVertices)
{
    MeshId res{ idCounter++ };
    meshOffsets[res] = head;
    head += numVertices;
    if (head > verticesAllocated)
    {
        verticesAllocated = head;
        resizeBuffers();
    }
    return res;
}

List<VertexData*> vertexDataList;

List<VertexData*> VertexData::getList()
{
    return vertexDataList;
}

void Seele::VertexData::init(Gfx::PGraphics graphics)
{
    this->graphics = graphics;
    verticesAllocated = NUM_DEFAULT_ELEMENTS;
    instanceDataLayout = graphics->createDescriptorLayout("VertexDataInstanceLayout");
    instanceDataLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    if (Gfx::useMeshShading)
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
}

VertexData::VertexData()
    : idCounter(0)
    , dirty(false)
{
}
