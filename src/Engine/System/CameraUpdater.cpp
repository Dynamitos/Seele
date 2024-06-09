#include "CameraUpdater.h"

using namespace Seele;
using namespace Seele::System;

CameraUpdater::CameraUpdater(PScene scene) : ComponentSystem(scene) {}

CameraUpdater::~CameraUpdater() {}

void CameraUpdater::update(Component::Camera& camera) { camera.buildViewMatrix(); }
