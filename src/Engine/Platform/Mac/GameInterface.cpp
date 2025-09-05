#include "GameInterface.h"
#include "dlfcn.h"
#include <filesystem>

using namespace Seele;

GameInterface::GameInterface(std::filesystem::path soPath) : lib(NULL), soPath(soPath) {}

GameInterface::~GameInterface() {}

Game* GameInterface::getGame() { return game; }

void GameInterface::reload() {
    if (lib != NULL) {
        destroyInstance(game);
        dlclose(lib);
    }
    //std::filesystem::copy(soPath.parent_path().parent_path() / "res" / "shaders", "shaders/game", std::filesystem::copy_options::overwrite_existing);
    lib = dlopen(soPath.c_str(), RTLD_NOW);
    createInstance = (decltype(createInstance))dlsym(lib, "createInstance");
    destroyInstance = (decltype(destroyInstance))dlsym(lib, "destroyInstance");
    game = createInstance();
}
