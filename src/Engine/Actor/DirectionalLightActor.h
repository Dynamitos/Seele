#pragma once
#include "Actor.h"
#include "Component/DirectionalLight.h"

namespace Seele {
class DirectionalLightActor : public Actor {
  public:
    DirectionalLightActor(PScene scene);
    DirectionalLightActor(PScene scene, Vector color, float intensity, Vector direction);
    virtual ~DirectionalLightActor();
    Component::DirectionalLight& getDirectionalLightComponent();
    const Component::DirectionalLight& getDirectionalLightComponent() const;
    Component::Transform& getTransformComponent();
    const Component::Transform& getTransformComponent() const;

  private:
};
DEFINE_REF(DirectionalLightActor)
} // namespace Seele
