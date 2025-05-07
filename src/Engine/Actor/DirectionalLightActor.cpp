#include "DirectionalLightActor.h"

using namespace Seele;

DirectionalLightActor::DirectionalLightActor(PScene scene) : Actor(scene) { attachComponent<Component::DirectionalLight>(); }

DirectionalLightActor::DirectionalLightActor(PScene scene, Vector color, float intensity, Vector direction) : Actor(scene) {
    attachComponent<Component::DirectionalLight>(Vector4(color, intensity));
    Vector lightDirection = Vector(direction);
    float dot = glm::dot(lightDirection, Math::Transform::FORWARD);
    Quaternion rotation = Quaternion(1, 0, 0, 0);
    // Handle the edge cases first
    if (glm::epsilonEqual(dot, 1.0f, 0.00001f)) {
        // Vectors are the same, no rotation
    } else if (glm::epsilonEqual(dot, -1.0f, 0.00001f)) {
        // Vectors are opposite need 180-degree rotation around any perpendicular axis
        rotation = glm::angleAxis(glm::pi<float>(), Vector(0, 1, 0));
    } else {
        // Normal case
        Vector axis = glm::normalize(glm::cross(lightDirection, Math::Transform::FORWARD));
        float angle = std::acos(dot); // angle between vectors
        rotation = glm::angleAxis(angle, axis);
    }
    attachComponent<Component::Transform>(Math::Transform(Vector(0, 0, 0), rotation));
}

DirectionalLightActor::~DirectionalLightActor() {}

Component::DirectionalLight& DirectionalLightActor::getDirectionalLightComponent() {
    return accessComponent<Component::DirectionalLight>();
}

const Component::DirectionalLight& DirectionalLightActor::getDirectionalLightComponent() const {
    return accessComponent<Component::DirectionalLight>();
}
Component::Transform& DirectionalLightActor::getTransformComponent() { return accessComponent<Component::Transform>(); }

const Component::Transform& DirectionalLightActor::getTransformComponent() const { return accessComponent<Component::Transform>(); }