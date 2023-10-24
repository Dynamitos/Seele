#include "VertexData.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"

using namespace Seele;

void VertexData::resetMeshData()
{
    materialData.clear();
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
            Gfx::PShaderBuffer meshDataBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
                .resourceData = {
                    .size = sizeof(MeshData) * meshes.size(),
                    .data = (uint8*)meshes.data(),
                },
                .stride = sizeof(MeshData)
            });
            matInst.descriptorSet = instanceDataLayout->allocateDescriptorSet();
            matInst.descriptorSet->updateBuffer(0, instanceBuffer);
            if (Gfx::useMeshShading)
            {
                matInst.descriptorSet->updateBuffer(1, meshDataBuffer);
                matInst.descriptorSet->updateBuffer(2, meshletBuffer);
            }
            matInst.descriptorSet->writeChanges();
            matInst.numMeshes = meshes.size();
        }
    }
}

List<VertexData*> vertexDataList;

List<VertexData*> VertexData::getList()
{
    return vertexDataList;
}

VertexData::VertexData(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    instanceDataLayout = graphics->createDescriptorLayout("VertexDataInstanceLayout");
    instanceDataLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    if (Gfx::useMeshShading)
    {
        // meshData
        instanceDataLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        // meshletData
        instanceDataLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        meshletBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
            .resourceData = {
                .size = sizeof(MeshletData),
                .data = nullptr,
            },
            .bDynamic = true,
        });
    }
    instanceDataLayout->create();
}