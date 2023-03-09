#pragma once
#include "Actor.h"
#include "Component/PointLight.h"

namespace Seele
{
class PointLightActor : public Actor
{
public:
    PointLightActor(PScene scene);
    virtual ~PointLightActor();
    Component::PointLight& getPointLightComponent();
    const Component::PointLight& getPointLightComponent() const;
private:
};
DEFINE_REF(PointLightActor)
} // namespace Seele
