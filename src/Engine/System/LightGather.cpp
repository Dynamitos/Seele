#include "LightGather.h"

using namespace Seele;
using namespace Seele::System;

LightGather::LightGather(PScene scene)
    : SystemBase(scene)
    , lightEnv(scene->getLightEnvironment())
{
}

LightGather::~LightGather()
{
}

void LightGather::update()
{
}
