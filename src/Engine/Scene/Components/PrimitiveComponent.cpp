#include "PrimitiveComponent.h"
#include "Scene/Scene.h"
#include "Material/MaterialAsset.h"
#include "Asset/MeshAsset.h"
#include "Graphics/VertexShaderInput.h"
#include <iostream>

using namespace Seele;

PrimitiveComponent::PrimitiveComponent()
{
}

PrimitiveComponent::PrimitiveComponent(PMeshAsset asset)
{
    auto assetMeshes = asset->getMeshes();
    staticMeshes.resize(assetMeshes.size());
    for (uint32 i = 0; i < assetMeshes.size(); i++)
    {
        auto& batch = staticMeshes[i];
        batch.material = asset->referencedMaterials[i];
        batch.isBackfaceCullingDisabled = false;
        batch.isCastingShadow = true;
        batch.primitiveComponent = this;
        batch.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        batch.useReverseCulling = false;
        batch.useWireframe = false;
        batch.vertexInput = assetMeshes[i]->vertexInput;
        MeshBatchElement batchElement;
        batchElement.baseVertexIndex = 0;
        batchElement.firstIndex = 0;
        batchElement.indexBuffer = assetMeshes[i]->indexBuffer;
        batchElement.indirectArgsBuffer = nullptr;
        batchElement.instanceRuns = nullptr;
        batchElement.isInstanced = false;
        batchElement.numInstances = 1;
        batchElement.numPrimitives = assetMeshes[i]->indexBuffer->getNumIndices() / 3; //TODO: hardcoded
        batch.elements.push_back(batchElement);
    }
}

PrimitiveComponent::~PrimitiveComponent()
{
}

void PrimitiveComponent::notifySceneAttach(PScene scene)
{
    scene->addPrimitiveComponent(this);
}

Matrix4 PrimitiveComponent::getRenderMatrix()
{
    return getAbsoluteTransform().toMatrix();
}
