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
    Game* reloadGame();
private:
    HMODULE lib = NULL;
    std::string dllPath;
    Game* game;
    Game* (*createInstance)() = nullptr;
    void (*destroyInstance)(Game*) = nullptr;
};
} // namespace Seele