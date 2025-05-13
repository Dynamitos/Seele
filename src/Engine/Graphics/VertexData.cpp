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
#include <meshoptimizer.h>
#include <metis.h>
#include <unordered_map>

using namespace Seele;

constexpr static uint64 NUM_DEFAULT_ELEMENTS = 36;
uint64 VertexData::meshletCount = 0;

void VertexData::resetMeshData() {
    std::unique_lock l(materialDataLock);
    instanceData.clear(true);
    instanceMeshData.clear(true);
    rayTracingScene.clear(true);
    transparentData.clear(true);
    for (auto& mat : materialData) {
        for (auto& inst : mat.instances) {
            inst.instanceData.clear(true);
            inst.instanceMeshData.clear(true);
            inst.cullingOffsets.clear(true);
            inst.rayTracingData.clear(true);
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

void VertexData::updateMesh(uint32 meshletOffset, PMesh mesh, Component::Transform& transform) {
    std::unique_lock l(materialDataLock);
    PMaterialInstance referencedInstance = mesh->referencedMaterial->getHandle();
    PMaterial mat = referencedInstance->getBaseMaterial();
    Matrix4 transformMatrix = transform.toMatrix() * mesh->transform;
    InstanceData inst = InstanceData{
        .transformMatrix = transformMatrix,
        .inverseTransformMatrix = glm::inverse(transformMatrix),
    };

    referencedInstance->updateDescriptor();
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
    const auto& data = registeredMeshes[mesh->id].meshData;
    uint32 numMeshlets = data.meshletRange.size;
    for (uint32 i = 0; i < (numMeshlets + Gfx::numMeshletsPerTask - 1) / Gfx::numMeshletsPerTask; ++i) {
        MeshData chunkMeshData = data;
        chunkMeshData.meshletRange = {
            .offset = data.meshletRange.offset + i * Gfx::numMeshletsPerTask,
            .size = std::min(numMeshlets - i * Gfx::numMeshletsPerTask, Gfx::numMeshletsPerTask),
        };
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
                .meshData = chunkMeshData,
                .cullingOffset = meshletOffset,
                .rayTracingScene = mesh->blas,
            });
        } else { // opaque
            matInstanceData.rayTracingData.add(mesh->blas);
            matInstanceData.instanceData.add(inst);
            matInstanceData.instanceMeshData.add(chunkMeshData);
            matInstanceData.cullingOffsets.add(meshletOffset);
        }
    }
}

void VertexData::createDescriptors() {
    std::unique_lock l(materialDataLock);

    Array<uint32> cullingOffsets;
    for (auto& mat : materialData) {
        for (auto& instance : mat.instances) {
            instance.offsets.instanceOffset = (uint32)instanceData.size();
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
        transparentData[i].offsets.instanceOffset = (uint32)instanceData.size();
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
    descriptorSet->updateBuffer(POSITIONS_NAME, 0, positionBuffer);
    descriptorSet->updateBuffer(INDEXBUFFER_NAME, 0, indexBuffer);
    descriptorSet->updateBuffer(INSTANCES_NAME, 0, instanceBuffer);
    descriptorSet->updateBuffer(MESHDATA_NAME, 0, instanceMeshDataBuffer);
    descriptorSet->updateBuffer(MESHLET_NAME, 0, meshletBuffer);
    descriptorSet->updateBuffer(PRIMITIVEINDICES_NAME, 0, primitiveIndicesBuffer);
    descriptorSet->updateBuffer(VERTEXINDICES_NAME, 0, vertexIndicesBuffer);
    descriptorSet->updateBuffer(CULLINGOFFSETS_NAME, 0, cullingOffsetBuffer);
    Material::updateDescriptor();
}

Array<VertexData::MeshletGroup> VertexData::groupMeshlets(std::span<MeshletDescription> meshlets) {
    auto groupWithAllMeshets = [&]() {
        MeshletGroup group;
        for (uint32 i = 0; i < meshlets.size(); i++) {
            group.meshlets.add(i);
        }
        return Array{group};
    };
    if (meshlets.size() < 8) {
        return groupWithAllMeshets();
    }
    struct MeshletEdge {
        explicit MeshletEdge(size_t a, size_t b) : first(std::min(a, b)), second(std::max(a, b)) {}

        bool operator==(const MeshletEdge& other) const = default;

        const size_t first;
        const size_t second;
    };

    struct MeshletEdgeHasher {
        size_t operator()(const MeshletEdge& edge) const { return CRC::Calculate(&edge, sizeof(MeshletEdge), CRC::CRC_32()); }
    };

    std::unordered_map<MeshletEdge, Array<size_t>, MeshletEdgeHasher> edges2Meshlets;
    std::unordered_map<size_t, Array<MeshletEdge>> meshlets2Edges;

    for (size_t meshletIndex = 0; meshletIndex < meshlets.size(); ++meshletIndex) {
        const auto& meshlet = meshlets[meshletIndex];
        auto getVertexIndex = [&](size_t index) {
            return vertexIndices[meshlet.vertexIndices.offset + primitiveIndices[meshlet.primitiveIndices.offset + index]];
        };
        const size_t triangleCount = meshlet.primitiveIndices.size;

        for (size_t triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex) {
            for (size_t i = 0; i < 3; ++i) {
                MeshletEdge edge{getVertexIndex(i + triangleIndex * 3), getVertexIndex(((i + 1) % 3) + triangleIndex * 3)};
                edges2Meshlets[edge].add(meshletIndex);
                meshlets2Edges[meshletIndex].emplace(edge);
            }
        }
    }

    std::erase_if(edges2Meshlets, [&](const auto& pair) { return pair.second.size() <= 1; });
    if (edges2Meshlets.empty()) {
        return groupWithAllMeshets();
    }

    idx_t vertexCount = meshlets.size();
    idx_t ncon = 1;
    idx_t nparts = meshlets.size() / 4;
    assert(nparts > 1);
    idx_t options[METIS_NOPTIONS];
    METIS_SetDefaultOptions(options);
    options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
    options[METIS_OPTION_CCORDER] = 1;

    Array<idx_t> partition;
    partition.resize(vertexCount);

    Array<idx_t> xadjacency;
    xadjacency.reserve(vertexCount + 1);

    Array<idx_t> edgeAdjacency;
    Array<idx_t> edgeWeights;

    for (size_t meshletIndex = 0; meshletIndex < meshlets.size(); ++meshletIndex) {
        size_t startIndexInEdgeAdjacency = edgeAdjacency.size();
        for (const auto& edge : meshlets2Edges[meshletIndex]) {
            auto connectionsIter = edges2Meshlets.find(edge);
            if (connectionsIter == edges2Meshlets.end()) {
                continue;
            }
            const auto& connections = connectionsIter->second;
            for (const auto& connectedMeshlet : connections) {
                if (connectedMeshlet != meshletIndex) {
                    auto existingEdgeIter =
                        std::find(edgeAdjacency.begin() + startIndexInEdgeAdjacency, edgeAdjacency.end(), connectedMeshlet);
                    if (existingEdgeIter == edgeAdjacency.end()) {
                        edgeAdjacency.emplace(connectedMeshlet);
                        edgeWeights.emplace(1);
                    } else {
                        ptrdiff_t d = std::distance(edgeAdjacency.begin(), existingEdgeIter);
                        assert(d >= 0);
                        assert(d < edgeWeights.size());
                        edgeWeights[d]++;
                    }
                }
            }
        }
        xadjacency.add(startIndexInEdgeAdjacency);
    }
    xadjacency.add(edgeAdjacency.size());
    assert(xadjacency.size() == meshlets.size() + 1);
    assert(edgeAdjacency.size() == edgeWeights.size());

    idx_t edgeCut;
    int result = METIS_PartGraphKway(&vertexCount, &ncon, xadjacency.data(), edgeAdjacency.data(), nullptr, nullptr, edgeWeights.data(),
                                     &nparts, nullptr, nullptr, options, &edgeCut, partition.data());

    assert(result == METIS_OK);

    Array<MeshletGroup> groups;
    groups.resize(nparts);
    for (size_t i = 0; i < meshlets.size(); ++i) {
        idx_t partitionNumber = partition[i];
        groups[partitionNumber].meshlets.add(i);
    }
    return groups;
}

void VertexData::loadMesh(MeshId id, Array<Vector> loadedPositions, Array<uint32> loadedIndices) {
    std::unique_lock l(vertexDataLock);
    RegisteredMesh& mesh = registeredMeshes[id];
    MeshData& data = mesh.meshData;

    // generate an LOD hierarchy for the given source mesh
    // load LOD 0
    size_t previousMeshletsStart = meshlets.size(); // todo:
    loadMeshlets(id, loadedPositions, loadedIndices);

    /* const int maxLod = 25;
    for (int lod = 0; lod < maxLod; ++lod) {
        float tLod = lod / (float)maxLod;

        std::span<MeshletDescription> previousLevelMeshlets =
            std::span{meshlets.data() + previousMeshletsStart, meshlets.size() - previousMeshletsStart};
        if (previousLevelMeshlets.size() <= 1) {
            return;
        }
        auto groups = groupMeshlets(previousLevelMeshlets);
        const uint32 newMeshletStart = meshlets.size();
        for (const auto& group : groups) {
            previousLevelMeshlets = std::span{meshlets.data() + previousMeshletsStart, meshlets.size() - previousMeshletsStart};
            Array<uint32> groupVertexIndices;
            for (const auto& meshletIndex : group.meshlets) {
                const auto& meshlet = meshlets[meshletIndex];
                size_t start = groupVertexIndices.size();
                groupVertexIndices.resize(start + meshlet.primitiveIndices.size * 3);
                for (size_t j = 0; j < meshlet.primitiveIndices.size * 3; ++j) {
                    groupVertexIndices[j + start] = vertexIndices[meshlet.vertexIndices.offset + primitiveIndices[meshlet.primitiveIndices.offset + j]];
                }
            }
            const float threshold = 0.5f;
            size_t targetIndexCount = groupVertexIndices.size() * threshold;
            float targetError = 0.9f * tLod + 0.01f * (1 - tLod);
            uint32 options = meshopt_SimplifyLockBorder;

            Array<uint32> simplifiedIndexBuffer;
            simplifiedIndexBuffer.resize(groupVertexIndices.size());
            float simplificationError = 0.f;

            size_t simplifiedIndexCount = meshopt_simplify(
                simplifiedIndexBuffer.data(), groupVertexIndices.data(), groupVertexIndices.size(), (float*)loadedPositions.data(),
                loadedPositions.size(), sizeof(Vector), targetIndexCount, targetError, options, &simplificationError);
            simplifiedIndexBuffer.resize(simplifiedIndexCount);
            if (simplifiedIndexCount > 0) {
                loadMeshlets(id, loadedPositions, simplifiedIndexBuffer);
                for (size_t i = newMeshletStart; i < meshlets.size(); ++i) {
                    meshlets[i].lod = lod + 1;
                }
                previousMeshletsStart = newMeshletStart;
            }
        }
    }*/
    std::memcpy(positions.data() + mesh.vertexOffset, loadedPositions.data(), loadedPositions.size() * sizeof(Vector));
}

void VertexData::removeMesh(MeshId id) {
    RegisteredMesh& removing = registeredMeshes[id];
    uint32 meshletOffset = meshlets.size();
    uint32 numMeshlets = 0;
    uint32 vertexIndicesOffset = vertexIndices.size();
    uint32 numVertexIndices = 0;
    uint32 primitiveIndicesOffset = primitiveIndices.size();
    uint32 numPrimitiveIndices = 0;
    uint32 indicesOffset = removing.meshData.indicesRange.offset;
    uint32 numIndices = removing.meshData.indicesRange.size;
    const auto& data = removing.meshData;
    meshletOffset = std::min(meshletOffset, data.meshletRange.offset);
    numMeshlets += data.meshletRange.size;
    for (uint32 m = 0; m < data.meshletRange.size; ++m) {
        MeshletDescription& meshlet = meshlets[data.meshletRange.offset + m];
        vertexIndicesOffset = std::min(vertexIndicesOffset, meshlet.vertexIndices.offset);
        numVertexIndices += meshlet.vertexIndices.size;
        primitiveIndicesOffset = std::min(primitiveIndicesOffset, meshlet.primitiveIndices.offset);
        numPrimitiveIndices += meshlet.primitiveIndices.size;
    }

    for (auto& mesh : registeredMeshes) {
        auto& data = mesh.meshData;
        if (data.meshletRange.offset > meshletOffset) {
            for (uint32 i = 0; i < data.meshletRange.size; ++i) {
                MeshletDescription& m = meshlets[data.meshletRange.offset + i];
                if (m.primitiveIndices.offset > primitiveIndicesOffset) {
                    m.primitiveIndices.offset -= numPrimitiveIndices;
                }
                if (m.vertexIndices.offset > vertexIndicesOffset) {
                    m.vertexIndices.offset -= numVertexIndices;
                }
            }
            data.meshletRange.offset -= numMeshlets;
            data.indicesRange.offset -= numIndices;
        }
    }
    uint32 numMeshletsToMove = meshlets.size() - (meshletOffset + numMeshlets);
    uint32 numVertexIndicesToMove = vertexIndices.size() - (vertexIndicesOffset + numVertexIndices);
    uint32 numPrimitiveIndicesToMove = primitiveIndices.size() - (primitiveIndicesOffset + numPrimitiveIndices);
    std::move(meshlets.begin() + meshletOffset + numMeshlets, meshlets.begin() + meshletOffset + numMeshlets + numMeshletsToMove,
              meshlets.begin() + meshletOffset);
    std::move(vertexIndices.begin() + vertexIndicesOffset + numVertexIndices,
              vertexIndices.begin() + vertexIndicesOffset + numVertexIndices + numVertexIndicesToMove,
              vertexIndices.begin() + vertexIndicesOffset);
    std::move(primitiveIndices.begin() + primitiveIndicesOffset + numPrimitiveIndices,
              primitiveIndices.begin() + primitiveIndicesOffset + numPrimitiveIndices + numPrimitiveIndicesToMove,
              primitiveIndices.begin() + primitiveIndicesOffset);
    uint32 numIndicesToMove = indices.size() - (indicesOffset + numIndices);
    std::move(indices.begin() + indicesOffset + numIndices, indices.begin() + indicesOffset + numIndices + numIndicesToMove,
              indices.begin() + indicesOffset);
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
        .name = "MeshletBuffer",
    });
    vertexIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * vertexIndices.size(),
                .data = (uint8*)vertexIndices.data(),
            },
        .numElements = vertexIndices.size(),
        .name = "VertexIndicesBuffer",
    });

    primitiveIndicesBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint8) * primitiveIndices.size(),
                .data = (uint8*)primitiveIndices.data(),
            },
        .numElements = primitiveIndices.size(),
        .name = "PrimitiveIndicesBuffer",
    });
    updateBuffers();
    dirty = false;
    graphics->buildBottomLevelAccelerationStructures(std::move(dataToBuild));
}

MeshId VertexData::allocateVertexData(uint64 numVertices) {
    std::unique_lock l(vertexDataLock);
    MeshId res{idCounter++};
    registeredMeshes.add({
        .vertexOffset = head,
        .vertexCount = numVertices,
    });
    head += numVertices;
    if (head > verticesAllocated) {
        verticesAllocated = 2 * head; // double capacity
        std::cout << "Resizing buffers to " << verticesAllocated << std::endl;

        resizeBuffers();
    }
    return res;
}

void VertexData::serializeMesh(MeshId id, ArchiveBuffer& buffer) {
    std::unique_lock l(vertexDataLock);
    Array<uint32> ind(registeredMeshes[id].meshData.indicesRange.size);
    std::memcpy(ind.data(), indices.data() + registeredMeshes[id].meshData.indicesRange.offset,
                registeredMeshes[id].meshData.indicesRange.size * sizeof(uint32));
    Array<Vector> pos(registeredMeshes[id].vertexCount);
    std::memcpy(pos.data(), positions.data() + registeredMeshes[id].vertexOffset, registeredMeshes[id].vertexCount * sizeof(Vector));
    Serialization::save(buffer, ind);
    Serialization::save(buffer, pos);
}

uint64 VertexData::deserializeMesh(MeshId id, ArchiveBuffer& buffer) {
    Array<Vector> pos;
    Array<uint32> ind;
    Serialization::load(buffer, ind);
    Serialization::load(buffer, pos);
    loadMesh(id, pos, ind);
    uint64 result = pos.size() * sizeof(Vector);
    result += ind.size() * sizeof(uint32);
    return result;
}

List<VertexData*> vertexDataList;

void VertexData::addVertexDataInstance(VertexData* vertexData) { vertexDataList.add(vertexData); }

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

    // positions
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = POSITIONS_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    // indexBuffer
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = INDEXBUFFER_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    // instanceData
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = INSTANCES_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    // meshData
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = MESHDATA_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    // meshletData
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = MESHLET_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    // primitiveIndices
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = PRIMITIVEINDICES_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    // vertexIndices
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = VERTEXINDICES_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    // cullingOffset
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = CULLINGOFFSETS_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    // cullingInfos
    instanceDataLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = CULLINGDATA_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });

    instanceDataLayout->create();

    cullingOffsetBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .name = "MeshletOffset",
    });
    instanceBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .name = "InstanceBuffer",
    });
    instanceMeshDataBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
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
    registeredMeshes.clear();
    materialData.clear();
}

uint32 VertexData::addCullingMapping(MeshId id) {
    uint32 result = (uint32)meshletCount;
    const auto& md = getMeshData(id);
    meshletCount += md.meshletRange.size;
    return result;
}

void VertexData::resizeBuffers() { positions.resize(verticesAllocated); }

void VertexData::updateBuffers() {
    positionBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = verticesAllocated * sizeof(Vector),
                .data = (uint8*)positions.data(),
            },
        .usage = Gfx::SE_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        .name = "Positions",
    });
}

void VertexData::loadMeshlets(MeshId id, const Array<Vector>& loadedPositions, const Array<uint32>& loadedIndices) {
    // Array<uint32> optimizedIndices = indices;
    // tipsifyIndexBuffer(indices, positions.size(), 25, optimizedIndices);

    const float coneWeight = 0.0f;

    const uint32 meshletOffset = meshlets.size();
    const uint32 vertexOffset = vertexIndices.size();
    const uint32 primitiveOffset = primitiveIndices.size();
    const uint32 maxMeshlets = meshopt_buildMeshletsBound(loadedIndices.size(), Gfx::numVerticesPerMeshlet, Gfx::numPrimitivesPerMeshlet);
    Array<meshopt_Meshlet> meshoptMeshlets;
    meshoptMeshlets.resize(maxMeshlets);

    Array<uint32> meshletVertexIndices;
    Array<uint8> meshletTriangles;
    meshletVertexIndices.resize(maxMeshlets * Gfx::numVerticesPerMeshlet);
    meshletTriangles.resize(maxMeshlets * Gfx::numPrimitivesPerMeshlet * 3);

    const uint32 meshletCount =
        meshopt_buildMeshlets(meshoptMeshlets.data(), meshletVertexIndices.data(), meshletTriangles.data(), loadedIndices.data(),
                              loadedIndices.size(), (float*)loadedPositions.data(), loadedPositions.size(), sizeof(Vector),
                              Gfx::numVerticesPerMeshlet, Gfx::numPrimitivesPerMeshlet, coneWeight);

    const meshopt_Meshlet& last = meshoptMeshlets[meshletCount - 1];
    const uint32 vertexCount = last.vertex_offset + last.vertex_count;
    const uint32 indexCount = last.triangle_offset + last.triangle_count * 3;
    vertexIndices.resize(vertexOffset + vertexCount);
    primitiveIndices.resize(primitiveOffset + indexCount);
    meshlets.resize(meshletOffset + meshletCount);

    std::memcpy(vertexIndices.data() + vertexOffset, meshletVertexIndices.data(), vertexCount * sizeof(uint32));
    std::memcpy(primitiveIndices.data() + primitiveOffset, meshletTriangles.data(), indexCount * sizeof(uint8));

    for (size_t i = 0; i < meshletCount; ++i) {
        MeshletDescription& m = meshlets[meshletOffset + i];
        m.vertexIndices = {
            .offset = vertexOffset + meshoptMeshlets[i].vertex_offset,
            .size = meshoptMeshlets[i].vertex_count,
        };
        m.primitiveIndices = {
            .offset = primitiveOffset + meshoptMeshlets[i].triangle_offset,
            .size = meshoptMeshlets[i].triangle_count,
        };
        m.indicesOffset = registeredMeshes[id].vertexOffset;
        // todo: use meshopt for bb generation
        m.bounding = AABB();
        for (size_t j = 0; j < m.vertexIndices.size; ++j) {
            m.bounding.adjust(loadedPositions[vertexIndices[meshoptMeshlets[i].vertex_offset + j]]);
        }
    }
    registeredMeshes[id].meshData = MeshData{
        .bounding = AABB(),
        .meshletRange =
            {
                .offset = meshletOffset,
                .size = meshletCount,
            },
        .indicesRange =
            {
                .offset = (uint32)indices.size(),
                .size = (uint32)loadedIndices.size(),
            },
    };
    // todo: in case of a index split for 16 bit, do something here
    indices.resize(indices.size() + loadedIndices.size());
    std::memcpy(indices.data() + registeredMeshes[id].meshData.indicesRange.offset, loadedIndices.data(),
                loadedIndices.size() * sizeof(uint32));
}

VertexData::VertexData() : idCounter(0), head(0), verticesAllocated(0), dirty(false) {}
