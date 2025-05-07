#include "LightGather.h"

using namespace Seele;
using namespace Seele::System;

LightGather::LightGather(PScene scene) : SystemBase(scene), lightEnv(scene->getLightEnvironment()) {}

LightGather::~LightGather() {}

void LightGather::update() {
    lightEnv->reset();
    scene->view<Component::PointLight, Component::Transform>(
        [this](Component::PointLight& pointLight, Component::Transform& transform) { lightEnv->addPointLight(pointLight, transform); });
    scene->view<Component::DirectionalLight, Component::Transform>(
        [this](Component::DirectionalLight& dirLight, Component::Transform& transform) {
            lightEnv->addDirectionalLight(dirLight, transform);
        });
}
