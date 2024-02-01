#pragma once
#include "Actor.h"
#include "Component/PointLight.h"

namespace Seele
{
class PointLightActor : public Actor
{
public:
    PointLightActor(PScene scene);
    PointLightActor(PScene scene, Vector position, Vector color, float attenuation);
    virtual ~PointLightActor();
    Component::PointLight& getPointLightComponent();
    const Component::PointLight& getPointLightComponent() const;
private:
};
DEFINE_REF(PointLightActor)
} // namespace Seele
