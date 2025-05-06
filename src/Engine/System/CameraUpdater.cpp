#include "CameraUpdater.h"

using namespace Seele;
using namespace Seele::System;

CameraUpdater::CameraUpdater(PScene scene) : ComponentSystem<Component::Camera, Component::Transform>(scene) {}

CameraUpdater::~CameraUpdater() {}

void CameraUpdater::update(Component::Camera& camera, Component::Transform& transform) {
    Vector eyePos = transform.getPosition();
    Vector lookAt = eyePos + transform.getForward();
}
