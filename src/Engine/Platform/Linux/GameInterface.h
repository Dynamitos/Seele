#pragma once
#include "Game.h"
#include "dlfcn.h"

namespace Seele {
class GameInterface {
  public:
    GameInterface(std::string soPath);
    ~GameInterface();
    Game* getGame();
    void reload();

  private:
    void* lib;
    std::string soPath;
    Game* game;
    Game* (*createInstance)() = nullptr;
    void (*destroyInstance)(Game*) = nullptr;
};
} // namespace Seele