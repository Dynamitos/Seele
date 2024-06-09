#include "LightGather.h"

using namespace Seele;
using namespace Seele::System;

LightGather::LightGather(PScene scene) : SystemBase(scene), lightEnv(scene->getLightEnvironment()) {}

LightGather::~LightGather() {}

void LightGather::update() {
    lightEnv->reset();
    scene->view<Component::PointLight>([this](Component::PointLight& pointLight) { lightEnv->addPointLight(pointLight); });
    scene->view<Component::DirectionalLight>([this](Component::DirectionalLight& dirLight) { lightEnv->addDirectionalLight(dirLight); });
}
