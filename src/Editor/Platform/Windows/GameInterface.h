#pragma once
#include "Game.h"
#include "Windows.h"

namespace Seele
{
class GameInterface
{
public:
    GameInterface(std::string dllPath);
    ~GameInterface();
    Game* getGame();
    void reload(AssetRegistry* registry);
private:
    HMODULE lib = NULL;
    std::string dllPath;
    Game* game;
    Game* (*createInstance)(AssetRegistry*) = nullptr;
    void (*destroyInstance)(Game*) = nullptr;
};
} // namespace Seele