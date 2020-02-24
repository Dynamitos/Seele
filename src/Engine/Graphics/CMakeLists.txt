target_sources(SeeleEngine
    PRIVATE
        Graphics.h
        Graphics.cpp
        View.h
        WindowManager.h
        WindowManager.cpp
        Window.h
        Window.cpp
        GraphicsResources.h)

add_subdirectory(Vulkan/)