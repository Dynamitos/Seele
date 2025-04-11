#pragma once
#include "Game.h"
#include "filesystem"
#include "dlfcn.h"

namespace Seele {
class GameInterface {
  public:
    GameInterface(std::filesystem::path soPath);
    ~GameInterface();
    Game* getGame();
    void reload();

  private:
    void* lib;
    std::filesystem::path soPath;
    Game* game;
    Game* (*createInstance)() = nullptr;
    void (*destroyInstance)(Game*) = nullptr;
};
} // namespace Seele