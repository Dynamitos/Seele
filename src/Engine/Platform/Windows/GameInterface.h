#pragma once
#include "Game.h"
#define NOIME
#include "Windows.h"
#include <filesystem>

namespace Seele {
class GameInterface {
  public:
    GameInterface(std::filesystem::path dllPath);
    ~GameInterface();
    Game* getGame();
    void reload();

  private:
    HMODULE lib = NULL;
    std::filesystem::path dllPath;
    Game* game;
    Game* (*createInstance)() = nullptr;
    void (*destroyInstance)(Game*) = nullptr;
};
} // namespace Seele