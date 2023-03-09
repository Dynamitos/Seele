#include "PointLightActor.h"

using namespace Seele;

PointLightActor::PointLightActor(PScene scene)
    : Actor(scene)
{
    attachComponent<Component::PointLight>();
}

PointLightActor::~PointLightActor()
{
    
}

Component::PointLight& PointLightActor::getPointLightComponent()
{
    return accessComponent<Component::PointLight>();
}

const Component::PointLight& PointLightActor::getPointLightComponent() const
{
    return accessComponent<Component::PointLight>();
}