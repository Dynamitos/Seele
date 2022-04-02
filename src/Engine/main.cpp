#include "Window/WindowManager.h"
#include "Scene/Components/PrimitiveComponent.h"
#include "Window/SceneView.h"
#include "Window/InspectorView.h"
#include "Asset/AssetRegistry.h"

using namespace Seele;

int main()
{
    PWindowManager windowManager = new WindowManager();
    AssetRegistry::init("C:\\Users\\Dynamitos\\TestSeeleProject");
    WindowCreateInfo mainWindowInfo;
    mainWindowInfo.title = "SeeleEngine";
    mainWindowInfo.width = 1280;
    mainWindowInfo.height = 720;
    mainWindowInfo.bFullscreen = false;
    mainWindowInfo.numSamples = 1;
    mainWindowInfo.pixelFormat = Gfx::SE_FORMAT_B8G8R8A8_UNORM;
    auto window = windowManager->addWindow(mainWindowInfo);
    ViewportCreateInfo sceneViewInfo;
    sceneViewInfo.sizeX = 1280;
    sceneViewInfo.sizeY = 720;
    sceneViewInfo.offsetX = 0;
    sceneViewInfo.offsetY = 0;
    PSceneView sceneView = new SceneView(windowManager->getGraphics(), window, sceneViewInfo);
    window->addView(sceneView);
    
    ViewportCreateInfo inspectorViewInfo;
    inspectorViewInfo.sizeX = 640;
    inspectorViewInfo.sizeY = 720;
    inspectorViewInfo.offsetX = 640;
    inspectorViewInfo.offsetY = 0;
    //PInspectorView inspectorView = new InspectorView(windowManager->getGraphics(), window, inspectorViewInfo);
    //window->addView(inspectorView);
    sceneView->setFocused();

    window->render();
    
    getGlobalThreadPool().mainLoop();
    return 0;
}
/*
#include <coroutine>
#include <iostream>
#include <thread>
#include <vector>

struct Return;
struct Promise
{
    Promise()
    {
        handle = std::coroutine_handle<Promise>::from_promise(*this);
        numRefs = 0;
    }
    Return get_return_object();
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
    void resume() { handle.resume(); }
    void addRef()
    {
        numRefs++;
    }
    void removeRef()
    {
        numRefs--;
        if(numRefs == 0)
        {
            if(handle.done())
            {
                handle.destroy();
            }
            else
            {
                resume();
            }
        }
    }
    std::coroutine_handle<Promise> handle;
    uint64_t numRefs;
};

struct Return
{
    using promise_type = Promise;
    Return(Promise* promise)
        : promise(promise)
    {
        promise->addRef();
    }
    Return(const Return& other)
    {
        promise = other.promise;
        if(promise != nullptr)
        {
            promise->addRef();
        }
    }
    Return(Return&& other)
    {
        promise = other.promise;
        other.promise = nullptr;
    }
    ~Return()
    {
        promise->removeRef();
    }
    Return& operator=(const Return& other)
    {
        if(this != &other)
        {
            if(promise != nullptr)
            {
                promise->removeRef();
            }
            promise = other.promise;
            if(promise != nullptr)
            {
                promise->addRef();
            }
        }
        return *this;
    }
    Return& operator=(Return&& other)
    {
        if(this != &other)
        {
            if(promise != nullptr)
            {
                promise->removeRef();
            }
            promise = other.promise;
        }
    }
    Promise* promise;
};


Return Promise::get_return_object(){ 
    return {this}; 
}

Return coro1()
{
    std::cout << "coro1" << std::endl;
    co_return;
}

Return coro2()
{
    std::cout << "coro2" << std::endl;
    co_return;
}

Return coroAll(std::vector<Return> coros)
{
    for(auto coro : coros)
    {
        coro.promise->resume();
    }
    co_return;
}

int main()
{
    std::vector<Return> returns{coro1(), coro2()};
    std::thread t = std::thread([returns](){
        coroAll(returns);
    });
    t.join();
}*/