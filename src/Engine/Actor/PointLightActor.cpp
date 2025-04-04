#include "PointLightActor.h"

using namespace Seele;

PointLightActor::PointLightActor(PScene scene) : Actor(scene) { attachComponent<Component::PointLight>(); }

PointLightActor::PointLightActor(PScene scene, Vector position, float intensity, Vector color, float attenuation) : Actor(scene) {
    attachComponent<Component::PointLight>(Vector4(position, intensity), Vector4(color, attenuation));
}

PointLightActor::~PointLightActor() {}

Component::PointLight& PointLightActor::getPointLightComponent() { return accessComponent<Component::PointLight>(); }

const Component::PointLight& PointLightActor::getPointLightComponent() const { return accessComponent<Component::PointLight>(); }