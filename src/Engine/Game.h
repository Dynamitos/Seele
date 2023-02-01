#pragma once
#include "Scene/Scene.h"
#include "System/SystemGraph.h"
#include "Asset/AssetRegistry.h"

namespace Seele
{
class Game
{
public:
    Game(AssetRegistry* registry) {}
    virtual ~Game() {}
    virtual void importAssets() = 0;
    virtual void setupScene(PScene scene, PSystemGraph graph) = 0;
};
} // namespace Seele