#include "GameInterface.h"
#include "Windows.h"

using namespace Seele;

GameInterface::GameInterface(std::filesystem::path dllPath) : dllPath(dllPath) {}

GameInterface::~GameInterface() {}

Game* GameInterface::getGame() { return game; }

void GameInterface::reload() {
    if (lib != NULL) {
        destroyInstance(game);
        FreeLibrary(lib);
    }
    std::filesystem::copy(dllPath.parent_path().parent_path() / "res" / "shaders", "./shaders/game", std::filesystem::copy_options::overwrite_existing);
    lib = LoadLibraryA(dllPath.string().c_str());
    createInstance = (decltype(createInstance))GetProcAddress(lib, "createInstance");
    destroyInstance = (decltype(destroyInstance))GetProcAddress(lib, "destroyInstance");
    game = createInstance();
}