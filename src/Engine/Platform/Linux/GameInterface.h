#pragma once
#include "Game.h"
#include "dlfcn.h"

namespace Seele
{
class GameInterface
{
public:
    GameInterface(std::string soPath);
    ~GameInterface();
    Game* getGame();
    void reload(AssetRegistry* registry);
private:
    void* lib;
    std::string soPath;
    Game* game;
    Game* (*createInstance)(AssetRegistry*) = nullptr;
    void (*destroyInstance)(Game*) = nullptr;
};
} // namespace Seele