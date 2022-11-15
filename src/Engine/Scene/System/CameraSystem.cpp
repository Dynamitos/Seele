#include "CameraSystem.h"
#include "Scene/Component/Camera.h"

using namespace Seele;
using namespace Seele::System;

void CameraSystem::update(Component::Camera& camera)
{
    camera.buildViewMatrix();
    camera.buildProjectionMatrix();
}