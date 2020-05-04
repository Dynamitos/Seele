#include "PrimitiveComponent.h"
#include "Scene/Scene.h"
#include "Material/MaterialInstance.h"

using namespace Seele;

PrimitiveComponent::PrimitiveComponent()
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
