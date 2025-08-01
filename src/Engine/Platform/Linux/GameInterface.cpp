#include "GameInterface.h"
#include <dlfcn.h>

using namespace Seele;

GameInterface::GameInterface(std::filesystem::path soPath) : lib(NULL), soPath(soPath) {}

GameInterface::~GameInterface() {}

Game* GameInterface::getGame() { return game; }

void GameInterface::reload() {
    if (lib != NULL) {
        destroyInstance(game);
        dlclose(lib);
    }
    lib = dlopen(soPath.string().c_str(), RTLD_NOW);
    createInstance = (decltype(createInstance))dlsym(lib, "createInstance");
    destroyInstance = (decltype(destroyInstance))dlsym(lib, "destroyInstance");
    game = createInstance();
}