#pragma once
#include "Actor.h"
#include "Component/DirectionalLight.h"

namespace Seele
{
class DirectionalLightActor : public Actor
{
public:
    DirectionalLightActor(PScene scene);
    DirectionalLightActor(PScene scene, Vector color, Vector direction);
    virtual ~DirectionalLightActor();
    Component::DirectionalLight& getDirectionalLightComponent();
    const Component::DirectionalLight& getDirectionalLightComponent() const;
private:
};
DEFINE_REF(DirectionalLightActor)
} // namespace Seele
