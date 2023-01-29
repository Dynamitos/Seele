#include "GameInterface.h"

using namespace Seele;

GameInterface::GameInterface(std::string dllPath)
    : dllPath(dllPath)
{
    reloadGame();
}

GameInterface::~GameInterface()
{
    
}

Game* GameInterface::reloadGame()
{
    if(lib != NULL)
    {
        destroyInstance(game);
        FreeLibrary(lib);
    }
    lib = LoadLibraryA(dllPath.c_str());
    createInstance = (decltype(createInstance))GetProcAddress(lib, "createInstance");
    destroyInstance = (decltype(destroyInstance))GetProcAddress(lib, "destroyInstance");
    game = createInstance();
    return game;
}