#pragma once
#include "Game.h"
#define NOIME
#include "Windows.h"

namespace Seele {
class GameInterface {
  public:
    GameInterface(std::string dllPath);
    ~GameInterface();
    Game* getGame();
    void reload();

  private:
    HMODULE lib = NULL;
    std::string dllPath;
    Game* game;
    Game* (*createInstance)() = nullptr;
    void (*destroyInstance)(Game*) = nullptr;
};
} // namespace Seele