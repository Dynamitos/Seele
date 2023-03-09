#include "DirectionalLightActor.h"

using namespace Seele;

DirectionalLightActor::DirectionalLightActor(PScene scene)
    : Actor(scene)
{
    attachComponent<Component::DirectionalLight>();
}

DirectionalLightActor::~DirectionalLightActor()
{
    
}

Component::DirectionalLight& DirectionalLightActor::getDirectionalLightComponent()
{
    return accessComponent<Component::DirectionalLight>();
}

const Component::DirectionalLight& DirectionalLightActor::getDirectionalLightComponent() const
{
    return accessComponent<Component::DirectionalLight>();
}