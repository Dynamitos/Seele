#include "PrimitiveComponent.h"
#include "Scene/Scene.h"
#include "Material/MaterialInstance.h"
#include "Asset/MeshAsset.h"
#include "Graphics/RenderPass/VertexFactory.h"

using namespace Seele;

PrimitiveComponent::PrimitiveComponent()
{
}

PrimitiveComponent::PrimitiveComponent(PMeshAsset asset)
{
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
    return getTransform().toMatrix();
}
