#include "PointLightActor.h"

using namespace Seele;

PointLightActor::PointLightActor(PScene scene) : Actor(scene) { attachComponent<Component::PointLight>(); }

PointLightActor::PointLightActor(PScene scene, Vector position, float intensity, Vector color, float attenuation) : Actor(scene) {
    attachComponent<Component::PointLight>(color, intensity, attenuation);
    attachComponent<Component::Transform>(Math::Transform(position));
}

PointLightActor::~PointLightActor() {}

Component::PointLight& PointLightActor::getPointLightComponent() { return accessComponent<Component::PointLight>(); }

const Component::PointLight& PointLightActor::getPointLightComponent() const { return accessComponent<Component::PointLight>(); }

Component::Transform& PointLightActor::getTransformComponent() { return accessComponent<Component::Transform>(); }

const Component::Transform& PointLightActor::getTransformComponent() const { return accessComponent<Component::Transform>(); }