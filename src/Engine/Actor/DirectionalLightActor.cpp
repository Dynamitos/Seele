#include "DirectionalLightActor.h"

using namespace Seele;

DirectionalLightActor::DirectionalLightActor(PScene scene) : Actor(scene) { attachComponent<Component::DirectionalLight>(); }

DirectionalLightActor::DirectionalLightActor(PScene scene, Vector color, float intensity, Vector direction) : Actor(scene) {
    attachComponent<Component::DirectionalLight>(Vector4(color, intensity), Vector4(direction, 0));
}

DirectionalLightActor::~DirectionalLightActor() {}

Component::DirectionalLight& DirectionalLightActor::getDirectionalLightComponent() {
    return accessComponent<Component::DirectionalLight>();
}

const Component::DirectionalLight& DirectionalLightActor::getDirectionalLightComponent() const {
    return accessComponent<Component::DirectionalLight>();
}