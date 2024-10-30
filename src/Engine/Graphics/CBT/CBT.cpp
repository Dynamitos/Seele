#include "CBT.h"
#include "Asset/AssetRegistry.h"
#include "Graphics/Shader.h"

using namespace Seele;

Array<Vector4> basePoints = {
    // Vector4(0.0f / 3, 0, 0.0f / 3, 1), Vector4(0.0f / 3, 0, 1.0f / 3, 1), Vector4(0.0f / 3, 0, 2.0f / 3, 1),
    // Vector4(0.0f / 3, 0, 3.0f / 3, 1), Vector4(1.0f / 3, 0, 0.0f / 3, 1), Vector4(1.0f / 3, 0, 1.0f / 3, 1),
    // Vector4(1.0f / 3, 0, 2.0f / 3, 1), Vector4(1.0f / 3, 0, 3.0f / 3, 1), Vector4(2.0f / 3, 0, 0.0f / 3, 1),
    // Vector4(2.0f / 3, 0, 1.0f / 3, 1), Vector4(2.0f / 3, 0, 2.0f / 3, 1), Vector4(2.0f / 3, 0, 3.0f / 3, 1),
    // Vector4(3.0f / 3, 0, 0.0f / 3, 1), Vector4(3.0f / 3, 0, 1.0f / 3, 1), Vector4(3.0f / 3, 0, 2.0f / 3, 1),
    // Vector4(3.0f / 3, 0, 3.0f / 3, 1),
    Vector4(0, -0.525731, 0.850651, 1),  Vector4(0.850651, 0, 0.525731, 1),  Vector4(0.850651, 0, -0.525731, 1),
    Vector4(-0.850651, 0, -0.525731, 1), Vector4(-0.850651, 0, 0.525731, 1), Vector4(-0.525731, 0.850651, 0, 1),
    Vector4(0.525731, 0.850651, 0, 1),   Vector4(0.525731, -0.850651, 0, 1), Vector4(-0.525731, -0.850651, 0, 1),
    Vector4(0, -0.525731, -0.850651, 1), Vector4(0, 0.525731, -0.850651, 1), Vector4(0, 0.525731, 0.850651, 1),
};
struct Halfedge {
    uint32 vertexID;
    uint32 nextID;
    uint32 prevID;
    uint32 twinID;
};
Array<Halfedge> edges = {
    // Halfedge{0, 1, 2, 4294967295},
    // Halfedge{4, 2, 0, 3},
    // Halfedge{1, 0, 1, 4294967295},
    //
    // Halfedge{4, 4, 5, 1},
    // Halfedge{5, 5, 3, 8},
    // Halfedge{1, 3, 4, 18},
    //
    // Halfedge{1, 7, 8, 4294967295},
    // Halfedge{5, 8, 6, 9},
    // Halfedge{2, 6, 7, 4},
    //
    // Halfedge{5, 10, 11, 7},
    // Halfedge{6, 11, 9, 14},
    // Halfedge{2, 9, 10, 24},
    //
    // Halfedge{2, 13, 14, 4294967295},
    // Halfedge{6, 14, 12, 15},
    // Halfedge{3, 12, 13, 10},
    //
    // Halfedge{6, 16, 17, 13},
    // Halfedge{7, 17, 15, 4294967295},
    // Halfedge{3, 15, 16, 30},
    //
    // Halfedge{4, 19, 20, 5},
    // Halfedge{8, 20, 18, 21},
    // Halfedge{5, 18, 19, 4294967295},
    //
    // Halfedge{8, 22, 23, 19},
    // Halfedge{9, 23, 21, 26},
    // Halfedge{5, 21, 22, 36},
    //
    // Halfedge{5, 25, 26, 11},
    // Halfedge{9, 26, 24, 27},
    // Halfedge{6, 24, 25, 22},
    //
    // Halfedge{9, 28, 29, 25},
    // Halfedge{10, 29, 27, 32},
    // Halfedge{6, 27, 28, 42},
    //
    // Halfedge{6, 31, 32, 17},
    // Halfedge{10, 32, 30, 33},
    // Halfedge{7, 30, 31, 28},
    //
    // Halfedge{10, 34, 35, 31},
    // Halfedge{11, 35, 33, 4294967295},
    // Halfedge{7, 33, 34, 48},
    //
    // Halfedge{8, 37, 38, 23},
    // Halfedge{12, 38, 36, 39},
    // Halfedge{9, 36, 37, 4294967295},
    //
    // Halfedge{12, 40, 41, 37},
    // Halfedge{13, 41, 39, 44},
    // Halfedge{9, 39, 40, 4294967295},
    //
    // Halfedge{9, 43, 44, 29},
    // Halfedge{13, 44, 42, 45},
    // Halfedge{10, 42, 43, 40},
    //
    // Halfedge{13, 46, 47, 43},
    // Halfedge{14, 47, 45, 50},
    // Halfedge{10, 45, 46, 4294967295},
    //
    // Halfedge{10, 49, 50, 35},
    // Halfedge{14, 50, 48, 51},
    // Halfedge{11, 48, 49, 46},
    //
    // Halfedge{14, 52, 53, 49},
    // Halfedge{15, 53, 51, 4294967295},
    // Halfedge{11, 51, 52, 4294967295},

    Halfedge{1, 1, 2, 5},     Halfedge{2, 2, 0, 36},    Halfedge{6, 0, 1, 39},    Halfedge{1, 4, 5, 51},    Halfedge{7, 5, 3, 48},
    Halfedge{2, 3, 4, 0},     Halfedge{3, 7, 8, 9},     Halfedge{4, 8, 6, 45},    Halfedge{5, 6, 7, 42},    Halfedge{4, 10, 11, 6},
    Halfedge{3, 11, 9, 56},   Halfedge{8, 9, 10, 57},   Halfedge{6, 13, 14, 15},  Halfedge{5, 14, 12, 47},  Halfedge{11, 12, 13, 40},
    Halfedge{5, 16, 17, 12},  Halfedge{6, 17, 15, 38},  Halfedge{10, 15, 16, 43}, Halfedge{9, 19, 20, 21},  Halfedge{10, 20, 18, 37},
    Halfedge{2, 18, 19, 50},  Halfedge{10, 22, 23, 18}, Halfedge{9, 23, 21, 54},  Halfedge{3, 21, 22, 44},  Halfedge{7, 25, 26, 27},
    Halfedge{8, 26, 24, 55},  Halfedge{9, 24, 25, 49},  Halfedge{8, 28, 29, 24},  Halfedge{7, 29, 27, 53},  Halfedge{0, 27, 28, 58},
    Halfedge{11, 31, 32, 33}, Halfedge{0, 32, 30, 52},  Halfedge{1, 30, 31, 41},  Halfedge{0, 34, 35, 30},  Halfedge{11, 35, 33, 46},
    Halfedge{4, 33, 34, 59},  Halfedge{6, 37, 38, 1},   Halfedge{2, 38, 36, 19},  Halfedge{10, 36, 37, 16}, Halfedge{1, 40, 41, 2},
    Halfedge{6, 41, 39, 14},  Halfedge{11, 39, 40, 32}, Halfedge{3, 43, 44, 8},   Halfedge{5, 44, 42, 17},  Halfedge{10, 42, 43, 23},
    Halfedge{5, 46, 47, 7},   Halfedge{4, 47, 45, 34},  Halfedge{11, 45, 46, 13}, Halfedge{2, 49, 50, 4},   Halfedge{7, 50, 48, 26},
    Halfedge{9, 48, 49, 20},  Halfedge{7, 52, 53, 3},   Halfedge{1, 53, 51, 31},  Halfedge{0, 51, 52, 28},  Halfedge{3, 55, 56, 22},
    Halfedge{9, 56, 54, 25},  Halfedge{8, 54, 55, 10},  Halfedge{4, 58, 59, 11},  Halfedge{8, 59, 57, 29},  Halfedge{0, 57, 58, 35},
};

uint32_t find_msb_64(uint64_t x) {
    uint32_t depth = 0;
    while (x > 0u) {
        ++depth;
        x >>= 1uLL;
    }
    return depth;
}

CPUMesh Seele::generateCPUMesh(uint32 cbtNumElements) {
    CPUMesh result;
    result.totalNumElements = edges.size() + cbtNumElements;

    result.heapIDArray.resize(result.totalNumElements);
    result.neighborsArray.resize(result.totalNumElements);
    std::memset(result.heapIDArray.data(), 0, result.totalNumElements * sizeof(uint64));
    std::memset(result.neighborsArray.data(), 0, result.totalNumElements * sizeof(UVector));

    result.minimalDepth = std::min(find_msb_64(edges.size()), 63u) + 1;
    const uint64 baseHeapID = 1ull << (result.minimalDepth - 1);

    result.basePoints.resize(edges.size() * 3);

    UVector neighbours;
    for (uint32 halfedgeIdx = 0; halfedgeIdx < (uint32)edges.size(); ++halfedgeIdx) {
        uint32 elementID = cbtNumElements + halfedgeIdx;

        result.heapIDArray[elementID] = baseHeapID + halfedgeIdx;

        Halfedge& halfedge = edges[halfedgeIdx];
        neighbours.x = halfedge.prevID != -1 ? cbtNumElements + halfedge.prevID : UINT32_MAX;
        neighbours.y = halfedge.nextID != -1 ? cbtNumElements + halfedge.nextID : UINT32_MAX;
        neighbours.z = halfedge.twinID != -1 ? cbtNumElements + halfedge.twinID : UINT32_MAX;
        result.neighborsArray[elementID] = UVector4(neighbours, 0);

        result.basePoints[3 * halfedgeIdx + 2] = basePoints[halfedge.vertexID];

        Halfedge& nextHalfedge = edges[halfedge.nextID];
        result.basePoints[3 * halfedgeIdx + 0] = basePoints[nextHalfedge.vertexID];

        Vector thirdVertex = result.basePoints[3 * halfedgeIdx + 2];
        float sumWeight = 1.0f;
        uint32 currentIdx = halfedge.nextID;
        while (currentIdx != halfedgeIdx) {
            Halfedge currentHalfedge = edges[currentIdx];
            Vector vp = basePoints[currentHalfedge.vertexID];
            thirdVertex += vp;
            sumWeight += 1.0f;
            currentIdx = currentHalfedge.nextID;
        }
        result.basePoints[3 * halfedgeIdx + 1] = Vector4(thirdVertex / sumWeight, 1);
    }
    return result;
}

Matrix3 splittingMatrix(uint32 bitValue) {
    float b = float(bitValue);
    float c = 1.0f - b;

    return glm::transpose(Matrix3({
        0.0f,
        b,
        c,
        0.5f,
        0.0f,
        0.5f,
        b,
        c,
        0.0f,
    }));
}

Matrix3 decodeSubdivisionMatrix(uint64 heapID) {
    Matrix3 m = Matrix3({1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f});
    int32 depth = find_msb_64(heapID) - 1;
    for (int32 bitID = depth - 1; bitID >= 0; --bitID) {
        m = splittingMatrix((heapID >> bitID) & 1u) * m;
    }
    return m;
}

void LebMatrixCache::init(Gfx::PGraphics graphics, uint32 depth) {
    cacheDepth = depth;
    uint32 matrixCount = 2ULL << cacheDepth;
    Matrix3 m;
    std::vector<Matrix3> table(matrixCount);
    table[0] = Matrix3({1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f});
    for (uint64 heapID = 1ULL; heapID < (2ULL << cacheDepth); ++heapID) {
        table[heapID] = decodeSubdivisionMatrix(heapID);
    }
    lebMatrixBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(Matrix3) * matrixCount,
                .data = (uint8*)table.data(),
            },
        .name = "LebMatrixCache",
    });
    lebMatrixBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                     Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                     Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void LebMatrixCache::release() {}

void MeshUpdater::init(Gfx::PGraphics gfx, Gfx::PDescriptorLayout viewParamsLayout) {
    graphics = gfx;
    layout = graphics->createDescriptorLayout("pParams");
    layout->addDescriptorBinding(Gfx::DescriptorBinding{0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{1, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{4, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{5, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{6, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{7, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{8, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{9, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{10, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{11, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{12, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{13, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{14, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{15, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{16, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{17, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{18, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{19, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{20, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{21, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{22, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{23, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{24, Gfx::SE_DESCRIPTOR_TYPE_SAMPLER});
    layout->addDescriptorBinding(Gfx::DescriptorBinding{25, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE});
    layout->create();
    pipelineLayout = graphics->createPipelineLayout("ComputeLayout");
    pipelineLayout->addDescriptorLayout(viewParamsLayout);
    pipelineLayout->addDescriptorLayout(layout);
    pipelineLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(uint32),
    });
    indirectBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * 9,
            },
        .usage = Gfx::SE_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        .name = "IndirectBuffer",
    });
    memoryBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(int32) * 2,
            },
        .name = "MemoryBuffer",
    });
    validationBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(int32) * 2,
            },
        .name = "ValidationBuffer",
    });
    validationBufferRB = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(int32) * 2,
            },
        .name = "ValidationBufferRB",
    });
    occupancyBufferRB = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32),
            },
        .name = "OccupancyBuffer",
    });
    debugBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = 1000000,
            },
        .name = "DebugBuffer",
    });

    graphics->beginShaderCompilation(ShaderCompilationInfo{
        .name = "CBTCompute",
        .modules = {"CBTCompute", "LEB"},
        .entryPoints =
            {
                {"Reset", "CBTCompute"},
                {"Classify", "CBTCompute"},
                {"Split", "CBTCompute"},
                {"PrepareIndirect", "CBTCompute"},
                {"Allocate", "CBTCompute"},
                {"Bisect", "CBTCompute"},
                {"PropagateBisect", "CBTCompute"},
                {"PrepareSimplify", "CBTCompute"},
                {"Simplify", "CBTCompute"},
                {"PropagateSimplify", "CBTCompute"},
                {"ReducePrePass", "CBTCompute"},
                {"ReduceFirstPass", "CBTCompute"},
                {"ReduceSecondPass", "CBTCompute"},
                {"BisectorIndexation", "CBTCompute"},
                {"PrepareBisectorIndirect", "CBTCompute"},
                {"Validate", "CBTCompute"},
                {"ClearBuffer", "LEB"},
                {"EvaluateLEB", "LEB"},
            },
        .rootSignature = pipelineLayout,
    });
    pipelineLayout->create();
    resetCS = graphics->createComputeShader({0});
    reset = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = resetCS,
        .pipelineLayout = pipelineLayout,
    });
    classifyCS = graphics->createComputeShader({1});
    classify = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = classifyCS,
        .pipelineLayout = pipelineLayout,
    });
    splitCS = graphics->createComputeShader({2});
    split = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = splitCS,
        .pipelineLayout = pipelineLayout,
    });
    prepareIndirectCS = graphics->createComputeShader({3});
    prepareIndirect = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = prepareIndirectCS,
        .pipelineLayout = pipelineLayout,
    });
    allocateCS = graphics->createComputeShader({4});
    allocate = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = allocateCS,
        .pipelineLayout = pipelineLayout,
    });
    bisectCS = graphics->createComputeShader({5});
    bisect = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = bisectCS,
        .pipelineLayout = pipelineLayout,
    });
    propagateBisectCS = graphics->createComputeShader({6});
    propagateBisect = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = propagateBisectCS,
        .pipelineLayout = pipelineLayout,
    });
    prepareSimplifyCS = graphics->createComputeShader({7});
    prepareSimplify = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = prepareSimplifyCS,
        .pipelineLayout = pipelineLayout,
    });
    simplifyCS = graphics->createComputeShader({8});
    simplify = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = simplifyCS,
        .pipelineLayout = pipelineLayout,
    });
    propagateSimplifyCS = graphics->createComputeShader({9});
    propagateSimplify = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = propagateSimplifyCS,
        .pipelineLayout = pipelineLayout,
    });
    reducePrePassCS = graphics->createComputeShader({10});
    reducePrePass = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = reducePrePassCS,
        .pipelineLayout = pipelineLayout,
    });
    reduceFirstPassCS = graphics->createComputeShader({11});
    reduceFirstPass = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = reduceFirstPassCS,
        .pipelineLayout = pipelineLayout,
    });
    reduceSecondPassCS = graphics->createComputeShader({12});
    reduceSecondPass = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = reduceSecondPassCS,
        .pipelineLayout = pipelineLayout,
    });
    bisectorIndexationCS = graphics->createComputeShader({13});
    bisectorIndexation = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = bisectorIndexationCS,
        .pipelineLayout = pipelineLayout,
    });
    prepareBisectorIndirectCS = graphics->createComputeShader({14});
    prepareBisectorIndirect = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = prepareBisectorIndirectCS,
        .pipelineLayout = pipelineLayout,
    });
    validateCS = graphics->createComputeShader({15});
    validate = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = validateCS,
        .pipelineLayout = pipelineLayout,
    });
    lebClearCS = graphics->createComputeShader({16});
    lebClear = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = lebClearCS,
        .pipelineLayout = pipelineLayout,
    });
    lebEvaluateCS = graphics->createComputeShader({17});
    lebEvaluate = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = lebEvaluateCS,
        .pipelineLayout = pipelineLayout,
    });
}

void MeshUpdater::release() {}

void MeshUpdater::evaluateLeb(const BaseMesh& baseMesh, CBTMesh& mesh, Gfx::PDescriptorSet viewParamsSet,
                              Gfx::PUniformBuffer geometryBuffer, Gfx::PUniformBuffer updateBuffer, Gfx::PShaderBuffer lebMatrixCache,
                              bool clear, bool complete) {
    if (clear) {
        graphics->beginDebugRegion("ClearLEB");
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(GEOMETRY_CB, 0, geometryBuffer);
        set->updateBuffer(LEB_POSITION_BUFFER, 0, mesh.currentVertexBuffer);
        set->writeChanges();
        Gfx::OComputeCommand clearCmd = graphics->createComputeCommand("Clear");
        clearCmd->bindPipeline(lebClear);
        clearCmd->bindDescriptor(set);
        clearCmd->dispatch((mesh.totalNumElements * 3 + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE, 1, 1);
        graphics->executeCommands(std::move(clearCmd));
        mesh.lebVertexBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                              Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                              Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        graphics->endDebugRegion();
    }
    graphics->beginDebugRegion("EvaluateLEB");
    Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
    set->updateBuffer(GEOMETRY_CB, 0, geometryBuffer);
    set->updateBuffer(UPDATE_CB, 0, updateBuffer);
    set->updateBuffer(CURRENT_VERTEX_BUFFER, 0, baseMesh.vertexBuffer);
    set->updateBuffer(HEAP_ID_BUFFER, 0, mesh.heapIDBuffer);
    set->updateBuffer(INDEXED_BISECTOR_BUFFER, 0, complete ? mesh.indexedBisectorBuffer : mesh.modifiedIndexedBisectorBuffer);
    set->updateBuffer(INDIRECT_DRAW_BUFFER, 0, mesh.indirectDrawBuffer);
    set->updateBuffer(LEB_MATRIX_CACHE, 0, lebMatrixCache);
    set->updateBuffer(LEB_POSITION_BUFFER, 0, mesh.lebVertexBuffer);
    set->writeChanges();
    Gfx::OComputeCommand evalCmd = graphics->createComputeCommand("EvalLEB");
    evalCmd->bindPipeline(lebEvaluate);
    evalCmd->bindDescriptor({set, viewParamsSet});
    uint32 val = complete;
    evalCmd->pushConstants(Gfx::SE_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32), &val);
    evalCmd->dispatchIndirect(mesh.indirectDispatchBuffer, complete ? 0 : sizeof(uint32) * 6);
    graphics->executeCommands(std::move(evalCmd));
    mesh.lebVertexBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                          Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    graphics->endDebugRegion();
}

void MeshUpdater::update(CBTMesh& mesh, Gfx::PDescriptorSet viewParamsSet, Gfx::PUniformBuffer geometryCB, Gfx::PUniformBuffer updateCB) {
    uint32 nextNeighborsBufferIdx = (mesh.currentNeighborsBufferIdx + 1) % 2;
    Gfx::PShaderBuffer currentNeighborsBuffer = mesh.neighborsBuffers[mesh.currentNeighborsBufferIdx];
    Gfx::PShaderBuffer nextNeighborsBuffer = mesh.neighborsBuffers[nextNeighborsBufferIdx];

    resetBuffers(mesh);
    graphics->beginDebugRegion("Classify");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(GEOMETRY_CB, 0, geometryCB);
        set->updateBuffer(UPDATE_CB, 0, updateCB);

        set->updateBuffer(CURRENT_VERTEX_BUFFER, 0, mesh.currentVertexBuffer);
        set->updateBuffer(INDEXED_BISECTOR_BUFFER, 0, mesh.indexedBisectorBuffer);

        set->updateBuffer(INDIRECT_DRAW_BUFFER, 0, mesh.indirectDrawBuffer);
        set->updateBuffer(HEAP_ID_BUFFER, 0, mesh.heapIDBuffer);
        set->updateBuffer(BISECTOR_DATA_BUFFER, 0, mesh.updateBuffer);
        set->updateBuffer(CLASSIFICATION_BUFFER, 0, mesh.classificationBuffer);
        set->writeChanges();
        Gfx::OComputeCommand classifyCmd = graphics->createComputeCommand("Classify");
        classifyCmd->bindPipeline(classify);
        classifyCmd->bindDescriptor({viewParamsSet, set});
        classifyCmd->dispatchIndirect(mesh.indirectDispatchBuffer, 0);
        graphics->executeCommands(std::move(classifyCmd));
        mesh.updateBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                           Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.classificationBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                   Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("PrepareIndirect");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(ALLOCATE_BUFFER, 0, mesh.classificationBuffer);
        set->updateBuffer(INDIRECT_DISPATCH_BUFFER, 0, indirectBuffer);
        set->writeChanges();
        Gfx::OComputeCommand prepareIndirectCmd = graphics->createComputeCommand("PrepareIndirect");
        prepareIndirectCmd->bindPipeline(prepareIndirect);
        prepareIndirectCmd->bindDescriptor(set);
        prepareIndirectCmd->dispatch(2, 1, 1);
        graphics->executeCommands(std::move(prepareIndirectCmd));
        indirectBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                        Gfx::SE_ACCESS_INDIRECT_COMMAND_READ_BIT, Gfx::SE_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
    }
    graphics->endDebugRegion();
    graphics->beginDebugRegion("Split");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(GEOMETRY_CB, 0, geometryCB);
        set->updateBuffer(UPDATE_CB, 0, updateCB);

        set->updateBuffer(CLASSIFICATION_BUFFER, 0, mesh.classificationBuffer);
        set->updateBuffer(HEAP_ID_BUFFER, 0, mesh.heapIDBuffer);
        set->updateBuffer(BISECTOR_DATA_BUFFER, 0, mesh.updateBuffer);
        set->updateBuffer(NEIGHBOURS_BUFFER, 0, currentNeighborsBuffer);
        set->updateBuffer(MEMORY_BUFFER, 0, memoryBuffer);
        set->updateBuffer(ALLOCATE_BUFFER, 0, mesh.allocateBuffer);
        set->writeChanges();
        Gfx::OComputeCommand splitCmd = graphics->createComputeCommand("Split");
        splitCmd->bindPipeline(split);
        splitCmd->bindDescriptor({viewParamsSet, set});
        splitCmd->dispatchIndirect(indirectBuffer, 0);
        graphics->executeCommands(std::move(splitCmd));
        memoryBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                      Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.updateBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                           Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.allocateBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("PrepareIndirect");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(ALLOCATE_BUFFER, 0, mesh.allocateBuffer);
        set->updateBuffer(INDIRECT_DISPATCH_BUFFER, 0, indirectBuffer);
        set->writeChanges();
        Gfx::OComputeCommand prepareIndirectCmd = graphics->createComputeCommand("PrepareIndirect");
        prepareIndirectCmd->bindPipeline(prepareIndirect);
        prepareIndirectCmd->bindDescriptor(set);
        prepareIndirectCmd->dispatch(1, 1, 1);
        graphics->executeCommands(std::move(prepareIndirectCmd));
        indirectBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                        Gfx::SE_ACCESS_INDIRECT_COMMAND_READ_BIT, Gfx::SE_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("Allocate");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(GEOMETRY_CB, 0, geometryCB);
        set->updateBuffer(UPDATE_CB, 0, updateCB);

        set->updateBuffer(CBT_BUFFER0, 0, mesh.gpuCBT.bufferArray[0]);
        set->updateBuffer(CBT_BUFFER1, 0, mesh.gpuCBT.bufferArray[1]);
        set->updateBuffer(ALLOCATE_BUFFER, 0, mesh.allocateBuffer);
        set->updateBuffer(BISECTOR_DATA_BUFFER, 0, mesh.updateBuffer);
        set->updateBuffer(MEMORY_BUFFER, 0, memoryBuffer);
        set->updateBuffer(DEBUG_BUFFER, 0, debugBuffer);
        set->writeChanges();
        Gfx::OComputeCommand allocateCmd = graphics->createComputeCommand("Allocate");
        allocateCmd->bindPipeline(allocate);
        allocateCmd->bindDescriptor({viewParamsSet, set});
        allocateCmd->dispatchIndirect(indirectBuffer, 0);
        graphics->executeCommands(std::move(allocateCmd));
        memoryBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                      Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.updateBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                           Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("Copy");
    currentNeighborsBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                            Gfx::SE_ACCESS_TRANSFER_READ_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    graphics->copyBuffer(currentNeighborsBuffer, nextNeighborsBuffer);
    nextNeighborsBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                         Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    graphics->endDebugRegion();

    graphics->beginDebugRegion("Bisect");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(GEOMETRY_CB, 0, geometryCB);
        set->updateBuffer(UPDATE_CB, 0, updateCB);

        set->updateBuffer(CBT_BUFFER0, 0, mesh.gpuCBT.bufferArray[0]);
        set->updateBuffer(CBT_BUFFER1, 0, mesh.gpuCBT.bufferArray[1]);
        set->updateBuffer(ALLOCATE_BUFFER, 0, mesh.allocateBuffer);
        set->updateBuffer(HEAP_ID_BUFFER, 0, mesh.heapIDBuffer);
        set->updateBuffer(BISECTOR_DATA_BUFFER, 0, mesh.updateBuffer);
        set->updateBuffer(NEIGHBOURS_BUFFER, 0, currentNeighborsBuffer);
        set->updateBuffer(NEIGHBOURS_OUTPUT_BUFFER, 0, nextNeighborsBuffer);
        set->updateBuffer(PROPAGATE_BUFFER, 0, mesh.propagateBuffer);
        set->writeChanges();
        Gfx::OComputeCommand bisectCmd = graphics->createComputeCommand("Bisect");
        bisectCmd->bindPipeline(bisect);
        bisectCmd->bindDescriptor({viewParamsSet, set});
        bisectCmd->dispatchIndirect(indirectBuffer, 0);
        graphics->executeCommands(std::move(bisectCmd));
        nextNeighborsBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.heapIDBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                           Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.propagateBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                              Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.gpuCBT.bufferArray[1]->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                    Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("PrepareIndirect");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(ALLOCATE_BUFFER, 0, mesh.propagateBuffer);
        set->updateBuffer(INDIRECT_DISPATCH_BUFFER, 0, indirectBuffer);
        set->writeChanges();
        Gfx::OComputeCommand prepareIndirectCmd = graphics->createComputeCommand("PrepareIndirectPropagateBisect");
        prepareIndirectCmd->bindPipeline(prepareIndirect);
        prepareIndirectCmd->bindDescriptor(set);
        prepareIndirectCmd->dispatch(1, 1, 1);
        graphics->executeCommands(std::move(prepareIndirectCmd));
        indirectBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                        Gfx::SE_ACCESS_INDIRECT_COMMAND_READ_BIT, Gfx::SE_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("PropagateBisect");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(GEOMETRY_CB, 0, geometryCB);
        set->updateBuffer(UPDATE_CB, 0, updateCB);

        set->updateBuffer(PROPAGATE_BUFFER, 0, mesh.propagateBuffer);
        set->updateBuffer(BISECTOR_DATA_BUFFER, 0, mesh.updateBuffer);
        set->updateBuffer(NEIGHBOURS_BUFFER, 0, nextNeighborsBuffer);
        set->writeChanges();
        Gfx::OComputeCommand propagateBisectCmd = graphics->createComputeCommand("PropagateBisect");
        propagateBisectCmd->bindPipeline(propagateBisect);
        propagateBisectCmd->bindDescriptor({viewParamsSet, set});
        propagateBisectCmd->dispatchIndirect(indirectBuffer, 0);
        graphics->executeCommands(std::move(propagateBisectCmd));
        mesh.updateBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                           Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        nextNeighborsBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("PrepareSimplify");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(GEOMETRY_CB, 0, geometryCB);
        set->updateBuffer(UPDATE_CB, 0, updateCB);

        set->updateBuffer(CLASSIFICATION_BUFFER, 0, mesh.classificationBuffer);
        set->updateBuffer(HEAP_ID_BUFFER, 0, mesh.heapIDBuffer);
        set->updateBuffer(BISECTOR_DATA_BUFFER, 0, mesh.updateBuffer);
        set->updateBuffer(NEIGHBOURS_BUFFER, 0, nextNeighborsBuffer);
        set->updateBuffer(SIMPLIFICATION_BUFFER, 0, mesh.simplificationBuffer);
        set->writeChanges();
        Gfx::OComputeCommand prepareSimplifyCmd = graphics->createComputeCommand("PrepareSimplify");
        prepareSimplifyCmd->bindPipeline(prepareSimplify);
        prepareSimplifyCmd->bindDescriptor({viewParamsSet, set});
        prepareSimplifyCmd->dispatchIndirect(indirectBuffer, 3 * sizeof(uint32_t));
        graphics->executeCommands(std::move(prepareSimplifyCmd));
        mesh.simplificationBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                   Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("PrepareIndirectSimplify");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(ALLOCATE_BUFFER, 0, mesh.simplificationBuffer);
        set->updateBuffer(INDIRECT_DISPATCH_BUFFER, 0, indirectBuffer);
        set->writeChanges();
        Gfx::OComputeCommand prepareIndirectCmd = graphics->createComputeCommand("PrepareIndirectSimplify");
        prepareIndirectCmd->bindPipeline(prepareIndirect);
        prepareIndirectCmd->bindDescriptor(set);
        prepareIndirectCmd->dispatch(1, 1, 1);
        graphics->executeCommands(std::move(prepareIndirectCmd));
        indirectBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                        Gfx::SE_ACCESS_INDIRECT_COMMAND_READ_BIT, Gfx::SE_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("Simplify");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(GEOMETRY_CB, 0, geometryCB);
        set->updateBuffer(UPDATE_CB, 0, updateCB);

        set->updateBuffer(SIMPLIFICATION_BUFFER, 0, mesh.simplificationBuffer);
        set->updateBuffer(BISECTOR_DATA_BUFFER, 0, mesh.updateBuffer);
        set->updateBuffer(NEIGHBOURS_BUFFER, 0, nextNeighborsBuffer);
        set->updateBuffer(HEAP_ID_BUFFER, 0, mesh.heapIDBuffer);
        set->updateBuffer(CBT_BUFFER0, 0, mesh.gpuCBT.bufferArray[0]);
        set->updateBuffer(CBT_BUFFER1, 0, mesh.gpuCBT.bufferArray[1]);
        set->updateBuffer(PROPAGATE_BUFFER, 0, mesh.propagateBuffer);
        set->writeChanges();
        Gfx::OComputeCommand simplifyCmd = graphics->createComputeCommand("simplify");
        simplifyCmd->bindPipeline(simplify);
        simplifyCmd->bindDescriptor({viewParamsSet, set});
        simplifyCmd->dispatchIndirect(indirectBuffer, 0);
        graphics->executeCommands(std::move(simplifyCmd));
        nextNeighborsBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.heapIDBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                           Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.updateBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                           Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.propagateBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                              Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.gpuCBT.bufferArray[1]->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                    Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("PrepareIndirectPropagateSimplify");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(ALLOCATE_BUFFER, 0, mesh.propagateBuffer);
        set->updateBuffer(INDIRECT_DISPATCH_BUFFER, 0, indirectBuffer);
        set->writeChanges();
        Gfx::OComputeCommand prepareIndirectCmd = graphics->createComputeCommand("PrepareIndirectPropagateSimplify");
        prepareIndirectCmd->bindPipeline(prepareIndirect);
        prepareIndirectCmd->bindDescriptor({viewParamsSet, set});
        prepareIndirectCmd->dispatch(2, 1, 1);
        graphics->executeCommands(std::move(prepareIndirectCmd));
        indirectBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                        Gfx::SE_ACCESS_INDIRECT_COMMAND_READ_BIT, Gfx::SE_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("PropagateSimplify");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(GEOMETRY_CB, 0, geometryCB);
        set->updateBuffer(UPDATE_CB, 0, updateCB);

        set->updateBuffer(PROPAGATE_BUFFER, 0, mesh.propagateBuffer);
        set->updateBuffer(HEAP_ID_BUFFER, 0, mesh.heapIDBuffer);
        set->updateBuffer(BISECTOR_DATA_BUFFER, 0, mesh.updateBuffer);
        set->updateBuffer(NEIGHBOURS_BUFFER, 0, nextNeighborsBuffer);
        set->writeChanges();
        Gfx::OComputeCommand propagateSimplifyCmd = graphics->createComputeCommand("PropagateSimplify");
        propagateSimplifyCmd->bindPipeline(propagateSimplify);
        propagateSimplifyCmd->bindDescriptor({viewParamsSet, set});
        propagateSimplifyCmd->dispatchIndirect(indirectBuffer, 3 * sizeof(uint32));
        graphics->executeCommands(std::move(propagateSimplifyCmd));
        nextNeighborsBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.updateBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                           Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    graphics->endDebugRegion();

    graphics->beginDebugRegion("Update Tree");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(CBT_BUFFER0, 0, mesh.gpuCBT.bufferArray[0]);
        set->updateBuffer(CBT_BUFFER1, 0, mesh.gpuCBT.bufferArray[1]);
        set->writeChanges();

        Gfx::OComputeCommand reducePrePassCmd = graphics->createComputeCommand("ReducePrepass");
        reducePrePassCmd->bindPipeline(reducePrePass);
        reducePrePassCmd->bindDescriptor(set);
        reducePrePassCmd->dispatch(mesh.gpuCBT.lastLevelSize / (4 * WORKGROUP_SIZE), 1, 1);
        graphics->executeCommands(std::move(reducePrePassCmd));
        mesh.gpuCBT.bufferArray[0]->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                    Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        Gfx::OComputeCommand reduceFirstPassCmd = graphics->createComputeCommand("ReduceFirstPass");
        reduceFirstPassCmd->bindPipeline(reduceFirstPass);
        reduceFirstPassCmd->bindDescriptor(set);
        reduceFirstPassCmd->dispatch(8, 1, 1);
        graphics->executeCommands(std::move(reduceFirstPassCmd));
        mesh.gpuCBT.bufferArray[0]->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                    Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        Gfx::OComputeCommand reduceSecondPassCmd = graphics->createComputeCommand("ReduceSecondPass");
        reduceSecondPassCmd->bindPipeline(reduceSecondPass);
        reduceSecondPassCmd->bindDescriptor(set);
        reduceSecondPassCmd->dispatch(1, 1, 1);
        graphics->executeCommands(std::move(reduceSecondPassCmd));
        mesh.gpuCBT.bufferArray[0]->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                    Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    graphics->endDebugRegion();

    mesh.currentNeighborsBufferIdx = nextNeighborsBufferIdx;

    prepareIndirection(mesh, geometryCB);
}

void MeshUpdater::validation(CBTMesh& mesh, Gfx::PUniformBuffer geometryCB) {
    uint32 numGroups = (mesh.totalNumElements + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;

    Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
    set->updateBuffer(GEOMETRY_CB, 0, geometryCB);

    set->updateBuffer(HEAP_ID_BUFFER, 0, mesh.heapIDBuffer);
    set->updateBuffer(BISECTOR_DATA_BUFFER, 0, mesh.updateBuffer);
    set->updateBuffer(NEIGHBOURS_BUFFER, 0, mesh.neighborsBuffers[mesh.currentNeighborsBufferIdx]);
    set->updateBuffer(VALIDATION_BUFFER, 0, validationBuffer);
    set->writeChanges();

    Gfx::OComputeCommand validateCmd = graphics->createComputeCommand("Validate");
    validateCmd->bindPipeline(validate);
    validateCmd->bindDescriptor(set);
    validateCmd->dispatch(numGroups, 1, 1);
    graphics->executeCommands(std::move(validateCmd));
    validationBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                      Gfx::SE_ACCESS_TRANSFER_READ_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    graphics->copyBuffer(validationBuffer, validationBufferRB);
}

void MeshUpdater::resetBuffers(CBTMesh& mesh) {
    graphics->beginDebugRegion("ResetBuffers");
    Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
    set->updateBuffer(CBT_BUFFER0, 0, mesh.gpuCBT.bufferArray[0]);
    set->updateBuffer(CBT_BUFFER1, 0, mesh.gpuCBT.bufferArray[1]);
    set->updateBuffer(MEMORY_BUFFER, 0, memoryBuffer);
    set->updateBuffer(CLASSIFICATION_BUFFER, 0, mesh.classificationBuffer);
    set->updateBuffer(ALLOCATE_BUFFER, 0, mesh.allocateBuffer);
    set->updateBuffer(INDIRECT_DRAW_BUFFER, 0, mesh.indirectDrawBuffer);
    set->updateBuffer(SIMPLIFICATION_BUFFER, 0, mesh.simplificationBuffer);
    set->updateBuffer(PROPAGATE_BUFFER, 0, mesh.propagateBuffer);
    set->writeChanges();

    Gfx::OComputeCommand resetCmd = graphics->createComputeCommand("Reset");
    resetCmd->bindPipeline(reset);
    resetCmd->bindDescriptor(set);
    resetCmd->dispatch(1, 1, 1);
    graphics->executeCommands(std::move(resetCmd));
    memoryBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                  Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    mesh.classificationBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                               Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    mesh.allocateBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                         Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    mesh.propagateBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                          Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    mesh.simplificationBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                               Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    mesh.indirectDrawBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    graphics->endDebugRegion();
}

void MeshUpdater::prepareIndirection(CBTMesh& mesh, Gfx::PUniformBuffer geometryCB) {
    uint32 numGroups = (mesh.totalNumElements + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    graphics->beginDebugRegion("BisectorIndexation");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(GEOMETRY_CB, 0, geometryCB);
        set->updateBuffer(HEAP_ID_BUFFER, 0, mesh.heapIDBuffer);
        set->updateBuffer(INDIRECT_DRAW_BUFFER, 0, mesh.indirectDrawBuffer);
        set->updateBuffer(BISECTOR_INDICES, 0, mesh.indexedBisectorBuffer);
        set->updateBuffer(BISECTOR_DATA_BUFFER, 0, mesh.updateBuffer);
        set->updateBuffer(NEIGHBOURS_BUFFER, 0, mesh.neighborsBuffers[mesh.currentNeighborsBufferIdx]);
        set->updateBuffer(VISIBLE_BISECTOR_INDICES, 0, mesh.visibleIndexedBisectorBuffer);
        set->updateBuffer(MODIFIED_BISECTOR_INDICES, 0, mesh.modifiedIndexedBisectorBuffer);
        set->writeChanges();
        Gfx::OComputeCommand bisectorIndexationCmd = graphics->createComputeCommand("BisectorIndexation");
        bisectorIndexationCmd->bindPipeline(bisectorIndexation);
        bisectorIndexationCmd->bindDescriptor(set);
        bisectorIndexationCmd->dispatch(numGroups, 1, 1);
        graphics->executeCommands(std::move(bisectorIndexationCmd));
        mesh.indirectDrawBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                 Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.indexedBisectorBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                    Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.visibleIndexedBisectorBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                           Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mesh.modifiedIndexedBisectorBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                            Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    graphics->endDebugRegion();
    graphics->beginDebugRegion("PrepareBisectorIndirectDispatch");
    {
        Gfx::PDescriptorSet set = layout->allocateDescriptorSet();
        set->updateBuffer(INDIRECT_DRAW_BUFFER, 0, mesh.indirectDrawBuffer);
        set->updateBuffer(INDIRECT_DISPATCH_BUFFER, 0, mesh.indirectDispatchBuffer);
        set->writeChanges();

        Gfx::OComputeCommand prepareBisectorIndirectCmd = graphics->createComputeCommand("PrepareBisectorIndirect");
        prepareBisectorIndirectCmd->bindPipeline(prepareBisectorIndirect);
        prepareBisectorIndirectCmd->bindDescriptor(set);
        prepareBisectorIndirectCmd->dispatch(1, 1, 1);
        graphics->executeCommands(std::move(prepareBisectorIndirectCmd));
        mesh.indirectDispatchBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                     Gfx::SE_ACCESS_INDIRECT_COMMAND_READ_BIT, Gfx::SE_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
        mesh.indirectDrawBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                 Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }
    graphics->endDebugRegion();
}

bool MeshUpdater::checkIfValid() { return true; }

void MeshUpdater::queryOccupancy(const CBTMesh& mesh) {}

uint32 MeshUpdater::getOccupancy() { return 0; }
