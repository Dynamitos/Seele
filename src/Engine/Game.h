#pragma once
#include "Scene/Scene.h"
#include "System/SystemGraph.h"

namespace Seele
{
class Game
{
public:
    Game() {}
    virtual ~Game() {}
    virtual void setupScene(PScene scene, PSystemGraph graph) = 0;
};
} // namespace Seele